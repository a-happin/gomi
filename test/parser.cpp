#include <chino/parser.hpp>
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
#include <unordered_map>
#include <charconv>
#define FOR_IMPL(i,b,e) for (auto [i, i ## _end] = std::tuple {(b), (e)}; i < i ## _end; ++ i)
#define FOR(...) FOR_IMPL(__VA_ARGS__)
#define rep(i,n) FOR(i,0zu,n)
#define ALL(x) std::ranges::begin (x), std::ranges::end (x)
#define dump(...) std::cout << #__VA_ARGS__ << " = " << (__VA_ARGS__) << "\n"
#define FORWARD(x) std::forward <decltype (x)> (x)
#ifndef dump
#endif

inline constexpr auto NaN = std::bit_cast <double> (0x7FF8000000000000);

inline auto operator << (std::ostream & stream, const std::u8string_view & str) -> decltype (auto)
{
  return stream << std::string_view {reinterpret_cast <const char *> (str.data ()), reinterpret_cast <const char *> (str.data () + str.length ())};
}

inline auto operator << (std::ostream & stream, const char32_t & c) -> decltype (auto)
{
  return chino::utf8::print_as_utf8 (stream, c);
}

template <typename CharT>
[[deprecated]] inline auto file_size (std::basic_istream <CharT> & stream) noexcept
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

template <std::integral T>
inline constexpr auto logical_rshift (T lhs, int rhs) noexcept
{
  return static_cast <T> (static_cast <std::make_unsigned_t <T>> (lhs) >> rhs);
}
static_assert (logical_rshift (std::int32_t {-1}, 1) == 0x7FFFFFFF);

namespace hoge
{
#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wpadded"
#endif
struct JSON : std::variant <std::monostate, bool, double, std::string, std::vector <JSON>, std::unordered_map <std::string, JSON>>
{
  using variant::variant;

  friend auto operator << (std::ostream & stream, const JSON & json) -> decltype (auto)
  {
    return json.print (stream, 2);
  }

  auto stringify (std::size_t spacer = 0) const
  {
    std::ostringstream ss;
    print (ss, spacer);
    return std::move (ss).str ();
  }

private:
  auto print (std::ostream & stream, const std::size_t & spacer) const -> std::ostream &
  {
    std::size_t indent = 0;
    auto impl = [&stream, &spacer, &indent] (auto & self, const JSON & json) -> void
    {
      std::visit (chino::overload {
        [&stream] (const std::monostate &) { stream << "null"; },
        [&stream] (const bool & b) { stream << (b ? "true" : "false"); },
        [&stream] (const double & val) { stream << val; },
        [&stream] (const std::string & str) { stream << '"' << str << '"'; },
        [&self, &stream, &spacer, &indent] (const std::vector <JSON> & arr) noexcept {
          stream << '[';
          indent += spacer;
          for (bool is_first = true; auto && elem : arr)
          {
            if (is_first)
            {
              is_first = false;
            }
            else
            {
              stream << ',';
              if (spacer > 0)
              {
                stream << '\n';
                rep (i, indent)
                {
                  stream << ' ';
                }
              }
            }
            self (self, elem);
          }
          indent -= spacer;
          stream << ']';
        },
        [&self, &stream, &spacer, &indent] (const std::unordered_map <std::string, JSON> & object) noexcept {
          stream << '{';
          indent += spacer;
          for (bool is_first = true; auto && [key, value] : object)
          {
            if (is_first)
            {
              is_first = false;
            }
            else
            {
              stream << ',';
              if (spacer > 0)
              {
                stream << '\n';
                rep (i, indent)
                {
                  stream << ' ';
                }
              }
            }
            stream << '"' << key << '"';
            if (spacer > 0)
            {
              stream << ' ';
            }
            self (self, value);
          }
          indent -= spacer;
          stream << '}';
        },
      }, json);
    };
    impl (impl, * this);
    return stream;
  }
};
#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif
}

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
      return stream << chino::ansi::red << "Error» " << e.pos << ": " << chino::ansi::bold << e.message << chino::ansi::reset;
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

namespace lexer
{
  USING_CHINO_PARSER_UTF8_COMBINATORS (StringReader);
  using namespace chino::char_utils;
  namespace result = chino::parser::result;

  inline constexpr auto keyword = [] (std::u8string_view str) constexpr noexcept
  {
    return and_ (string (str), negative_lookahead (character_if (unicode::is_XID_Continue)));
  };

