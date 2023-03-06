module;

#include <boost/stl_interfaces/iterator_interface.hpp>

#include <cassert>
#include <coroutine>
#include <ranges>
#include <variant>

export module corotest;

using boost::stl_interfaces::iterator_interface;

template <typename ValueType, typename Coroutine> class lazy_cache_promise {
  struct CacheEmpty {
    bool operator==(const CacheEmpty &) const = default;
  };
  struct CacheFilled {
    bool operator==(const CacheFilled &) const = default;
    ValueType value;
  };
  struct Done {
    bool operator==(const Done &) const = default;
  };
  std::variant<CacheEmpty, CacheFilled, Done> state_ = CacheEmpty{};

public:
  ///////////////////////////////
  // Coroutine promise interface
  ///////////////////////////////
  Coroutine get_return_object() {
    return Coroutine{
        std::coroutine_handle<lazy_cache_promise>::from_promise(*this)};
  }
  std::suspend_always initial_suspend() { return {}; }
  std::suspend_never final_suspend() noexcept {
    state_ = Done{};
    return {};
  }
  void unhandled_exception() {}
  std::suspend_always yield_value(ValueType value) {
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

  const ValueType &get_filled_cache_value() const {
    CacheFilled const *const cacheFilled = std::get_if<CacheFilled>(&state_);
    assert(cacheFilled != nullptr);
    return cacheFilled->value;
  }
  void clear_cache() { state_ = CacheEmpty{}; }
};

// This CRTP class provides an input iterator interface to a class with a
// coroutine handle to a lazy_cache_promise.
//
// Requirements - `Derived` must have
// d.get_coroutine_handle() which returns a value of type
// `std::coroutine_handle<lazy_cache_promise<ValueType, C>>` for some type C.
export template <typename Derived, typename ValueType>
struct input_iterator_coroutine_adapter
    : iterator_interface<Derived, std::input_iterator_tag, ValueType> {
public:
  using base_type =
      iterator_interface<Derived, std::input_iterator_tag, ValueType>;
  const ValueType &operator*() const {
    const auto &h = derived().get_coroutine_handle();
    assert(h);
    try_fill_cache();
    const auto &p = h.promise();
    assert(p.has_filled_cache());
    return p.get_filled_cache_value();
  }
  Derived &operator++() {
    const auto &h = derived().get_coroutine_handle();
    assert(h);
    try_fill_cache();
    auto &p = h.promise();
    assert(!p.is_done());
    p.clear_cache();
    return derived();
  }
  bool operator==(const input_iterator_coroutine_adapter &rhs) const {
    return derived().is_end() == rhs.is_end();
  }

  using base_type::operator++;

private:
  void try_fill_cache() const {
    const auto &h = derived().get_coroutine_handle();
    if (h.promise().has_empty_cache())
      h.resume();
  }
  bool is_end() const {
    const auto &h = derived().get_coroutine_handle();
    if (!h) {
      return true;
    } else {
      try_fill_cache();
      return h.promise().is_done();
    }
  }
  constexpr Derived &derived() noexcept {
    return static_cast<Derived &>(*this);
  }
  constexpr Derived const &derived() const noexcept {
    return static_cast<Derived const &>(*this);
  }
};

// This class is a generator coroutine and an input iterator.
export template <typename ValueType>
struct input_iterator_generator
    : input_iterator_coroutine_adapter<input_iterator_generator<ValueType>,
                                       ValueType> {

  using promise_type = lazy_cache_promise<ValueType, input_iterator_generator>;

  input_iterator_generator() {}

private:
  input_iterator_generator(std::coroutine_handle<promise_type> h)
      : handle_(std::move(h)) {}

  // Required for input_iterator_coroutine_adapter.
  const std::coroutine_handle<promise_type> &get_coroutine_handle() const {
    return handle_;
  }

  friend class input_iterator_coroutine_adapter<input_iterator_generator,
                                                ValueType>;
  friend class lazy_cache_promise<ValueType, input_iterator_generator>;

  std::coroutine_handle<promise_type> handle_ = {};
};

BOOST_STL_INTERFACES_STATIC_ASSERT_CONCEPT(input_iterator_generator<int>, std::input_iterator)

// This class is a generator coroutine and a ranges view.
export
template <typename ValueType>
struct view_generator
    : public std::ranges::view_interface<view_generator<ValueType>> {

  // Coroutine interface
  using promise_type = lazy_cache_promise<ValueType, view_generator>;

  /////////////////
  // View interface
  /////////////////
  class iterator
      : public input_iterator_coroutine_adapter<iterator, ValueType> {
  public:
    iterator() : handle_{} {}
  private:
    iterator(std::coroutine_handle<promise_type> handle)
        : handle_(std::move(handle)) {}
    const std::coroutine_handle<promise_type> &get_coroutine_handle() const {
      return handle_;
    }
    friend class input_iterator_coroutine_adapter<iterator, ValueType>;
    friend class view_generator;
    std::coroutine_handle<promise_type> handle_;
  };

  auto begin() const { return iterator{handle_}; }
  auto end() const { return iterator{}; }

private:
  friend class lazy_cache_promise<ValueType, view_generator>;
  view_generator(std::coroutine_handle<promise_type> h)
      : handle_(std::move(h)) {}

  std::coroutine_handle<promise_type> handle_ = {};
};

BOOST_STL_INTERFACES_STATIC_ASSERT_CONCEPT(view_generator<int>::iterator,
                                           std::input_iterator)
