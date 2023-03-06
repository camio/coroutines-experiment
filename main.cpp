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


  std::cout << "***Using the view interface" << std::endl;
  for (auto x : g() )
    std::cout << x << std::endl;
  std::cout << "***/Using the view interface" << std::endl;

  auto y = g();
  std::ranges::begin(y); // verifies that it is a range

  // Also, it would be good to test default constructability.
}
