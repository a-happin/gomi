#ifndef CHINO_UTILITY_HPP
#define CHINO_UTILITY_HPP
#include <iostream>
#include <tuple>

namespace chino
{
  using std::size_t;

  template <typename Result, typename ... Args>
  using function_t = auto (Args ...) -> Result;

  template <typename F, typename Fn>
  concept Function = requires (F && f, typename function_traits <Fn>::Args t)
  {
    {std::apply (std::forward <F> (f), std::move (t))} -> std::same_as <typename function_traits <Fn>::Result>;
  };
  static_assert (Function <auto (int &&, int, int, int, const int&) -> int, auto (int, int &, int &&, char, int) -> int>);

  template <typename T, size_t N>
  using array_t = T[N];

  template <typename CharT>
  inline constexpr auto read_all (std::basic_istream <CharT> & stream)
  {
    return std::basic_string <CharT> {std::istreambuf_iterator <CharT> {stream}, std::istreambuf_iterator <CharT> {}};
  }

  inline auto omajinai ()
  {
    std::cin.tie (nullptr);
    std::ios::sync_with_stdio (false);
    std::cout << std::boolalpha;
  }
}

#endif
