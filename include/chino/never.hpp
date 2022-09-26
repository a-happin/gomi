#ifndef CHINO_NEVER_HPP
#define CHINO_NEVER_HPP
#include <utility>

namespace chino
{
  [[noreturn]] inline auto std_unreachable () noexcept
  {
  #ifdef __GNUC__
    __builtin_unreachable ();
  #elifdef _MSC_VER
    __assume (false);
  #else
    std::unreachable ();
  #endif
  }

  struct never
  {
    constexpr never () noexcept = delete;

    template <typename T> requires true
    [[noreturn]] constexpr operator T () const noexcept
    {
      std_unreachable ();
    }

  #if !(defined (__GNUC__) && !defined (__clang__))
    template <typename T>
    [[noreturn]] constexpr operator T & () const noexcept
    {
      std_unreachable ();
    }

    template <typename T>
    [[noreturn]] constexpr operator T && () const noexcept
    {
      std_unreachable ();
    }
  #endif
  };

  static_assert (std::is_convertible_v <never, int>);
#if !(defined(__GNUC__) && !defined (__clang__))
  static_assert (std::is_convertible_v <never, int &>);
  static_assert (std::is_convertible_v <never, int &&>);
#endif

  template <typename T = never>
  [[noreturn]] inline constexpr auto unreachable () noexcept -> T
  {
    std_unreachable ();
  }
}


#endif

