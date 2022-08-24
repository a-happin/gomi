#ifndef CHINO_PARSER_UTF8_HPP
#define CHINO_PARSER_UTF8_HPP
#include <sstream>
#include <stdexcept>
#include <chino/parser.hpp>
#include <chino/utf8.hpp>
#include <chino/char_utils.hpp>
#include <charconv>

namespace chino::parser::utf8
{
  struct invalid_utf8_error : std::runtime_error {
    invalid_utf8_error (std::ptrdiff_t pos)
      : std::runtime_error {
        [&] () {
          std::ostringstream ss;
          ss << "UTF-8として不正な文字列です\n" << (pos) << "バイト目に不明なバイト列を検出しました";
          return std::move (ss).str ();
        } ()
      }
    {
    }
    ~invalid_utf8_error () noexcept override;
  };
  invalid_utf8_error::~invalid_utf8_error () noexcept = default;

  struct StringReader
  {
    const char8_t * ite;
    const char8_t * end;
    struct Position
    {
      std::size_t line;
      std::size_t col;
      friend constexpr auto operator == (const Position &, const Position &) noexcept -> bool = default;
      friend constexpr auto operator <=> (const Position &, const Position &) noexcept -> std::strong_ordering = default;
    } position;

    constexpr StringReader () noexcept
      : ite {nullptr}
      , end {nullptr}
      , position {1, 1}
    {}

    constexpr StringReader (std::u8string_view str)
      : ite {str.data ()}
      , end {str.data () + str.length ()}
      , position {1, 1}
    {
      if (auto p = chino::utf8::find_invalid (str); p != nullptr)
      {
        throw invalid_utf8_error {str.end () - p + 1};
      }
    }

    inline constexpr auto as_str () const noexcept
    {
      return std::u8string_view {ite, end};
    }

    inline constexpr auto can_read () const noexcept
    {
      return ite < end;
    }

    inline constexpr auto peek () const noexcept
    {
      return chino::utf8::unsafe_codepoint (ite);
    }

    inline constexpr auto next () noexcept -> decltype (auto)
    {
      if (* ite == u8'\n')
      {
        ++ ite;
        ++ position.line;
        position.col = 1;
      }
      else if (* ite == u8'\r')
      {
        ++ ite;
        ++ position.line;
        position.col = 1;
        if (can_read () && * ite == u8'\n')
        {
          ++ ite;
        }
      }
      else
      {
        ite += chino::utf8::char_width (* ite);
        ++ position.col;
      }
      return * this;
    }
  };


  // --------------------------------
  // basic parsers
  // --------------------------------
  inline constexpr auto epsilon = [] (StringReader & input) constexpr noexcept -> result::result_t <std::u8string_view, never>
  {
    return result::success {input.as_str ().substr (0, 0)};
  };


  // --------------------------------
  // 特殊parser combinator
  // --------------------------------
  // 文字列連結に特化したand_
  // and_: (Parser <u8string_view> ...) -> Parser <u8string_view>
  namespace impl
  {
    template <typename E, typename P, typename ... Ps>
    inline constexpr auto and_ (StringReader & input, const char8_t * begin, const char8_t * end, P && p, Ps && ... ps) -> result::result_t <std::u8string_view, E>
    {
      auto res = std::forward <P> (p) (input);
      if (is_success (res))
      {
        auto res_str = get_success (std::move (res));
        if (end != res_str.data ())
        {
          throw std::runtime_error {"rejoin failed: 不正なパーサーが存在します"};
        }

        if constexpr (sizeof ... (Ps) == 0)
        {
          return result::success {std::u8string_view {begin, end + res_str.length ()}};
        }
        else
        {
          return and_ <E> (input, begin, end + res_str.length (), std::forward <Ps> (ps) ...);
        }
      }
      else if (is_failure (res))
      {
        return result::failure <E> {get_failure (std::move (res))};
      }
      else
      {
        return {};
      }
    }
  }
  inline constexpr auto and_ = [] <typename ... Ps> requires (std::same_as <ParserResultT <Ps, StringReader>, std::u8string_view> && ...) (Ps ... ps) constexpr noexcept
  {
    return [... ps = std::move (ps)] (StringReader & input) constexpr
    {
      return impl::and_ <make_variant_t <ParserResultE <Ps, StringReader> ...>> (input, input.ite, input.ite, std::move (ps) ...);
    };
  };

