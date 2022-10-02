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
#define FOR(...) FOR_IMPL (__VA_ARGS__)
#define rep(i,n) FOR (i, 0zu, n)
#define ALL(x) std::ranges::begin (x), std::ranges::end (x)
#define dump(...) std::cout << #__VA_ARGS__ << " = " << (__VA_ARGS__) << "\n"
#define FORWARD(x) std::forward <decltype (x)> (x)
#if defined(dump) && defined(rep) && defined(FOR_IMPL) && defined(FOR)
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

struct StringReader : chino::utf8::UTF8StringReader
{
  using super = chino::utf8::UTF8StringReader;
  struct Position
  {
    const Source * source_ptr;
    chino::utf8::UTF8StringReader::Position pos;

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
    : super (std::u8string_view {source.str})
    , source_ptr {&source}
    , errors_ptr {&errors}
  {
  }

  constexpr auto position () const noexcept
  {
    return Position {source_ptr, super::position ()};
  }
};

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

template <typename T>
inline constexpr auto constructor = [] <typename ... Args> (Args && ... args) constexpr noexcept
{
  return T {std::forward <Args> (args) ...};
};

template <typename T>
inline constexpr auto construct_from_tuple = [] <typename Tuple> (Tuple && t) constexpr noexcept
{
  return std::apply (constructor <T>, std::forward <Tuple> (t));
};

template <std::size_t i>
inline constexpr auto get_fn = [] <typename Tuple> (Tuple && t) constexpr noexcept -> decltype (auto)
{
  return std::get <i> (std::forward <Tuple> (t));
};

template <std::size_t ... indices>
inline constexpr auto subtuple = [] <typename Tuple> (Tuple && t) constexpr noexcept
{
  return std::tuple {std::get <indices> (std::forward <Tuple> (t)) ...};
};

namespace parser
{
  using namespace chino::parser;
  /* using chino::parser::map, chino::parser::flat_map, chino::parser::catch_error, chino::parser::and_, chino::parser::or_, chino::parser::lookahead, chino::parser::negative_lookahead, chino::parser::optional, chino::parser::repeat, chino::parser::separated; */
  /* namespace result = chino::parser::result; */
  template <typename T>
  concept Parser = chino::parser::Parser <T, StringReader>;

  inline constexpr auto raise = [] (std::string_view message) constexpr noexcept
  {
    return [message] (StringReader &) constexpr noexcept
    {
      return result::failure {std::string {message}};
    };
  };

  inline constexpr auto recover = [] (std::string && message, StringReader & input) -> result::undefined
  {
    input.errors_ptr->push_back (StringReader::Error {input.position (), std::move (message)});
    return {};
  };

  namespace lexer
  {
    /* using namespace chino::parser::utf8; */
    using chino::parser::utf8::map, chino::parser::utf8::flat_map, chino::parser::utf8::catch_error, chino::parser::utf8::and_, chino::parser::utf8::or_, chino::parser::utf8::lookahead, chino::parser::utf8::negative_lookahead, chino::parser::utf8::optional, chino::parser::utf8::repeat, chino::parser::utf8::position, chino::parser::utf8::character_if, chino::parser::utf8::any_character, chino::parser::utf8::eof, chino::parser::utf8::character, chino::parser::utf8::character_unless, chino::parser::utf8::string;
    using namespace chino::char_utils;

    inline constexpr auto keyword = [] (std::u8string_view str) constexpr noexcept
    {
      return and_ (string (str), negative_lookahead (character_if (unicode::is_XID_Continue)));
    };

    inline constexpr Parser auto spaces = repeat <> (character_if (unicode::is_white_space));

    inline constexpr auto wrap_spaces = [] <Parser P> (P && p) constexpr noexcept
    {
      return map (
        parser::and_ (spaces, std::forward <P> (p), spaces),
        get_fn <1>
      );
    };

    inline constexpr Parser auto digits = repeat <1> (character_if (ascii::is_digit));
    inline constexpr Parser auto hex_digits = repeat <1> (character_if (ascii::is_hex_digit));

    inline constexpr Parser auto decimal_exponent = and_ (
      character_if ([] (char32_t c) constexpr noexcept { return c == U'e' || c == U'E'; }),
      optional (character_if ([] (char32_t c) constexpr noexcept { return c == U'-' || c == U'+'; })),
      digits
    );

