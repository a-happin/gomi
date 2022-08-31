#include <chino/parser/utf8.hpp>
#include <iostream>
#include <variant>
#include <string>
#include <charconv>

inline auto operator << (std::ostream & stream, const std::u8string_view & str) -> decltype (auto)
{
  return stream << std::string_view {reinterpret_cast <const char *> (str.data ()), reinterpret_cast <const char *> (str.data () + str.length ())};
}

inline auto operator << (std::ostream & stream, const char32_t & c) -> decltype (auto)
{
  return chino::utf8::print_as_utf8 (stream, c);
}


namespace ast
{
  /* struct Token */
  /* { */
  /*   chino::utf8::StringReader::Position pos; */
  /*   std::u8string_view str; */
  /* }; */

  /* struct IntegralLiteral */
  /* { */
  /*   chino::utf8::StringReader::Position pos; */
  /*   std::u8string_view str; */
  /*   uint64_t value; */
  /* }; */
}

namespace lexer
{
  using namespace chino::parser::utf8;
  using namespace chino::char_utils;

  namespace detail
  {
    inline constexpr auto is_identifier_head = [] (char32_t c) constexpr noexcept {
      return is_alpha (c) || c == U'_';
    };
    inline constexpr auto is_identifier_tail = chino::char_utils::is_word;

    inline constexpr auto digits = more (character_if (is_digit));

    inline constexpr auto identifier = and_ (
      character_if (is_identifier_head),
      repeat (character_if (is_identifier_tail))
    );

    inline constexpr auto integral = flat_map (digits, from_string_view <int64_t>);

    inline constexpr auto decimal_exponent = and_ (
      character_if ([] (char32_t c) constexpr noexcept { return c == U'e' || c == U'E'; }),
      optional (character_if ([] (char32_t c) constexpr noexcept { return c == U'-' || c == U'+'; })),
      digits
    );
    inline constexpr auto floating_point = and_ (
      optional (character (U'-')),
      digits,
      optional (and_ (
          character (U'.'),
          digits
      )),
      optional (decimal_exponent)
    );

  }

  inline constexpr auto int_literal = tokenize (detail::integral);
  inline constexpr auto double_literal = tokenize (
    flat_map (detail::floating_point, from_string_view <double>)
  );
}

namespace parser
{
  using namespace chino::parser;
  using namespace chino::char_utils;
}

auto main () -> int
{
  using namespace chino::parser::utf8;
  using chino::never;

  std::u8string source {u8"18123.0abcd"};
  StringReader input {source};
  std::vector <std::string> errors;
  auto res = lexer::double_literal (input);
  /* static_assert (std::is_same_v <decltype (res), chino::parser::result::result_t <std::variant <char32_t, int>, std::string>>); */
  /* static_assert (std::is_same_v <decltype (res), chino::parser::result::result_t <std::tuple <char32_t, char32_t, char32_t, int>, chino::parser::never>>); */
  /* static_assert (std::is_same_v <decltype (res), chino::parser::result::result <std::u8string_view, never>>); */
  if (is_success (res))
  {
    auto && [pos, str, value] = get_success (res);
    auto [line, col] = pos;
    std::cout << "Success: ";
    std::cout << "line = " << line << ", col = " << col << ", str = " << str << ", value = " << value;
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
