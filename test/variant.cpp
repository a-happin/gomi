#include <chino/variant.hpp>

template <typename T, typename U> requires std::is_same_v <T, U>
inline constexpr bool assert_eq (const T & a, const U & b) noexcept
{
  return a == b;
}

auto main () -> int
{
  constexpr std::variant <int, double> v = 0.0;
  constexpr auto a = chino::variant_cast <int> (v);
  static_assert (assert_eq (a, 0));
}
