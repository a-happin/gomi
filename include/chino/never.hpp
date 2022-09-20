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
  #endif
  }

  struct never
  {
    constexpr never () noexcept = delete;

    template <typename T> requires (not std::is_reference_v <T>)
    [[noreturn]] constexpr operator T () const noexcept
    {
      std_unreachable ();
    }

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
  };

  template <typename T = never>
  [[noreturn]] inline constexpr auto unreachable () noexcept -> T
  {
    std_unreachable ();
  }
}

#endif

