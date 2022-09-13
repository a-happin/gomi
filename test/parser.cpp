#include <chino/parser/utf8.hpp>
#include <chino/utf8.hpp>
#include <chino/char_utils.hpp>
#include <chino/utf8/string_reader.hpp>
#include <chino/string_cat.hpp>
#include <chino/ansi.hpp>
#include <iostream>
#include <fstream>
#include <variant>
#include <string>
#include <list>
#include <charconv>
#define dump(...) std::cout << #__VA_ARGS__ << " = " << (__VA_ARGS__) << "\n"
#ifndef dump
#endif

inline auto operator << (std::ostream & stream, const std::u8string_view & str) -> decltype (auto)
{
  return stream << std::string_view {reinterpret_cast <const char *> (str.data ()), reinterpret_cast <const char *> (str.data () + str.length ())};
}

inline auto operator << (std::ostream & stream, const char32_t & c) -> decltype (auto)
{
  return chino::utf8::print_as_utf8 (stream, c);
}

template <typename CharT>
inline auto file_size (std::basic_istream <CharT> & stream) noexcept
{
  auto begin = stream.tellg ();
  stream.seekg (0, std::ios::end);
  auto end = stream.tellg ();
  auto size = static_cast <std::size_t> (end - begin);
  stream.seekg (begin);
  return size;
}

inline auto read_file (std::string_view path)
{
  if (std::ifstream in {std::string {path}, std::ios::in | std::ios::binary | std::ios::ate}; in)
  {
    auto file_size = in.tellg ();
    std::u8string contents (static_cast <std::u8string::size_type> (file_size), u8'\0');
    in.seekg (0, std::ios::beg);
    in.read (reinterpret_cast <char *> (contents.data ()), file_size);
    return contents;
  }
  else
  {
    throw std::runtime_error {chino::string_cat ("cannot open file: ", path)};
  }
}

namespace lexer
{
  struct Source
  {
    std::string_view path;
    std::u8string str;

    explicit Source (std::string_view path_)
      : path {path_}
      , str {read_file (path)}
    {
    }
  };

  struct StringReader : chino::utf8::StringReader
  {
    using super = chino::utf8::StringReader;
    struct Position
    {
      const Source * source_ptr;
      chino::utf8::StringReader::Position pos;

      friend auto operator << (std::ostream & stream, const Position & p) -> decltype (auto)
      {
        return stream << "in \"" << p.source_ptr->path << "\" at " << p.pos;
      }
    };
    struct Error
    {
      Position pos;
      std::string message;

      friend auto operator << (std::ostream & stream, const Error & e) -> decltype (auto)
      {
        return stream << chino::ansi::red << "Error» " << e.pos << ": " << e.message << chino::ansi::reset;
      }
    };

    const Source * source_ptr;
    std::list <Error> * errors_ptr;

    explicit StringReader (Source & source, std::list <Error> & errors)
      : super (source.str)
      , source_ptr {&source}
      , errors_ptr {&errors}
    {
    }

    constexpr auto position () const noexcept
    {
      return Position {source_ptr, super::position ()};
    }
  };


  USING_CHINO_PARSER_UTF8_COMBINATORS (StringReader);
  using namespace chino::char_utils;
  namespace result = chino::parser::result;

  inline constexpr auto keyword = [] (std::u8string_view str) constexpr noexcept
  {
    return and_ (string (str), negative_lookahead (character_if (unicode::is_XID_CONTINUE)));
  };

  inline constexpr auto raise = [] (std::string_view message) constexpr noexcept
  {
    return [message] (StringReader &) constexpr noexcept
    {
      return result::failure {std::string {message}};
    };
  };

  inline constexpr auto recover = [] (std::string && message, StringReader & input)
  {
    input.errors_ptr->push_back (StringReader::Error {input.position (), std::move (message)});
  };

