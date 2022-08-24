#include <chino/parser/utf8.hpp>
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

namespace lexer
{
  using namespace chino::parser::utf8;
  using namespace chino::char_utils;

  enum class TokenKind : uint_fast64_t
  {};

  struct Token
  {
    TokenKind kind;
    StringReader::Position pos;
    std::u8string_view str;
  };

  inline constexpr auto tokenize = [] (TokenKind kind) constexpr noexcept {
    return [kind = std::move (kind)] (std::tuple <StringReader::Position, std::u8string_view> && res) constexpr noexcept {
      auto && [pos, str] = std::move (res);
      return Token {std::move (kind), std::move (pos), std::move (str)};
    };
  };

  inline constexpr auto is_identifier_head = [] (char32_t c) constexpr noexcept {
    return is_alpha (c) || c == U'_';
  };
  inline constexpr auto is_identifier_tail = chino::char_utils::is_word;

  inline constexpr auto digits = characters_more (is_digit);

  inline constexpr auto identifier = and_ (
    character_if (is_identifier_head),
    characters_while (is_identifier_tail)
  );
}

namespace parser
{

  using namespace chino::parser::utf8;
  using namespace chino::parser;
  using namespace chino::char_utils;

  inline constexpr auto and_ = chino::parser::and_;

  inline constexpr auto digits = characters_more (is_digit);


  inline constexpr auto parse_int = flat_map (
    characters_more (is_digit),
    from_chars <double>
  );

  inline constexpr auto generic_floating_point = flat_map (
    and_ (digits, character (U'.'), digits),
    rejoin
  );

  inline constexpr auto literal_suffix = or_ (
    lexer::identifier,
    epsilon
  );

  inline constexpr auto decimal_exponent = or_ (
    flat_map (
      and_ (
        character_if ([] (char32_t c) { return c == U'e' || c == U'E'; }),
        or_ (
          character_if ([] (char32_t c) { return c == U'-' || c == U'+'; }),
          epsilon
        ),
        digits
      ),
      rejoin
    ),
    epsilon
  );

  inline constexpr auto generic_floating_point_literal = or_ (
    flat_map (and_ (character_opt (U'-'), digits, decimal_exponent), rejoin),
    flat_map (and_ (character_opt (U'-'), digits, character (U'.'), digits, decimal_exponent), rejoin)
  );

  inline constexpr auto floating_point_literal = and_ (
      generic_floating_point_literal,
      literal_suffix
  );
}

auto main () -> int
{
  using namespace chino::parser::utf8;
  std::u8string source {u8"e18123.0abcd"};
  StringReader input {source};
  std::vector <std::string> errors;
  auto res = parser::decimal_exponent (input);
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
