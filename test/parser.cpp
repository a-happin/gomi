#include <chino/parser/utf8_parsers.hpp>
#include <iostream>
#include <variant>
#include <string>
#include <charconv>

inline auto operator << (std::ostream & stream, const std::u8string_view & str) -> decltype (auto)
{
  return stream << std::string_view {reinterpret_cast <const char *> (str.data ()), reinterpret_cast <const char *> (str.data () + str.length ())};
}

inline auto operator << (std::ostream & stream, const char32_t &c) -> decltype (auto)
{
  return chino::utf8::print_as_utf8 (stream, c);
}

namespace parser
{
  using namespace chino::parser::utf8_parsers;
  using namespace chino::parser;
  using namespace chino::char_utils;

  inline constexpr auto digits = characters_more (is_digit);

  inline constexpr auto parse_int = flat_map (
    characters_more (is_digit),
    from_chars <double>
  );

  inline constexpr auto generic_floating_point = flat_map (
    and_ (digits, character (U'.'), digits),
    rejoin
  );

  inline constexpr auto = 
}

auto main () -> int
{
  using namespace chino::parser::utf8_parsers;
  std::u8string source {u8"123.0abcd"};
  StringReader input {source};
  std::vector <std::string> errors;
  auto res = parser::floating_point (input);
  /* static_assert (std::is_same_v <decltype (res), chino::parser::result::result_t <std::variant <char32_t, int>, std::string>>); */
  /* static_assert (std::is_same_v <decltype (res), chino::parser::result::result_t <std::tuple <char32_t, char32_t, char32_t, int>, chino::parser::never>>); */
  static_assert (std::is_same_v <decltype (res), chino::parser::result::result_t <std::u8string_view, chino::parser::never>>);
  if (is_success (res))
  {
    std::cout << "Success: ";
    std::cout << get_success (res);
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
  std::cout << "Rest: \"" << input.as_str () << "\"" << std::endl;
}
