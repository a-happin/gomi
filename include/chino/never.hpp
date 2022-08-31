#ifndef CHINO_NEVER_HPP
#define CHINO_NEVER_HPP
#include <utility>
#include <chino/type_traits/copy_cvref_from.hpp>

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

    template <typename T>
    [[noreturn]] operator T & () const & noexcept
    {
      std_unreachable ();
    }
    template <typename T>
    [[noreturn]] operator T && () const && noexcept
    {
      std_unreachable ();
    }
  };

  [[noreturn]] inline auto unreachable () noexcept -> never
  {
    std_unreachable ();
  }
}

#endif