  inline constexpr auto raise = [] (std::string_view message) constexpr noexcept
  {
    return [message] (StringReader &) constexpr noexcept
    {
      return result::failure {std::string {message}};
    };
  };

  inline constexpr auto recover = [] (std::string && message, StringReader & input) -> result::result <chino::never, chino::never>
  {
    input.errors_ptr->push_back (StringReader::Error {input.position (), std::move (message)});
    return {};
  };

  inline constexpr auto spaces = repeat (character_if (unicode::is_white_space));

  inline constexpr auto wrap_spaces = [] <chino::parser::Parser <StringReader> P> (P p) constexpr noexcept
  {
    return map (
      chino::parser::and_ <StringReader> (spaces, std::move (p), spaces),
      [] (auto && t) constexpr noexcept { return std::get <1> (FORWARD (t)); }
    );
  };

  inline constexpr auto digits = repeat (character_if (ascii::is_digit), 1);
  inline constexpr auto hex_digits = repeat (character_if (ascii::is_hex_digit), 1);

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
        catch_error (raise ("文字列定数が閉じていません"), recover),
        and_ ()
      )
    );
  };
  inline constexpr auto string_literal = wrap_spaces (quoted_string (U'"'));

  inline constexpr auto true_literal = wrap_spaces (keyword (u8"true"));
  inline constexpr auto false_literal = wrap_spaces (keyword (u8"false"));

  inline constexpr auto null_literal = wrap_spaces (keyword (u8"null"));

  inline constexpr auto bracket_begin = wrap_spaces (character (U'['));
  inline constexpr auto bracket_end = wrap_spaces (character (U']'));

  inline constexpr auto brace_begin = wrap_spaces (character (U'{'));
  inline constexpr auto brace_end = wrap_spaces (character (U'}'));

  inline constexpr auto colon = wrap_spaces (character (U':'));
}

namespace ast
{
  struct SourceRange
  {
    using Position = StringReader::Position;
    Position begin, end;
  };

  #ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpadded"
  #endif

  template <typename T>
  struct Located
  {
    SourceRange range;
    T value;

    // なぜか必要
    // 下でusingしてるからじゃん
    /* constexpr Located (SourceRange && range_, T && value_) noexcept */
    /*   : range {std::move (range_)} */
    /*   , value {std::forward <T> (value_)} */
    /* { */
    /* } */
  };
  template <typename T>
  Located (SourceRange &&, T &&) noexcept -> Located <T>;

  template <template <typename> class F>
  struct Fix : Located <F <Fix <F>>>
  {
    /* using Located <F <Fix <F>>>::Located; */
  };

  template <typename T>
  using array_t = std::vector <T>;

  template <typename T>
  using object_t = std::unordered_map <std::u8string, T>;

  template <typename T>
  using JSON_F = std::variant <std::monostate, bool, double, std::u8string, array_t <T>, object_t <T>>;
  using JSON = Fix <JSON_F>;

  #ifdef __GNUC__
    #pragma GCC diagnostic pop
  #endif
}

namespace parser
{
  USING_CHINO_PARSER_COMBINATORS (StringReader);
  using chino::parser::Parser;
  namespace result = chino::parser::result;

  inline constexpr auto located = [] <Parser <StringReader> P> (P p) constexpr noexcept
  {
    return map  (
      and_  (lexer::position, std::move (p), lexer::position),
      [] (auto && t) constexpr noexcept { auto && [p1, value, p2] = FORWARD (t); return ast::Located {ast::SourceRange {FORWARD (p1), FORWARD (p2)}, FORWARD (value)}; }
    );
  };

  inline constexpr auto wrap_spaces = [] <Parser <StringReader> P> (P p) constexpr noexcept
  {
    return map (
      and_ (lexer::spaces, std::move (p), lexer::spaces),
      [] (auto && t) constexpr noexcept { return std::get <1> (FORWARD (t)); }
    );
  };

  inline constexpr auto null_literal = map (
    lexer::null_literal,
    [] (auto &&) constexpr noexcept { return std::monostate {}; }
  );
  static_assert (std::is_same_v <chino::parser::ParserResultT <decltype (null_literal), StringReader>, std::monostate>);

  inline constexpr auto true_literal = map (
    lexer::true_literal,
    [] (auto &&) constexpr noexcept { return true; }
  );
  static_assert (std::is_same_v <chino::parser::ParserResultT <decltype (true_literal), StringReader>, bool>);

