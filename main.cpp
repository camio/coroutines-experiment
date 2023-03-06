#include <algorithm>
#include <coroutine>
#include <iostream>
#include <ranges>
#include <vector>

import corotest;

input_iterator_generator<int> f() {
  std::cout << "f starts. About to yield 1" << std::endl;
  co_yield 1;
  std::cout << "About to yield 2" << std::endl;
  co_yield 2;
  std::cout << "About to exit" << std::endl;
}

view_generator<int> g() {
  std::cout << "f starts. About to yield 1" << std::endl;
  co_yield 1;
  std::cout << "About to yield 2" << std::endl;
  co_yield 2;
  std::cout << "About to exit" << std::endl;
}

template <class T, class A>
class VectorView : public std::ranges::view_interface<VectorView<T, A>> {
public:
  VectorView() = default;
  VectorView(const std::vector<T, A> &vec)
      : m_begin(vec.cbegin()), m_end(vec.cend()) {}
  auto begin() const { return m_begin; }
  auto end() const { return m_end; }

private:
  typename std::vector<T, A>::const_iterator m_begin{}, m_end{};
};

int main() {
  /*
  auto i = f();
  std::cout << "==> begin " << std::endl;
  i == input_iterator_generator<int>{};
  std::cout << "==> get value " << std::endl;
  auto v = *i;
  std::cout << "==> " << v << std::endl;
  */

  for (auto i = f(); i != input_iterator_generator<int>{}; ++i)
    std::cout << "==> " << *i << std::endl;

  std::vector<int> v;
  std::copy(f(), input_iterator_generator<int>{}, std::back_inserter(v));
  for (auto x : v)
    std::cout << x << std::endl;

  auto r = g();
  for( auto i = r.begin(); i != r.end(); ++i)
    std::cout << *i << std::endl;

  //     for (auto x : g() )
  //       std::cout << x << std::endl;

  // VectorView<int, std::allocator<int>> gah;
}
