#ifndef CHINO_PARSER_UTF8_HPP
#define CHINO_PARSER_UTF8_HPP
#include <chino/parser.hpp>
#include <chino/utf8.hpp>
#include <chino/utf8/string_reader.hpp>
#include <chino/char_utils.hpp>
#include <charconv>

namespace chino::parser::utf8
{
  using chino::utf8::StringReader;

  // --------------------------------
  // basic parsers
  // --------------------------------
  inline constexpr auto epsilon = [] (StringReader & input) constexpr noexcept -> result::result <std::u8string_view, never>
  {
    return result::success {input.as_str ().substr (0, 0)};
  };


  // --------------------------------
  // parser combinator
  // --------------------------------
  using chino::parser::or_;
  using chino::parser::map;
  using chino::parser::flat_map;
  using chino::parser::recover;


  // --------------------------------
  // 特殊parser combinator
  // --------------------------------
  // and_: (Parser <u8string_view> ...) -> Parser <u8string_view>
  namespace impl
  {
    template <typename E, typename P, typename ... Ps>
    inline constexpr auto and_ (StringReader & input, const char8_t * begin, const char8_t * end, P && p, Ps && ... ps) -> result::result <std::u8string_view, E>
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
    return [... ps = std::move (ps)] (StringReader & input) constexpr -> result::result <std::u8string_view, make_variant <ParserResultE <Ps, StringReader> ...>>
    {
      return impl::and_ <make_variant <ParserResultE <Ps, StringReader> ...>> (input, input.ptr, input.ptr, std::move (ps) ...);
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
    return [p = std::move (p)] (StringReader & input) constexpr -> result::result <std::u8string_view, ParserResultE <P, StringReader>>
    {
      auto begin = input.ptr;
      auto end = input.ptr;
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


  // more: (Parser <u8string_view>) -> Parser <u8string_view>
  inline constexpr auto more = [] <typename P> requires (std::same_as <ParserResultT <P, StringReader>, std::u8string_view>) (P p) constexpr noexcept
  {
    return disallow_empty (repeat (std::move (p)));
  };


  // --------------------------------
  // example parsers
  // --------------------------------
  inline constexpr auto position = [] (StringReader & input) constexpr noexcept -> result::result <StringReader::Position, never>
  {
    return result::success {input.position};
  };


  inline constexpr auto any_character = [] (StringReader & input) constexpr noexcept -> result::result <std::u8string_view, never>
  {
    if (input.can_read ())
    {
      auto begin = input.ptr;
      input.next ();
      return result::success {std::u8string_view {begin, input.ptr}};
    }
    return {};
  };


  inline constexpr auto eof = [] (StringReader & input) constexpr noexcept -> result::result <std::u8string_view, never>
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


  inline constexpr auto lookahead_character = [] <typename F> requires requires (F f, chino::utf8::codepoint_t c)
  {
    {f (c)} -> std::same_as <bool>;
  }
  (F f) constexpr noexcept
  {
    return [f = std::move (f)] (StringReader & input) constexpr noexcept -> result::result <std::u8string_view, never>
    {
      if (input.can_read () && f (input.peek ()))
      {
        return result::success {input.as_str ().substr (0, 0)};
      }
      return {};
    };
  };


  inline constexpr auto negative_lookahead_character = [] <typename F> requires requires (F f, chino::utf8::codepoint_t c)
  {
    {f (c)} -> std::same_as <bool>;
  }
  (F f) constexpr noexcept
  {
    return [f = std::move (f)] (StringReader & input) constexpr noexcept -> result::result <std::u8string_view, never>
    {
      if (input.can_read () && f (input.peek ()))
      {
        return {};
      }
      return result::success {input.as_str ().substr (0, 0)};
    };
  };


  inline constexpr auto character = [] (chino::utf8::codepoint_t c) constexpr noexcept
  {
    return [c = std::move (c)] (StringReader & input) constexpr noexcept -> result::result <std::u8string_view, never>
    {
      if (input.can_read () && input.peek () == c)
      {
        auto begin = input.ptr;
        input.next ();
        return result::success {std::u8string_view {begin, input.ptr}};
      }
      else
      {
        return {};
      }
    };
  };


  /* inline constexpr auto character_opt = [] (chino::utf8::codepoint_t c) constexpr noexcept */
  /* { */
  /*   return or_ (character (c), epsilon); */
  /* }; */


  inline constexpr auto character_if = [] <typename F>
  requires requires (F f, chino::utf8::codepoint_t c)
  {
    {f (c)} -> std::same_as <bool>;
  }
  (F f) constexpr noexcept
  {
    return [f = std::move (f)] (StringReader & input) constexpr noexcept -> result::result <std::u8string_view, never>
    {
      if (input.can_read () && f (input.peek ()))
      {
        auto begin = input.ptr;
        input.next ();
        return result::success {std::u8string_view {begin, input.ptr}};
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
    return [f = std::move (f)] (StringReader & input) constexpr noexcept -> result::result <std::u8string_view, never>
    {
      if (input.can_read () && not f (input.peek ()))
      {
        auto begin = input.ptr;
        input.next ();
        return result::success {std::u8string_view {begin, input.ptr}};
      }
      return {};
    };
  };


  /* inline constexpr auto characters_while = [] <typename F> */
  /* requires requires (F f, chino::utf8::codepoint_t c) */
  /* { */
  /*   {f (c)} -> std::same_as <bool>; */
  /* } */
  /* (F f) constexpr noexcept */
  /* { */
  /*   return [f = std::move (f)] (StringReader & input) constexpr noexcept -> result::result <std::u8string_view, never> */
  /*   { */
  /*     auto begin = input.ite; */
  /*     while (input.can_read () && f (input.peek ())) */
  /*     { */
  /*       input.next (); */
  /*     } */
  /*     return result::success {std::u8string_view {begin, input.ite}}; */
  /*   }; */
  /* }; */


  /* inline constexpr auto characters_until = [] <typename F> */
  /* requires requires (F f, chino::utf8::codepoint_t c) */
  /* { */
  /*   {f (c)} -> std::same_as <bool>; */
  /* } */
  /* (F f) constexpr noexcept */
  /* { */
  /*   return [f = std::move (f)] (StringReader & input) constexpr noexcept -> result::result <std::u8string_view, never> */
  /*   { */
  /*     auto begin = input.ite; */
  /*     while (input.can_read () && not f (input.peek ())) */
  /*     { */
  /*       input.next (); */
  /*     } */
  /*     return result::success {std::u8string_view {begin, input.ite}}; */
  /*   }; */
  /* }; */


  /* inline constexpr auto characters_more = [] <typename F> */
  /* requires requires (F f, chino::utf8::codepoint_t c) */
  /* { */
  /*   {f (c)} -> std::same_as <bool>; */
  /* } */
  /* (F f) constexpr noexcept */
  /* { */
  /*   return [f = std::move (f)] (StringReader & input) constexpr noexcept -> result::result <std::u8string_view, never> */
  /*   { */
  /*     if (input.can_read () && f (input.peek ())) */
  /*     { */
  /*       auto begin = input.ite; */
  /*       input.next (); */
  /*       while (input.can_read () && f (input.peek ())) */
  /*       { */
  /*         input.next (); */
  /*       } */
  /*       return result::success {std::u8string_view {begin, input.ite}}; */
  /*     } */
  /*     else */
  /*     { */
  /*       return {}; */
  /*     } */
  /*   }; */
  /* }; */


  inline constexpr auto string = [] (std::u8string_view str) constexpr noexcept
  {
    return [str = std::move (str)] (StringReader & input) constexpr noexcept -> result::result <std::u8string_view, never>
    {
      if (auto substr = input.as_str ().substr (0, str.length ()); substr == str)
      {
        auto end = input.ptr + str.length ();
        while (input.ptr < end)
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


  /* inline constexpr auto keyword = [] (std::u8string_view str) constexpr noexcept */
  /* { */
  /*   return [str = std::move (str)] (StringReader & input) constexpr noexcept -> result::result <std::u8string_view, never> */
  /*   { */
  /*     if (auto substr = input.as_str ().substr (0, str.length ()); substr == str) */
  /*     { */
  /*       auto end = input.ite + str.length (); */
  /*       if (input.end == end || not chino::char_utils::is_word (chino::utf8::unsafe_codepoint (end))) */
  /*       { */
  /*         while (input.ite < end) */
  /*         { */
  /*           input.next (); */
  /*         } */
  /*         return result::success {substr}; */
  /*       } */
  /*     } */
  /*     return {}; */
  /*   }; */
  /* }; */


  /* inline constexpr auto rejoin = [] <typename T, std::same_as <T> ... Ts> (std::tuple <std::basic_string_view <T>, std::basic_string_view <Ts> ...> && t) constexpr noexcept */
  /* { */
  /*   return std::apply ([] (auto && ... xs) constexpr -> result::result <std::basic_string_view <T>, never> */
  /*   { */
  /*     std::basic_string_view <T> strs[] = {std::forward <decltype (xs)> (xs) ...}; */
  /*     for (size_t i = 0; i < sizeof ... (Ts); ++ i) */
  /*     { */
  /*       if (strs[i].end () != strs[i + 1].begin ()) */
  /*       { */
  /*         throw std::runtime_error {"rejoin failed"}; */
  /*       } */
  /*     } */
  /*     return result::success {std::basic_string_view <T> {strs[0].begin (), strs[sizeof ... (Ts)].end ()}}; */
  /*   }, std::move (t)); */
  /* }; */


  // tokenize: (Parser <T>) -> Parser <(Position, u8string_view, T)>
  inline constexpr auto tokenize = [] <typename P> (P p) constexpr noexcept {
    return [p = std::move (p)] (StringReader & input) constexpr noexcept -> result::result <std::tuple <StringReader::Position, std::u8string_view, ParserResultT <P, StringReader>>, ParserResultE <P, StringReader>>
    {
      auto pos = input.position;
      auto begin = input.ptr;
      auto res = std::move (p) (input);
      if (is_success (res))
      {
        return result::success {std::tuple {std::move (pos), std::u8string_view {begin, input.ptr}, get_success (std::move (res))}};
      }
      else if (is_failure (res))
      {
        return as_failure (std::move (res));
      }
      else
      {
        return {};
      }
    };
  };


  template <typename T>
  inline constexpr auto from_string_view = [] (std::u8string_view str) -> result::result <T, std::errc>
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