  inline constexpr auto false_literal = map (
    lexer::false_literal,
    [] (auto &&) constexpr noexcept { return false; }
  );
  static_assert (std::is_same_v <chino::parser::ParserResultT <decltype (false_literal), StringReader>, bool>);

  inline constexpr auto double_literal = catch_error (
    flat_map (
      lexer::floating_point,
      [] (std::u8string_view str, StringReader &) constexpr noexcept { return chino::parser::utf8::from_string_view <double> (str); }
    ),
    [] (auto &&, StringReader & input) constexpr noexcept
    {
      lexer::recover (std::string {"double型への変換に失敗しました"}, input);
      return result::success {NaN};
    }
  );
  static_assert (std::is_same_v <chino::parser::ParserResultT <decltype (double_literal), StringReader>, double>);

  inline constexpr auto string_literal = map (
    lexer::string_literal,
    [] (std::u8string_view && str) constexpr noexcept { return std::u8string {FORWARD (str)}; }
  );
  static_assert (std::is_same_v <chino::parser::ParserResultT <decltype (string_literal), StringReader>, std::u8string>);

  inline constexpr auto json (StringReader &) -> lexer::result::result <ast::JSON, chino::never>;

  inline constexpr auto array = map (
    and_ (
      lexer::bracket_begin,
      repeat (json),
      or_ (
        lexer::bracket_end,
        catch_error (lexer::raise ("角括弧が閉じていません"), lexer::recover),
        lexer::and_ ()
      )
    ),
    [] (auto && t) constexpr noexcept { return std::get <1> (t); }
  );
  static_assert (std::is_same_v <chino::parser::ParserResultT <decltype (array), StringReader>, ast::array_t <ast::JSON>>);

  inline constexpr auto object = map (
    and_ (
      lexer::brace_begin,
      repeat (and_ (
          lexer::string_literal,
          lexer::colon,
          json
      )),
      or_ (
        lexer::brace_end,
        catch_error (lexer::raise ("波括弧が閉じていません"), lexer::recover),
        lexer::and_ ()
      )
    ),
    [] (auto && t) {
      ast::object_t <ast::JSON> res;
      for (auto && [key, colon, value] : std::get <1> (FORWARD (t)))
      {
        static_assert (std::is_same_v <decltype (key), std::u8string_view>);
        res.emplace (std::u8string {ALL (key)}, FORWARD (value));
      }
      return res;
    }
  );
  static_assert (std::is_same_v <chino::parser::ParserResultT <decltype (object), StringReader>, ast::object_t <ast::JSON>>);

  inline constexpr auto json (StringReader & input) -> lexer::result::result <ast::JSON, chino::never>
  {
    return map (
      located (
        or_ (
          null_literal,
          true_literal,
          false_literal,
          double_literal,
          string_literal,
          array,
          object
        )
      ),
      [] <typename T> (T && t) constexpr noexcept { auto && [range, value] = FORWARD (t); return ast::JSON {FORWARD (range), FORWARD (value)}; }
    ) (input);
  }
}

auto main () -> int
{
  /* std::u8string source {u8"\"hello\a\n1e309.0ebcd)"}; */
  Source source {"test/example.txt"};
  std::list <StringReader::Error> errors;
  StringReader input {source, errors};

  auto res = parser::map (
    parser::and_ (
      parser::wrap_spaces (parser::json),
      parser::or_ (
        lexer::eof,
        parser::catch_error (lexer::raise ("余計な文字が含まれています"), lexer::recover)
      )
    ),
    [] (auto && t) constexpr noexcept { return std::get <0> (FORWARD (t)); }
  ) (input);

  if (not errors.empty ())
  {
    for (auto && elem : errors)
    {
      std::cout << elem << "\n";
    }
    return 1;
  }

  if (is_success (res))
  {
    auto && value = get_success (res);
    static_cast <void> (value);
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
    /*   }, */
    /*   [] <typename Position> (const std::tuple <Position, JSON> & t) */
    /*   { */
    /*     auto && [pos, str] = t; */
    /*     std::cout << "position = " << pos << ", str = " << str; */
    /*   } */
    /* }, value); */
    /* auto && [pos, str] = value; */
    /* std::cout << "position = " << pos << ", str = " << str; */
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