  // optional: (Parser <u8string_view>) -> Parser <u8string_view>
  inline constexpr auto optional = [] <typename P> requires (std::same_as <ParserResultT <P, StringReader>, std::u8string_view>) (P p) constexpr noexcept
  {
    return or_ (std::move (p), epsilon);
  };

  // repeat: (Parser <u8string_view>) -> Parser <u8string_view>
  inline constexpr auto repeat = [] <typename P> requires (std::same_as <ParserResultT <P, StringReader>, std::u8string_view>) (P p) constexpr noexcept
  {
    return [p = std::move (p)] (StringReader & input) constexpr -> result::result_t <std::u8string_view, ParserResultE <P, StringReader>>
    {
      auto begin = input.ite;
      auto end = input.ite;
      while (true)
      {
        auto res = p (input);
        if (is_success (res))
        {
          auto res_str = get_success (res);
          if (end != res_str.data ())
          {
            throw std::runtime_error {"rejoin failed: 不正なパーサーが存在します"};
          }
          end += res_str.length ();
        }
        else if (is_failure (res))
        {
          return as_failure (std::move (res));
        }
        else
        {
          break;
        }
      }
      return result::success {std::u8string_view {begin, end}};
    };
  };



  // --------------------------------
  // example parsers
  // --------------------------------
  inline constexpr auto position = [] (StringReader & input) constexpr noexcept -> result::result_t <StringReader::Position, never>
  {
    return result::success {input.position};
  };


  inline constexpr auto eof = [] (StringReader & input) constexpr noexcept -> result::result_t <std::u8string_view, never>
  {
    if (input.can_read ())
    {
      return {};
    }
    else
    {
      return result::success {input.as_str ().substr (0, 0)};
    }
  };


  inline constexpr auto raise = [] (std::string_view message) constexpr noexcept -> result::result_t <never, std::string>
  {
    return result::failure {std::string {message}};
  };


  inline constexpr auto character = [] (chino::utf8::codepoint_t c) constexpr noexcept
  {
    return [c = std::move (c)] (StringReader & input) constexpr noexcept -> result::result_t <std::u8string_view, never>
    {
      auto begin = input.ite;
      if (input.can_read () && input.peek () == c)
      {
        input.next ();
        return result::success {std::u8string_view {begin, input.ite}};
      }
      else
      {
        return {};
      }
    };
  };

  inline constexpr auto character_opt = [] (chino::utf8::codepoint_t c) constexpr noexcept
  {
    return or_ (character (c), epsilon);
  };


  inline constexpr auto character_if = [] <typename F>
  requires requires (F f, chino::utf8::codepoint_t c)
  {
    {f (c)} -> std::same_as <bool>;
  }
  (F f) constexpr noexcept
  {
    return [f = std::move (f)] (StringReader & input) constexpr noexcept -> result::result_t <std::u8string_view, never>
    {
      auto begin = input.ite;
      if (input.can_read () && f (input.peek ()))
      {
        input.next ();
        return result::success {std::u8string_view {begin, input.ite}};
      }
      return {};
    };
  };


  inline constexpr auto character_unless = [] <typename F> requires requires (F f, chino::utf8::codepoint_t c)
  {
    {f (c)} -> std::same_as <bool>;
  }
  (F f) constexpr noexcept
  {
    return [f = std::move (f)] (StringReader & input) constexpr noexcept -> result::result_t <std::u8string_view, never>
    {
      auto begin = input.ite;
      if (input.can_read () && not f (input.peek ()))
      {
        input.next ();
        return result::success {std::u8string_view {begin, input.ite}};
      }
      return {};
    };
  };


