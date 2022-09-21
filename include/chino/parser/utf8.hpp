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


  // --------------------------------
  // 特殊parser combinator
  // --------------------------------
  // and_: ((I -> Result <std::u8string_view, Es>) ...) -> I -> Result <std::u8string_view, make_variant <Es ...>>
  inline constexpr auto and_ = [] <typename ... Ps> (Ps && ... ps) constexpr noexcept
  {
    return [captured = detail::make_capture (std::forward <Ps> (ps) ...)] <Input I> (I & input) constexpr noexcept
    {
      static_assert ((std::is_same_v <success_type <Ps, I>, std::u8string_view> && ...));
      using Undefined = make_variant <undefined_type <Ps, I> ...>;
      using T = std::u8string_view;
      using E = make_variant <failure_type <Ps, I> ...>;
      constexpr auto impl = [] <std::size_t i, typename U> (auto & self, I & in, U && value, auto & captured_) constexpr noexcept -> result::result3 <Undefined, T, E>
      {
        if constexpr (i < sizeof ... (Ps))
        {
          if (auto res = std::get <i> (captured_) (in); is_success (res))
          {
            return self.template operator () <i + 1> (self, in, operation::rejoin (std::forward <U> (value), get_success (std::move (res))), captured_);
          }
          else if (is_failure (res))
          {
            return result::failure <E> {get_failure (std::move (res))};
          }
          else
          {
            return as_undefined (res);
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
  inline constexpr auto negative_lookahead = [] <typename P> (P && p) constexpr noexcept
  {
    return [captured = detail::make_capture (std::forward <P> (p))] <std::copyable I> requires Parser <P, I> (I & input) constexpr noexcept -> result::optional <std::u8string_view>
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
  inline constexpr auto optional = [] <typename P> (P && p) constexpr noexcept
  {
    return or_ (std::forward <P> (p), and_ ());
  };


  // repeat: (I -> Result <std::u8string_view, E>) -> I -> Result <std::u8string_view, E>
  template <std::size_t min = 0, std::size_t max = -1zu>
  inline constexpr auto repeat = [] <typename P> (P && p) constexpr noexcept
  {
    return [captured = detail::make_capture (std::forward <P> (p))] <Input I> (I & input) constexpr -> result::result3 <std::conditional_t <min == 0, never, result::undefined>, std::u8string_view, failure_type <P, I>>
    {
      auto & [p_] = captured;
      static_assert (std::same_as <success_type <P, I>, std::u8string_view>);
      auto v = input.as_str ().substr (0, 0);
      std::size_t i = 0;
      for (; i < max; ++ i)
      {
        if (auto res = p_ (input); is_success (res))
        {
          v = operation::rejoin (std::move (v), get_success (std::move (res)));
        }
        else if (is_failure (res))
        {
          return as_failure (std::move (res));
        }
        else
        {
          static_cast <void> (as_undefined (res));
          break;
        }
      }
      if constexpr (min > 0)
      {
        if (i < min)
        {
          return {};
        }
      }
      return result::success {std::move (v)};
    };
  };


  // --------------------------------
  // example parsers
  // --------------------------------
  inline constexpr auto position = [] <Input I> (I & input) constexpr noexcept
  {
    return result::success {input.position ()};
  };


  inline constexpr auto character_if = [] <std::predicate <char32_t> F> (F && f) constexpr noexcept
  {
    return [captured = detail::make_capture (std::forward <F> (f))] <Input I> (I & input) constexpr noexcept -> result::optional <std::u8string_view>
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

  inline constexpr auto any_character = character_if ([] (auto &&) constexpr noexcept { return true; });

  inline constexpr auto eof = negative_lookahead (any_character);

  inline constexpr auto character = [] (char32_t c) constexpr noexcept
  {
    return character_if ([c = std::move (c)] (char32_t x) constexpr noexcept { return x == c; });
  };

  inline constexpr auto character_unless = [] <std::predicate <char32_t> F> (F && f) constexpr noexcept
  {
    return character_if ([captured = detail::make_capture (std::forward <F> (f))] (char32_t x) constexpr noexcept { return not std::get <0> (captured) (x); });
  };


  inline constexpr auto string = [] (std::u8string_view str) constexpr noexcept
  {
    return [str = std::move (str)] <Input I> (I & input) constexpr noexcept -> result::optional <std::u8string_view>
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


  template <typename T>
  inline constexpr auto from_string_view = [] (std::u8string_view str) noexcept -> result::result2 <T, std::errc>
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
