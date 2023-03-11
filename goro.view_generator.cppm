module;

#include <boost/stl_interfaces/iterator_interface.hpp>

#include <coroutine>
#include <ranges> // std::ranges::view_interface

export module goro.view_generator;

import goro.lazy_cache_promise;
import goro.input_iterator_coroutine_adapter;

namespace goro {
// This class is a generator coroutine and a ranges view.
export template <typename ValueType>
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
} // namespace goro
