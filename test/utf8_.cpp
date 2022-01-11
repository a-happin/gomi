#include <iostream>
#include <fstream>
#include <chrono>
#include <chino/utf8.hpp>

auto main () -> int
{
  std::ifstream fin ("./include/chino/utf8.hpp");
  std::u8string str {std::istreambuf_iterator <char> {fin}, std::istreambuf_iterator <char> {}};
  auto start = std::chrono::system_clock::now ();
  for (auto i = 0; i < 100000; ++ i)
  {
    chino::utf8::validate (str);
  }
  auto is_valid = chino::utf8::validate (str) == str.length ();
  auto end = std::chrono::system_clock::now ();
  auto elapsed = std::chrono::duration_cast <std::chrono::milliseconds> (end - start).count ();
  std::cout << is_valid << std::endl;
  std::cout << elapsed << std::endl;
}
