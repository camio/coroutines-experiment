#include <algorithm>
#include <coroutine>
#include <iostream>

import corotest;

input_iterator_generator<int> f() {
  std::cout << "f starts. About to yield 1" << std::endl;
  co_yield 1;
  std::cout << "About to yield 2" << std::endl;
  co_yield 2;
  std::cout << "About to exit" << std::endl;
}

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
}
