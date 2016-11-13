#include <operator.hpp>
#include <iostream>

auto main () -> int {
  using namespace gomi;
  {
    constexpr auto a = plus_t {} (123 , 321);
    static_assert (a == 444 , "");
  }
  {
    std::string s;
    add_t {} (assign_t {} (s , "hello, ") , "hoge");
    std::cout << s << std::endl;
    std::cout << noexcept_t {} (s) << std::endl;
    std::cout << sizeof_t {} (s) << std::endl;
  }
}
