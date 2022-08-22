#include <type_traits>
#include <chino/overload.hpp>

template <typename T, typename U> requires (std::is_same_v <T, U>)
inline constexpr bool assert_eq (const T & a, const U & b) noexcept
{
  return a == b;
}

auto main () -> int
{
  constexpr auto f = chino::overload {
    [] (char x) constexpr noexcept { return x; },
    [] (short x) constexpr noexcept { return x; },
    [] (int x) constexpr noexcept { return x; },
    [] (long x) constexpr noexcept { return x; },
  };

  static_assert (assert_eq (f (0), 0));
  static_assert (assert_eq (f (0L), 0L));
  static_assert (assert_eq (f (static_cast <short> (0)), static_cast <short> (0)));
  static_assert (assert_eq (f ('\0'), '\0'));
}
