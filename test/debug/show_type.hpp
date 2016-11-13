#ifndef DEBUG_SHOW_TYPE_HPP
#define DEBUG_SHOW_TYPE_HPP
#include <cstdlib>
#include <string>
#include <cxxabi.h>

template <typename>
struct type_holder {};

namespace debug {
  template <typename T>
  auto show_type () {
    int status;
    auto name = typeid (type_holder <T>).name ();
    auto realname = abi::__cxa_demangle (name , 0 , 0 , &status);
    std::string res {realname};
    std::free (realname);
    return std::string {res.begin () + 12 , res.end () - 1};
  }
} // namespace debug

#endif // DEBUG_SHOW_TYPE_HPP
