module;

#include <boost/stl_interfaces/iterator_interface.hpp>

#include <cassert>

export module goro.input_iterator_coroutine_adapter;

import goro.lazy_cache_promise;

using boost::stl_interfaces::iterator_interface;

namespace goro {

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

} // namespace goro
