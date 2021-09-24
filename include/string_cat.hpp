#ifndef STRING_CAT_HPP
#define STRING_CAT_HPP
#include <sstream>

template <typename Char = char, typename ... Ts>
inline auto string_cat (Ts && ... strs)
{
  std::basic_ostringstream <Char> ss;
  (ss << ... << std::forward <Ts> (strs));
  return move (ss).str ();
}

#endif