    inline constexpr Parser auto floating_point = and_ (
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
    inline constexpr Parser auto escape_sequence = and_ (
      character (U'\\'),
      or_ (
        character_if (is_simple_escape_sequence_characters),
        and_ (character (U'u'), repeat <4, 4> (character_if (ascii::is_hex_digit)))
      )
    );
    inline constexpr auto quoted_string = [] (char32_t quot) constexpr noexcept {
      return and_ (
        character (quot),
        repeat <> (
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
    inline constexpr Parser auto string_literal = wrap_spaces (quoted_string (U'"'));

    inline constexpr Parser auto true_literal = wrap_spaces (keyword (u8"true"));
    inline constexpr Parser auto false_literal = wrap_spaces (keyword (u8"false"));

    inline constexpr Parser auto null_literal = wrap_spaces (keyword (u8"null"));

    inline constexpr Parser auto bracket_begin = wrap_spaces (character (U'['));
    inline constexpr Parser auto bracket_end = wrap_spaces (character (U']'));

    inline constexpr Parser auto brace_begin = wrap_spaces (character (U'{'));
    inline constexpr Parser auto brace_end = wrap_spaces (character (U'}'));

    inline constexpr Parser auto colon = wrap_spaces (character (U':'));
    inline constexpr Parser auto comma = wrap_spaces (character (U','));
  }

  inline constexpr auto located = [] <Parser P> (P && p) constexpr noexcept
  {
    return map  (
      and_  (lexer::position, std::forward <P> (p), lexer::position),
      [] (auto && t) constexpr noexcept { auto && [p1, value, p2] = FORWARD (t); return ast::Located {ast::SourceRange {FORWARD (p1), FORWARD (p2)}, FORWARD (value)}; }
    );
  };

  inline constexpr Parser auto null_literal = map (
    lexer::null_literal,
    [] (auto &&) constexpr noexcept { return std::monostate {}; }
  );
  static_assert (std::is_same_v <chino::parser::success_type <decltype (null_literal), StringReader>, std::monostate>);

  inline constexpr Parser auto true_literal = map (
    lexer::true_literal,
    [] (auto &&) constexpr noexcept { return true; }
  );
  static_assert (std::is_same_v <chino::parser::success_type <decltype (true_literal), StringReader>, bool>);

  inline constexpr Parser auto false_literal = map (
    lexer::false_literal,
    [] (auto &&) constexpr noexcept { return false; }
  );
  static_assert (std::is_same_v <chino::parser::success_type <decltype (false_literal), StringReader>, bool>);

  inline constexpr Parser auto double_literal = catch_error (
    flat_map (
      lexer::floating_point,
      [] (std::u8string_view str, StringReader &) constexpr noexcept { return chino::parser::utf8::from_string_view <double> (str); }
    ),
    [] (auto &&, StringReader & input) constexpr noexcept
    {
      recover (std::string {"double型への変換に失敗しました"}, input);
      return result::success {NaN};
    }
  );
  static_assert (std::is_same_v <chino::parser::success_type <decltype (double_literal), StringReader>, double>);

  inline constexpr Parser auto string_literal = map (
    lexer::string_literal,
    [] (std::u8string_view && str) constexpr noexcept { return std::u8string {FORWARD (str)}; }
  );
  static_assert (std::is_same_v <chino::parser::success_type <decltype (string_literal), StringReader>, std::u8string>);

  inline constexpr auto json (StringReader &) -> result::optional <ast::JSON>;

  inline constexpr Parser auto array = map (
    and_ (
      lexer::bracket_begin,
      separated (json, lexer::comma),
      or_ (
        lexer::bracket_end,
        catch_error (raise ("角括弧が閉じていません"), recover),
        lexer::and_ ()
      )
    ),
    get_fn <1>
  );
  static_assert (std::is_same_v <chino::parser::success_type <decltype (array), StringReader>, ast::array_t <ast::JSON>>);

  inline constexpr Parser auto object = map (
    and_ (
      lexer::brace_begin,
      separated (and_ (
          lexer::string_literal,
          lexer::colon,
          json
      ), lexer::comma),
      or_ (
        lexer::brace_end,
        catch_error (raise ("波括弧が閉じていません"), recover),
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
  static_assert (std::is_same_v <chino::parser::success_type <decltype (object), StringReader>, ast::object_t <ast::JSON>>);

  inline constexpr auto json (StringReader & input) -> result::optional <ast::JSON>
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
      constructor <ast::JSON>
    ) (input);
  }
}

auto main () -> int
{
  /* std::u8string source {u8"\"hello\a\n1e309.0ebcd)"}; */
  Source source {"test/example.txt"};
  std::list <StringReader::Error> errors;
  StringReader input {source, errors};

  std::cout << "parser size = " << sizeof (parser::object) << std::endl;

  auto res = parser::map (
    parser::and_ (
      parser::lexer::wrap_spaces (parser::json),
      parser::or_ (
        parser::lexer::eof,
        parser::catch_error (parser::raise ("余計な文字が含まれています"), parser::recover)
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
