#include <algorithm>
#include <coroutine>
#include <iostream>

import corotest;

generator<int> f() {
  std::cout << "f starts. About to yield 1" << std::endl;
  co_yield 1;
  std::cout << "About to yield 2" << std::endl;
  co_yield 2;
  std::cout << "About to exit" << std::endl;
 }

 int main() {
   for( auto i = f(); i != generator<int>{}; ++i )
     std::cout << *i << std::endl;

   std::vector<int> v;
   std::copy(f(), generator<int>{}, std::back_inserter(v));
   for(auto x : v)
     std::cout << x << std::endl;
 }
