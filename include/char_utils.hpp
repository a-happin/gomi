#ifndef CHAR_UTILS_HPP
#define CHAR_UTILS_HPP
#include <string_view>
#include <chino/macros/typed_string.hpp>

namespace char_utils
{
  // 使う側への注意: 必要以上にusingしないこと
  namespace operators {
    template <typename F1, typename F2>
    /* あると警告出でくるし */
    /* 閉じ込めてADLでしか呼び出せないようにしたい */
    /* requires (std::is_invocable_r_v <bool, F1, char>     && std::is_invocable_r_v <bool, F2, char>) */
    /*       || (std::is_invocable_r_v <bool, F1, wchar_t>  && std::is_invocable_r_v <bool, F2, wchar_t>) */
    /*       || (std::is_invocable_r_v <bool, F1, char16_t> && std::is_invocable_r_v <bool, F2, char16_t>) */
    /*       || (std::is_invocable_r_v <bool, F1, char32_t> && std::is_invocable_r_v <bool, F2, char32_t>) */
    /*       || (std::is_invocable_r_v <bool, F1, char8_t>  && std::is_invocable_r_v <bool, F2, char8_t>) */
    inline constexpr auto operator | (F1 && lhs, F2 && rhs) noexcept
    {
      return [captured = std::tuple <F1, F2> {std::forward <F1> (lhs), std::forward <F2> (rhs)}]
      <typename CharT>
      (CharT c) constexpr noexcept (noexcept (std::forward <F1> (lhs) (c) || std::forward <F2> (rhs) (c)))
      requires std::is_invocable_r_v <bool, F1, CharT> && std::is_invocable_r_v <bool, F2, CharT>
      /* requires requires { */
      /*   requires std::is_invocable_r_v <bool, F1, CharT> && std::is_invocable_r_v <bool, F2, CharT>; */
      /* } */
      {
        return std::get <0> (captured) (c) || std::get <1> (captured) (c);
      };
    }

    template <typename F1, typename F2>
    inline constexpr auto operator & (F1 && lhs, F2 && rhs) noexcept
    {
      return [captured = std::tuple <F1, F2> {std::forward <F1> (lhs), std::forward <F2> (rhs)}]
      <typename CharT>
      (CharT c) constexpr noexcept (noexcept (std::forward <F1> (lhs) (c) && std::forward <F2> (rhs) (c)))
      requires std::is_invocable_r_v <bool, F1, CharT> && std::is_invocable_r_v <bool, F2, CharT>
      {
        return std::get <0> (captured) (c) && std::get <1> (captured) (c);
      };
    }

    template <typename F>
    inline constexpr auto operator not (F && f) noexcept
    {
      return [captured = std::tuple <F> {std::forward <F> (f)}]
      <typename CharT>
      (CharT c) constexpr noexcept (noexcept (std::forward <F> (f) (c)))
      requires std::is_invocable_r_v <bool, F, CharT>
      {
        return not std::get <0> (captured) (c);
      };
    }
  }

  // ADL firewall
  namespace detail {
    using namespace operators;

    inline constexpr auto is_anychar = [] <typename CharT> (CharT) constexpr noexcept
    {
      return true;
    };

    inline constexpr auto is_not_anychar = [] <typename CharT> (CharT) constexpr noexcept
    {
      return false;
    };

    inline constexpr auto is_char = [] <typename CharT1> (CharT1 c1) constexpr noexcept
    {
      return [c1] <typename CharT2> (CharT2 c2) constexpr noexcept
      requires std::same_as <CharT1, CharT2>
      {
        return c1 == c2;
      };
    };

    inline constexpr auto is_char_in_range = [] <typename CharT1> (CharT1 min, CharT1 max) constexpr noexcept
    {
      return [min, max] <typename CharT2> (CharT2 c) constexpr noexcept
      requires std::same_as <CharT1, CharT2>
      {
        return min <= c && c <= max;
      };
    };

    inline constexpr auto contains = [] <typename CharT1> (std::basic_string_view <CharT1> str) constexpr noexcept
    {
      return [str] <typename CharT2> (CharT2 c) constexpr noexcept
      requires std::same_as <CharT1, CharT2>
      {
        return str.contains (c);
      };
    };

    inline constexpr auto is_space = [] <typename CharT> (CharT c) constexpr noexcept
    {
      constexpr auto impl = is_char_in_range (TYPED_CHAR (CharT, '\t'), TYPED_CHAR (CharT, '\r'))
                          | is_char (TYPED_CHAR (CharT, ' '))
                          ;
      return impl (c);
    };

    // no endline spaces
    inline constexpr auto is_blank = [] <typename CharT> (CharT c) constexpr noexcept
    {
      constexpr auto impl = is_char (TYPED_CHAR (CharT, '\t'))
                          | is_char (TYPED_CHAR (CharT, ' '))
                          ;
      return impl (c);
    };

    inline constexpr auto is_digit = [] <typename CharT> (CharT c) constexpr noexcept
    {
      constexpr auto impl = is_char_in_range (TYPED_CHAR (CharT, '0'), TYPED_CHAR (CharT, '9'));
      return impl (c);
    };

    inline constexpr auto is_upper = [] <typename CharT> (CharT c) constexpr noexcept
    {
      constexpr auto impl = is_char_in_range (TYPED_CHAR (CharT, 'A'), TYPED_CHAR (CharT, 'Z'));
      return impl (c);
    };

    inline constexpr auto is_lower = [] <typename CharT> (CharT c) constexpr noexcept
    {
      constexpr auto impl = is_char_in_range (TYPED_CHAR (CharT, 'a'), TYPED_CHAR (CharT, 'z'));
      return impl (c);
    };

    inline constexpr auto is_alpha = is_upper | is_lower;

    inline constexpr auto is_alnum = is_digit | is_alpha;

    // hexdecimal digit
    inline constexpr auto is_xdigit = [] <typename CharT> (CharT c) constexpr noexcept
    {
      constexpr auto impl = is_digit
                          | is_char_in_range (TYPED_CHAR (CharT, 'A'), TYPED_CHAR (CharT, 'F'))
                          | is_char_in_range (TYPED_CHAR (CharT, 'a'), TYPED_CHAR (CharT, 'f'))
                          ;
      return impl (c);
    };

    inline constexpr auto is_word = [] <typename CharT> (CharT c) constexpr noexcept
    {
      constexpr auto impl = is_alnum
                          | is_char (TYPED_CHAR (CharT, '_'))
                          ;
      return impl (c);
    };
  } // namespace detail

  using detail::is_anychar
      , detail::is_not_anychar
      , detail::is_char
      , detail::is_char_in_range
      , detail::is_space
      , detail::is_blank
      , detail::is_digit
      , detail::is_upper
      , detail::is_lower
      , detail::is_alpha
      , detail::is_alnum
      , detail::is_xdigit
      , detail::is_word
      ;

  inline constexpr auto equal_to_case_insensitive = [] <typename CharT1, typename CharT2> (CharT1 c1, CharT2 c2) constexpr noexcept
  {
    if (is_upper (c1)) c1 += (TYPED_CHAR (CharT1, 'a') - TYPED_CHAR (CharT1, 'A'));
    if (is_upper (c2)) c2 += (TYPED_CHAR (CharT2, 'a') - TYPED_CHAR (CharT2, 'A'));
    return c1 == c2;
  };
} // namespace char_utils
#endif
