#ifndef CHINO_PARSER_UTF8_HPP
#define CHINO_PARSER_UTF8_HPP
#include <chino/parser.hpp>
#include <string_view>
#include <stdexcept>
#include <charconv>

namespace chino::parser::utf8
{
  // --------------------------------
  // rejoin
  // --------------------------------
  namespace operation
  {
    inline constexpr auto rejoin = [] (std::u8string_view && lhs, std::u8string_view && rhs) constexpr -> std::u8string_view
    {
      if (lhs.end () != rhs.begin ())
      {
        throw std::runtime_error {"rejoin failed: 不正なparserが存在します"};
      }
      return std::u8string_view {lhs.begin (), rhs.end ()};
    };
  }


  // --------------------------------
  // Concept
  // --------------------------------
  // ここで定義されるほとんどのパーサーコンビネータが要求するパーサーの引数
  template <typename I>
  concept Input = requires (I & input)
  {
    requires std::copyable <I>;
    {input.position ()} -> std::copyable;
    {input.pointer ()} -> std::same_as <const char8_t *>;
    {input.as_str ()} -> std::same_as <std::u8string_view>;
    {input.can_read ()} -> std::same_as <bool>;
    {input.peek ()} -> std::same_as <char32_t>;
    {input.next ()};
  };

  // --------------------------------
  // parser combinator
  // --------------------------------
  using chino::parser::map;
  using chino::parser::flat_map;
  using chino::parser::catch_error;
  using chino::parser::or_;
  using chino::parser::lookahead;

  #if defined (__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpadded"
  #endif

