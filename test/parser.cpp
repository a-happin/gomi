#include <chino/parser.hpp>
#include <iostream>
#include <variant>

inline auto operator << (std::ostream & stream, const std::u8string_view & str) -> decltype (auto)
{
  for (auto && elem : str)
  {
    stream << static_cast <char> (elem);
  }
  return stream;
}

inline auto operator << (std::ostream & stream, const char32_t &c) -> decltype (auto)
{
  return chino::utf8::print_as_utf8 (stream, c);
}

auto main () -> int
{
  std::u8string source{u8"abcd"};
  chino::parser::ParserInput input{source};
  auto parser = chino::parser::and_ (
    chino::parser::char_ (U'a'),
    chino::parser::char_ (U'b'),
    chino::parser::char_ (U'c'),
    chino::parser::map (chino::parser::char_ (U'd'), [dummy=0](auto &&) { return dummy; })
  );
  auto res = parser (input);
  /* static_assert (std::is_same_v <decltype (res), chino::parser::result::result_t <std::variant <char32_t, int>, std::string>>); */
  static_assert (std::is_same_v <decltype (res), chino::parser::result::result_t <std::tuple <char32_t, char32_t, char32_t, int>, chino::parser::never>>);
  if (is_success (res))
  {
    std::cout << "Success: ";
    /* std::visit ([](auto && x) -> decltype (auto) { return std::cout << x; }, get_success (res)); */
    std::cout << std::endl;
  }
  else if (is_failure (res))
  {
    std::cout << "Failure: ";
    /* std::cout << get_failure (res) << std::endl; */
  }
  else
  {
    std::cout << "None" << std::endl;
  }
  std::cout << "Rest: \"" << input.reader.as_str () << "\"" << std::endl;
}
