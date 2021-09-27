#ifndef UTILITY_HPP
#define UTILITY_HPP
#include <iostream>

namespace util
{
  using std::size_t;

  template <typename Result, typename ... Args>
  using function_t = auto (Args ...) -> Result;

  template <typename T, size_t N>
  using array_t = T[N];

  template <typename CharT>
  inline constexpr auto read_all (std::basic_istream <CharT> & stream)
  {
    return std::basic_string <CharT> {std::istreambuf_iterator <CharT> {stream}, std::istreambuf_iterator <CharT> {}};
  }
}

#endif
