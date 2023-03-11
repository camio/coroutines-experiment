module;

#include <cassert>
#include <coroutine>
#include <variant>

export module goro.lazy_cache_promise;

namespace goro {

export template <typename ValueType, typename Coroutine>
class lazy_cache_promise {
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

} // namespace goro
