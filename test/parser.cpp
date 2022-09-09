#include <chino/parser/utf8.hpp>
#include <chino/utf8.hpp>
#include <chino/char_utils.hpp>
#include <chino/utf8/string_reader.hpp>
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
  /* using namespace chino::parser::utf8; */
  using chino::utf8::StringReader;
  USING_CHINO_PARSER_UTF8_COMBINATORS (StringReader);
  using namespace chino::char_utils;

  inline constexpr auto keyword = [] (std::u8string_view str) constexpr noexcept
  {
    return and_ (string (str), negative_lookahead (character_if (unicode::is_XID_CONTINUE)));
  };

  inline auto from_token (const std::tuple <StringReader::Position, std::u8string_view> & token) noexcept
  {
    using chino::parser::utf8::from_string_view;
    auto && [pos, str] = token;
    auto res = from_string_view <double> (str);
    if (is_success (res))
    {
      auto value = get_success (std::move (res));
      return chino::parser::result::success {std::tuple {std::move (pos), std::move (str), std::move (value)}};
    }
    else
    {
      std::cerr << "conversion failed:\n  position = " << pos.line << ":" << pos.col << "\n  str = " << str << "\n";
      return chino::parser::result::success {std::tuple {std::move (pos), std::move (str), std::bit_cast <double> (0x7FF8000000000000)}};
    }
  }



  namespace detail
  {
    inline constexpr auto is_identifier_head = [] (char32_t c) constexpr noexcept {
      return c == U'_' || unicode::is_XID_START (c);
    };
    inline constexpr auto is_identifier_tail = unicode::is_XID_CONTINUE;

    inline constexpr auto digits = repeat (character_if (ascii::is_digit), 1);
    inline constexpr auto hex_digits = repeat (character_if (ascii::is_hex_digit), 1);

    inline constexpr auto identifier = and_ (
      character_if (is_identifier_head),
      repeat (character_if (is_identifier_tail))
    );

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

    inline constexpr Range simple_escape_sequence_characters[] = {
      {U'\"', U'\"'},
      {U'\'', U'\''},
      {U'?', U'?'},
      {U'\\', U'\\'},
      {U'a', U'b'},
      {U'f', U'f'},
      {U'n', U'n'},
      {U'r', U'r'},
      {U't', U't'},
      {U'v', U'v'},
    };
    inline constexpr auto is_simple_escape_sequence_characters = contains {simple_escape_sequence_characters};
    inline constexpr auto escape_sequence = and_ (
      character (U'\\'),
      or_ (
        character_if (is_simple_escape_sequence_characters),
        and_ (character (U'u'), repeat (character_if (ascii::is_hex_digit), 4, 4))
      )
    );
    inline constexpr auto quoted_string = [] (char32_t quot) constexpr noexcept {
      return and_ (
        character (quot),
        repeat (
          or_ (
            and_ (
              negative_lookahead (character (U'\\')),
              negative_lookahead (character (quot)),
              negative_lookahead (character_if (ascii::is_endline)),
              any_character
            ),
            escape_sequence
          )
        ),
        or_ (
          character (quot),
          [] (StringReader &) noexcept { return chino::parser::result::failure {std::string_view {"文字列定数が閉じていません"}}; }
        )
      );
    };
    inline constexpr auto string_literal = quoted_string (U'"');

    inline constexpr auto bool_literal = or_ (
      keyword (u8"true"),
      keyword (u8"false")
    );
  }

  inline constexpr auto int_literal = tokenize (detail::digits);
  inline constexpr auto double_literal = flat_map (
    tokenize (detail::floating_point),
    from_token
  );
  inline constexpr auto string_literal = tokenize (detail::string_literal);
  inline constexpr auto bool_literal = tokenize (detail::bool_literal);
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
  using chino::utf8::StringReader;

  std::u8string source {u8"1e309.0ebcd"};
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
    std::cout << "line = " << line << ", col = " << col << ", value = " << value;
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