  inline constexpr auto characters_while = [] <typename F>
  requires requires (F f, chino::utf8::codepoint_t c)
  {
    {f (c)} -> std::same_as <bool>;
  }
  (F f) constexpr noexcept
  {
    return [f = std::move (f)] (StringReader & input) constexpr noexcept -> result::result_t <std::u8string_view, never>
    {
      auto begin = input.ite;
      while (input.can_read () && f (input.peek ()))
      {
        input.next ();
      }
      return result::success {std::u8string_view {begin, input.ite}};
    };
  };


  inline constexpr auto characters_until = [] <typename F>
  requires requires (F f, chino::utf8::codepoint_t c)
  {
    {f (c)} -> std::same_as <bool>;
  }
  (F f) constexpr noexcept
  {
    return [f = std::move (f)] (StringReader & input) constexpr noexcept -> result::result_t <std::u8string_view, never>
    {
      auto begin = input.ite;
      while (input.can_read () && not f (input.peek ()))
      {
        input.next ();
      }
      return result::success {std::u8string_view {begin, input.ite}};
    };
  };


  inline constexpr auto characters_more = [] <typename F>
  requires requires (F f, chino::utf8::codepoint_t c)
  {
    {f (c)} -> std::same_as <bool>;
  }
  (F f) constexpr noexcept
  {
    return [f = std::move (f)] (StringReader & input) constexpr noexcept -> result::result_t <std::u8string_view, never>
    {
      if (input.can_read () && f (input.peek ()))
      {
        auto begin = input.ite;
        input.next ();
        while (input.can_read () && f (input.peek ()))
        {
          input.next ();
        }
        return result::success {std::u8string_view {begin, input.ite}};
      }
      else
      {
        return {};
      }
    };
  };

  inline constexpr auto string = [] (std::u8string_view str) constexpr noexcept
  {
    return [str = std::move (str)] (StringReader & input) constexpr noexcept -> result::result_t <std::u8string_view, never>
    {
      if (auto substr = input.as_str ().substr (0, str.length ()); substr == str)
      {
        auto end = input.ite + str.length ();
        while (input.ite < end)
        {
          input.next ();
        }
        return result::success {substr};
      }
      else
      {
        return {};
      }
    };
  };


  inline constexpr auto keyword = [] (std::u8string_view str) constexpr noexcept
  {
    return [str = std::move (str)] (StringReader & input) constexpr noexcept -> result::result_t <std::u8string_view, never>
    {
      if (auto substr = input.as_str ().substr (0, str.length ()); substr == str)
      {
        auto end = input.ite + str.length ();
        if (input.end == end || not chino::char_utils::is_word (chino::utf8::unsafe_codepoint (end)))
        {
          while (input.ite < end)
          {
            input.next ();
          }
          return result::success {substr};
        }
      }
      return {};
    };
  };


  inline constexpr auto rejoin = [] <typename T, std::same_as <T> ... Ts> (std::tuple <std::basic_string_view <T>, std::basic_string_view <Ts> ...> && t) constexpr noexcept
  {
    return std::apply ([] (auto && ... xs) constexpr -> result::result_t <std::basic_string_view <T>, never>
    {
      std::basic_string_view <T> strs[] = {std::forward <decltype (xs)> (xs) ...};
      for (size_t i = 0; i < sizeof ... (Ts); ++ i)
      {
        if (strs[i].end () != strs[i + 1].begin ())
        {
          throw std::runtime_error {"rejoin failed"};
        }
      }
      return result::success {std::basic_string_view <T> {strs[0].begin (), strs[sizeof ... (Ts)].end ()}};
    }, std::move (t));
  };


  template <typename T>
  inline constexpr auto from_chars = [] (std::u8string_view str) -> result::result_t <T, std::errc>
  {
    T value;
    if (auto [ptr, errc] = std::from_chars (reinterpret_cast <const char *> (str.data ()), reinterpret_cast <const char *> (str.data () + str.length ()), value); errc == std::errc {})
    {
      return result::success {std::move (value)};
    }
    else
    {
      return result::failure {errc};
    }
  };
}
#endif
