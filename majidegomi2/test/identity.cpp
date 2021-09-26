#include <identity.hpp>
#include "debug/show_type.hpp"
#include <iostream>

auto main () -> int {
  using namespace gomi;
  using a = Identity <const volatile int &>;
  std::cout << debug::show_type <a> () << std::endl;
  std::cout << debug::show_type <a::type> () << std::endl;
  static_assert (std::is_same <a::type , const volatile int &> {} , "");
}