  // --------------------------------
  // 特殊parser combinator
  // --------------------------------
  // and_: ((I -> Result <std::u8string_view, Es>) ...) -> I -> Result <std::u8string_view, make_variant <Es ...>>
  template <Input I>
  inline constexpr auto and_ = [] <Parser <I> ... Ps> requires (std::same_as <success_type <Ps, I>, std::u8string_view> && ...) (Ps && ... ps) constexpr noexcept
  {
    using T = std::u8string_view;
    using E = make_variant <failure_type <Ps, I> ...>;
    return [captured = std::tuple <Ps ...> {std::forward <Ps> (ps) ...}] (I & input) constexpr noexcept -> result::result <T, E>
    {
      constexpr auto impl = [] <std::size_t i, typename U> (auto & self, I & in, U && value, auto & captured_) constexpr noexcept -> result::result <T, E>
      {
        if constexpr (i < sizeof ... (Ps))
        {
          auto res = std::get <i> (captured_) (in);
          if (is_success (res))
          {
            return self.template operator () <i + 1> (self, in, operation::rejoin (std::forward <U> (value), get_success (std::move (res))), captured_);
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
        else
        {
          return result::success <T> {std::forward <U> (value)};
        }
      };
      return impl.template operator () <0> (impl, input, input.as_str ().substr (0, 0), captured);
    };
  };


  // negative_lookahead: (I -> Result <unknown, unknown>) -> I -> Result <std::u8string_view, never>
  template <std::copyable I>
  inline constexpr auto negative_lookahead = [] <Parser <I> P> (P && p) constexpr noexcept
  {
    return [captured = std::tuple <P> {std::forward <P> (p)}] (I & input) constexpr noexcept -> result::result <std::u8string_view, never>
    {
      auto dummy = input;
      if (auto res = std::get <0> (captured) (dummy); is_success (res))
      {
        return {};
      }
      else
      {
        return result::success {input.as_str ().substr (0, 0)};
      }
    };
  };

  // optional: (I -> Result <std::u8string_view, E>) -> I -> Result <std::u8string_view, E>
  template <Input I>
  inline constexpr auto optional = [] <Parser <I> P> requires (std::same_as <success_type <P, I>, std::u8string_view>) (P && p) constexpr noexcept
  {
    return or_ <I> (std::forward <P> (p), and_ <I> ());
  };


  // repeat: (Parser <u8string_view>) -> Parser <u8string_view>
  // repeat: (I -> Result <std::u8string_view, E>) -> I -> Result <std::u8string_view, E>
  template <typename I>
  inline constexpr auto repeat = [] <Parser <I> P> requires (std::same_as <success_type <P, I>, std::u8string_view>) (P && p, std::size_t min = 0, std::size_t max = -1zu) constexpr noexcept
  {
    return [captured = std::tuple <P> {std::forward <P> (p)}, min, max] (I & input) constexpr -> result::result <std::u8string_view, failure_type <P, I>>
    {
      auto v = input.as_str ().substr (0, 0);
      std::size_t i = 0;
      for (; i < max; ++ i)
      {
        auto res = std::get <0> (captured) (input);
        if (is_success (res))
        {
          v = operation::rejoin (std::move (v), get_success (std::move (res)));
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
      if (i < min)
      {
        return {};
      }
      else
      {
        return result::success {std::move (v)};
      }
    };
  };


  // --------------------------------
  // example parsers
  // --------------------------------
  template <Input I>
  inline constexpr auto position = [] (I & input) constexpr noexcept
  {
    return result::success {input.position ()};
  };


  template <Input I>
  inline constexpr auto character_if = [] <typename F>
  requires requires (F && f, char32_t c)
  {
    {f (c)} -> std::same_as <bool>;
  }
  (F && f) constexpr noexcept
  {
    return [captured = std::tuple <F> {std::forward <F> (f)}] (I & input) constexpr noexcept -> result::result <std::u8string_view, never>
    {
      if (input.can_read () && std::get <0> (captured) (input.peek ()))
      {
        auto begin = input.pointer ();
        input.next ();
        return result::success {std::u8string_view {begin, input.pointer ()}};
      }
      else
      {
        return {};
      }
    };
  };

  template <Input I>
  inline constexpr auto any_character = character_if <I> ([] (auto &&) constexpr noexcept { return true; });

  template <Input I>
  inline constexpr auto eof = negative_lookahead <I> (any_character <I>);

  template <Input I>
  inline constexpr auto character = [] (char32_t c) constexpr noexcept
  {
    return character_if <I> ([c = std::move (c)] (char32_t x) constexpr noexcept { return x == c; });
  };

  template <Input I>
  inline constexpr auto character_unless = [] <typename F>
  requires requires (F && f, char32_t c)
  {
    {f (c)} -> std::same_as <bool>;
  }
  (F && f) constexpr noexcept
  {
    return character_if <I> ([captured = std::tuple <F> {std::forward <F> (f)}] (char32_t x) constexpr noexcept { return not std::get <0> (captured) (x); });
  };


  template <Input I>
  inline constexpr auto string = [] (std::u8string_view str) constexpr noexcept
  {
    return [str = std::move (str)] (I & input) constexpr noexcept -> result::result <std::u8string_view, never>
    {
      if (auto substr = input.as_str ().substr (0, str.length ()); substr == str)
      {
        auto end = input.pointer () + str.length ();
        while (input.pointer () < end)
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


  #if defined (__GNUC__)
    #pragma GCC diagnostic pop
  #endif


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

#define USING_CHINO_PARSER_UTF8_COMBINATORS(I) \
  inline constexpr auto map                = chino::parser::utf8::map <I>; \
  inline constexpr auto flat_map           = chino::parser::utf8::flat_map <I>; \
  inline constexpr auto catch_error        = chino::parser::utf8::catch_error <I>; \
  inline constexpr auto and_               = chino::parser::utf8::and_ <I>; \
  inline constexpr auto or_                = chino::parser::utf8::or_ <I>; \
  inline constexpr auto lookahead          = chino::parser::utf8::lookahead <I>; \
  inline constexpr auto negative_lookahead = chino::parser::utf8::negative_lookahead <I>; \
  inline constexpr auto optional           = chino::parser::utf8::optional <I>; \
  inline constexpr auto repeat             = chino::parser::utf8::repeat <I>; \
  inline constexpr auto position           = chino::parser::utf8::position <I>; \
  inline constexpr auto character_if       = chino::parser::utf8::character_if <I>; \
  inline constexpr auto any_character      = chino::parser::utf8::any_character <I>; \
  inline constexpr auto eof                = chino::parser::utf8::eof <I>; \
  inline constexpr auto character          = chino::parser::utf8::character <I>; \
  inline constexpr auto character_unless   = chino::parser::utf8::character_unless <I>; \
  inline constexpr auto string             = chino::parser::utf8::string <I>

#endif
