#ifndef STRING_CAT_HPP
#define STRING_CAT_HPP
#include <sstream>

namespace chino
{
  template <typename CharT = char, typename ... Ts>
  inline auto string_cat (Ts && ... strs)
  {
    std::basic_ostringstream <CharT> ss;
    (ss << ... << std::forward <Ts> (strs));
    return move (ss).str ();
  }
}

#endif