  inline auto from_token = [] <typename Position> (std::tuple <Position, std::u8string_view> && token, StringReader & input) noexcept
  {
    using chino::parser::utf8::from_string_view;
    auto && [pos, str] = token;
    if (auto res = from_string_view <double> (str); is_success (res))
    {
      auto value = get_success (std::move (res));
      return chino::parser::result::success {std::tuple {std::move (pos), std::move (str), std::move (value)}};
    }
    else
    {
      recover ("conversion to double failed", input);
      /* std::cerr << "Error: " << pos << ": conversion failed.\n"; */
      return chino::parser::result::success {std::tuple {std::move (pos), std::move (str), std::bit_cast <double> (0x7FF8000000000000)}};
    }
  };



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
          catch_error (raise ("文字列定数が閉じていません"), [] <typename Error> (Error && e, StringReader & input) noexcept -> result::result <chino::never, chino::never> { recover (std::forward <Error> (e), input); return {}; }),
          and_ ()
        )
      );
    };
    inline constexpr auto string_literal = quoted_string (U'"');

    inline constexpr auto bool_literal = or_ (
      keyword (u8"true"),
      keyword (u8"false")
    );

    inline constexpr auto null_literal = keyword (u8"null");
  }

  inline constexpr auto int_literal = tokenize (detail::digits);
  inline constexpr auto double_literal = flat_map (
    tokenize (detail::floating_point),
    from_token
  );
  inline constexpr auto string_literal = tokenize (detail::string_literal);
  inline constexpr auto bool_literal = tokenize (detail::bool_literal);
  inline constexpr auto null_literal = tokenize (detail::null_literal);
}

namespace parser
{
  using lexer::StringReader;
  USING_CHINO_PARSER_COMBINATORS (StringReader);

  inline constexpr auto literal = or_ (
    lexer::double_literal,
    lexer::string_literal,
    lexer::bool_literal,
    lexer::null_literal
  );
}

auto main () -> int
{
  using lexer::Source;
  using lexer::StringReader;

  /* std::u8string source {u8"\"hello\a\n1e309.0ebcd)"}; */
  Source source {"example.txt"};
  std::list <StringReader::Error> errors;
  StringReader input {source, errors};

  /* auto res = parser::literal (input); */
  auto res = lexer::repeat (
    lexer::character_unless (chino::char_utils::unicode::is_XID_CONTINUE)
  )
  (input);
  /* auto res = chino::parser::utf8::eof <StringReader> (input); */

  /* static_assert (std::is_same_v <decltype (res), chino::parser::result::result_t <std::variant <char32_t, int>, std::string>>); */
  /* static_assert (std::is_same_v <decltype (res), chino::parser::result::result_t <std::tuple <char32_t, char32_t, char32_t, int>, chino::parser::never>>); */
  /* static_assert (std::is_same_v <decltype (res), chino::parser::result::result <std::u8string_view, never>>); */
  for (auto && elem : errors)
  {
    std::cout << elem << "\n";
  }
  if (is_success (res))
  {
    auto && value = get_success (res);
    std::cout << "Success: ";
    /* std::visit (chino::overload { */
    /*   [] <typename Position> (const std::tuple <Position, std::u8string_view, double> & t) */
    /*   { */
    /*     auto && [pos, str, v] = t; */
    /*     std::cout << "position = " << pos << ", str = " << str << ", value = " << v; */
    /*   }, */
    /*   [] <typename Position> (const std::tuple <Position, std::u8string_view> & t) */
    /*   { */
    /*     auto && [pos, str] = t; */
    /*     std::cout << "position = " << pos << ", str = " << str; */
    /*   } */
    /* }, value); */
    std::cout << std::endl;
  }
  else if (is_failure (res))
  {
    std::cout << "Failure: ";
    auto && e = get_failure (res);
    static_cast <void> (e);
    /* static_assert (std::is_same_v <decltype (e), void>); */
    /* std::cout << e << "\n"; */
    /* std::cout << get_failure (res) << std::endl; */
    std::cout << "\n";
  }
  else
  {
    std::cout << "None\n";
  }
  std::cout << "Rest: \"" << input.as_str () << "\"\n";
}
