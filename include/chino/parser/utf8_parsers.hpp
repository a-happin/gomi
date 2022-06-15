#ifndef CHINO_PARSER_UTF8_PARSERS_HPP
#define CHINO_PARSER_UTF8_PARSERS_HPP
#include <chino/parser.hpp>
#include <chino/utf8.hpp>
#include <chino/char_utils.hpp>
#include <charconv>

namespace chino::parser::utf8_parsers
{
  struct StringReader
  {
    chino::utf8::codepoint_iterator ite, end;
    struct Position
    {
      std::size_t line;
      std::size_t col;
    } position;

    constexpr StringReader () noexcept
      : ite {}
      , end {}
      , position {1, 1}
    {}

    constexpr StringReader (std::u8string_view str) noexcept
      : ite {str.data ()}
      , end {str.data () + str.length ()}
      , position {1, 1}
    {}

    inline constexpr auto as_str () const noexcept
    {
      return std::u8string_view {static_cast <const char8_t *> (ite), static_cast <const char8_t *> (end)};
    }

    inline constexpr auto can_read () const noexcept
    {
      return ite < end;
    }

    inline constexpr auto peek () const noexcept
    {
      return * ite;
    }

    inline constexpr auto next () noexcept -> decltype (auto)
    {
      if (* static_cast <const char8_t *> (ite) == u8'\n')
      {
        ++ ite;
        ++ position.line;
        position.col = 1;
      }
      else if (* static_cast <const char8_t *> (ite) == u8'\r')
      {
        ++ ite;
        ++ position.line;
        position.col = 1;
        if (can_read () && * static_cast <const char8_t *> (ite) == u8'\n')
        {
          ++ ite;
        }
      }
      else
      {
        ++ ite;
        ++ position.col;
      }
      return * this;
    }
  };

  // --------------------------------
  // example parsers
  // --------------------------------
  inline constexpr auto epsilon = [] (StringReader & input) constexpr noexcept -> result::result_t <std::u8string_view, never>
  {
    return result::success {input.as_str ().substr (0, 0)};
  };


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
        return result::success {std::u8string_view {static_cast <const char8_t *> (begin), static_cast <const char8_t *> (input.ite)}};
      }
      else
      {
        return {};
      }
    };
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
        return result::success {std::u8string_view {static_cast <const char8_t *> (begin), static_cast <const char8_t *> (input.ite)}};
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
        return result::success {std::u8string_view {static_cast <const char8_t *> (begin), static_cast <const char8_t *> (input.ite)}};
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
      return result::success {std::u8string_view {static_cast <const char8_t *> (begin), static_cast <const char8_t *> (input.ite)}};
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
      return result::success {std::u8string_view {static_cast <const char8_t *> (begin), static_cast <const char8_t *> (input.ite)}};
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
        return result::success {std::u8string_view {static_cast <const char8_t *> (begin), static_cast <const char8_t *> (input.ite)}};
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
        auto end = chino::utf8::codepoint_iterator {static_cast <const char8_t *> (input.ite) + substr.length ()};
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
        auto end = chino::utf8::codepoint_iterator {static_cast <const char8_t *> (input.ite) + substr.length ()};
        if (input.end <= end || not chino::char_utils::is_word (* end))
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
