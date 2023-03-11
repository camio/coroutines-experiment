module;

#include <boost/stl_interfaces/iterator_interface.hpp> // BOOST_STL_INTERFACES_STATIC_ASSERT_CONCEPT

#include <coroutine>
#include <iterator> // std::input_iterator
#include <utility>  // std::move

export module goro.input_iterator_generator;
import goro.input_iterator_coroutine_adapter;
import goro.lazy_cache_promise;

namespace goro {

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

BOOST_STL_INTERFACES_STATIC_ASSERT_CONCEPT(input_iterator_generator<int>,
                                           std::input_iterator)

} // namespace goro
