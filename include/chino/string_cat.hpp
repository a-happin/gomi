#ifndef CHINO_STRING_CAT_HPP
#define CHINO_STRING_CAT_HPP
#include <sstream>

namespace chino
{
  template <typename CharT = char, typename ... Ts>
  requires requires (std::basic_ostringstream <CharT> & ss, Ts && ... xs)
  {
    {(ss << ... << std::forward <Ts> (xs))};
  }
  inline auto string_cat (Ts && ... xs) -> std::basic_string <CharT>
  {
    std::basic_ostringstream <CharT> ss;
    (ss << ... << std::forward <Ts> (xs));
    return move (ss).str ();
  }
}

#endif
