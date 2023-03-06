module;

#include <boost/stl_interfaces/iterator_interface.hpp>

#include <cassert>
#include <coroutine>
#include <ranges>
#include <variant>

export module corotest;

using boost::stl_interfaces::iterator_interface;

template <typename T, typename C> class lazy_cache_promise {
  struct CacheEmpty {
    bool operator==(const CacheEmpty &) const = default;
  };
  struct CacheFilled {
    bool operator==(const CacheFilled &) const = default;
    T value;
  };
  struct Done {
    bool operator==(const Done &) const = default;
  };
  std::variant<CacheEmpty, CacheFilled, Done> state_ = CacheEmpty{};

public:
  ///////////////////////////////
  // Coroutine promise interface
  ///////////////////////////////
  C get_return_object() {
    return C{std::coroutine_handle<lazy_cache_promise>::from_promise(*this)};
  }
  std::suspend_always initial_suspend() { return {}; }
  std::suspend_never final_suspend() noexcept {
    state_ = Done{};
    return {};
  }
  void unhandled_exception() {}
  std::suspend_always yield_value(T value) {
    state_ = CacheFilled{std::move(value)};
    return {};
  }

  /////////////////////
  // External interface
  /////////////////////

  bool has_empty_cache() const {
    return std::get_if<CacheEmpty>(&state_) != nullptr;
  }
  bool has_filled_cache() const {
    return std::get_if<CacheFilled>(&state_) != nullptr;
  }
  bool is_done() const { return std::get_if<Done>(&state_) != nullptr; }

  const T &get_filled_cache_value() const {
    CacheFilled const *const cacheFilled = std::get_if<CacheFilled>(&state_);
    assert(cacheFilled != nullptr);
    return cacheFilled->value;
  }
  void clear_cache() { state_ = CacheEmpty{}; }
};

export template <typename T>
struct input_iterator_generator
    : iterator_interface<input_iterator_generator<T>, std::input_iterator_tag,
                         T> {

  //////////////////////
  // Coroutine interface
  //////////////////////

  using promise_type = lazy_cache_promise<T, input_iterator_generator>;

  // TODO: I want this to be private, but I don't know how to do that
  // Adding
  // `friend class iterator_interface<input_iterator_generator<T>,
  // std::input_iterator_tag, T>;` doesn't work
  input_iterator_generator(std::coroutine_handle<promise_type> h)
      : handle_(std::move(h)) {}

  /////////////////////
  // Iterator interface
  /////////////////////

  const T &operator*() const {
    assert(handle_);
    try_fill_cache();
    assert(handle_.promise().has_filled_cache());
    return handle_.promise().get_filled_cache_value();
  }
  input_iterator_generator &operator++() {
    assert(handle_);
    try_fill_cache();
    assert(!handle_.promise().is_done());
    handle_.promise().clear_cache();
    return *this;
  }
  friend bool operator==(const input_iterator_generator &lhs,
                         const input_iterator_generator &rhs) {
    return lhs.is_end() == rhs.is_end();
  }

  /////////////////////
  // External interface
  /////////////////////
  input_iterator_generator() {}

private:
  void try_fill_cache() const {
    if (handle_.promise().has_empty_cache())
      handle_.resume();
  }
  bool is_end() const {
    if (!handle_) {
      return true;
    } else {
      try_fill_cache();
      return handle_.promise().is_done();
    }
  }

  std::coroutine_handle<promise_type> handle_ = {};
};

export template <typename BaseType, typename T>
struct input_iterator_generator_g
    : iterator_interface<BaseType, std::input_iterator_tag, T> {

  /////////////////////
  // Iterator interface
  /////////////////////

  template <typename unused = void> const T &operator*() const {
    assert(static_cast<BaseType const *>(this)->handle_);
    try_fill_cache();
    assert(static_cast<BaseType const *>(this)->handle_.promise().has_filled_cache());
    return static_cast<BaseType const *>(this)
        ->handle_.promise()
        .get_filled_cache_value();
  }
  template <typename unused = void> BaseType &operator++() {
    assert(static_cast<BaseType *>(this)->handle_);
    try_fill_cache();
    assert(!static_cast<BaseType *>(this)->handle_.promise().is_done());
    static_cast<BaseType *>(this)->handle_.promise().clear_cache();
    return *static_cast<BaseType *>(this);
  }
  template <typename unused = void> bool operator==(const BaseType &rhs) const{
    return static_cast<BaseType const *>(this)->is_end() == rhs.is_end();
  }

private:
  template <typename unused = void> void try_fill_cache() const {
    if (static_cast<BaseType const *>(this)->handle_.promise().has_empty_cache())
      static_cast<BaseType const *>(this)->handle_.resume();
  }
  template <typename unused = void> bool is_end() const {
    if (!static_cast<BaseType const *>(this)->handle_) {
      return true;
    } else {
      try_fill_cache();
      return static_cast<BaseType const *>(this)->handle_.promise().is_done();
    }
  }
};

export template <typename T>
struct view_generator // : public std::ranges::view_interface<view_generator<T>>
{
  //////////////////////
  // Coroutine interface
  //////////////////////

  using promise_type = lazy_cache_promise<T, view_generator>;

  struct iterator : public input_iterator_generator_g<iterator, T> {
    iterator(std::coroutine_handle<promise_type> handle)
        : handle_(std::move(handle)) {}
    iterator() : handle_{} {}
    std::coroutine_handle<promise_type> handle_;
  };

  view_generator(std::coroutine_handle<promise_type> h)
      : handle_(std::move(h)) {}

  auto begin() const { return iterator(handle_); }
  auto end() const { return iterator(); }

  std::coroutine_handle<promise_type> handle_ = {};
};
