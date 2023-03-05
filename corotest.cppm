module;

#include <boost/stl_interfaces/iterator_interface.hpp>

#include <cassert>
#include <coroutine>
#include <iostream>
#include <variant>

export module corotest;

using boost::stl_interfaces::iterator_interface;

export template <typename T>
struct generator
    : iterator_interface<generator<T>, std::input_iterator_tag, T> {

  struct promise_type {
    struct NotStarted{
      bool operator==(const NotStarted &) const = default;
    };
    struct HasValue {
      bool operator==(const HasValue &) const = default;
      T value;
    };
    struct Done {
      bool operator==(const Done &) const = default;
    };
    std::variant<NotStarted, HasValue, Done> state_ = NotStarted{};

    generator get_return_object() {
      return {std::coroutine_handle<promise_type>::from_promise(*this)};
    }
    std::suspend_always initial_suspend() {
      std::cout << "initial_suspend" << std::endl;
      return {};
    }
    std::suspend_never final_suspend() noexcept {
      state_ = Done{};
      return {};
    }
    void unhandled_exception() {}
    std::suspend_always yield_value(T value) {
      state_ = HasValue{std::move(value)};
      return {};
    }
  };

 private: generator(std::coroutine_handle<promise_type> h) : handle_(std::move(h)) {
  }
 public:

  T operator*() const {
    assert( handle_ );
    struct Visitor {
      std::coroutine_handle<promise_type> * const handle_;
      T operator()( const typename promise_type::NotStarted & ) const {
        handle_->resume();

        typename promise_type::HasValue * v = std::get_if< typename promise_type::HasValue >( &handle_->promise().state_ );
        if( v )
          return v->value;
        else
          std::abort(); // Dereference of a past-the-end iterator
      }
      T operator()(const typename promise_type::HasValue & v) const {
        return v.value;
      }
      T operator()(const typename promise_type::Done &) const {
        std::abort(); // Dereference of a past-the-end iterator
      }
    };

    return std::visit(Visitor{&handle_}, handle_.promise().state_);
  }
  generator& operator++()  {
    assert(handle_);
    **this;           // Ensure the iterator position has been consumed
    handle_.resume(); // Move to the next position
    return *this;
  }
  friend bool operator==(generator lhs, generator rhs)
  {
    if( lhs.handle_ ) {
      if (rhs.handle_) return false;
      else return lhs.past_the_end();
    }
    else {
      if (rhs.handle_)
        return rhs.past_the_end();
      else
        return true;
    }
  }
  generator(){}
 private:
   bool past_the_end() const {
    assert(handle_);
    struct Visitor {
      std::coroutine_handle<promise_type> *const handle_;
      bool operator()(const typename promise_type::NotStarted &) const {
        handle_->resume();

        return std::get_if<typename promise_type::Done>(
                   &handle_->promise().state_) != nullptr;
      }
      bool operator()(const typename promise_type::HasValue &v) const {
        return false;
      }
      bool operator()(const typename promise_type::Done &) const {
        return true;
      }
    };

    return std::visit(Visitor{&handle_}, handle_.promise().state_);
   }

  mutable std::coroutine_handle<promise_type> handle_ = {};
};
