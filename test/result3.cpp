#include <chino/parser/result.hpp>
#include <string_view>
#include <cstring>
#include <bit>

namespace result = chino::parser::result;

inline constexpr auto my_div (int a, int b) -> result::result2 <int, std::string_view>
{
  if (b == 0)
  {
    using std::literals::string_view_literals::operator""sv;
    return result::failure {"div by 0"sv};
  }
  else
  {
    return result::success {a / b};
  }
}

inline constexpr auto create_never () noexcept
{
  using chino::never;
  char storage[sizeof (never)];
  return std::bit_cast <never> (storage);
};

auto main () -> int
{
  constexpr std::same_as <std::variant <result::success <int>, result::failure <std::string_view>>> auto a = my_div (1, 0);
  constexpr auto b = catch_error (std::move (a), [] (auto &&) constexpr noexcept { return result::success {0}; });
  constexpr auto c = catch_error (std::move (b), [] (auto &&) constexpr noexcept { return result::success {0}; });

  static_cast <void> (c);

  constexpr auto n = create_never ();
  static_cast <void> (n);
  auto sn = result::success <chino::never> {n};
  result::success <int> fi = sn;
  static_cast <void> (fi);

  /* static_assert (std::convertible_to <chino::never, result::failure <int>>); */
  /* static_assert (std::convertible_to <chino::never, result::failure <int> &>); */
  /* static_assert (std::convertible_to <chino::never, result::failure <int> &&>); */
  /* static_assert (std::convertible_to <chino::never, const result::failure <int>>); */
  /* static_assert (std::convertible_to <chino::never, const result::failure <int> &>); */
  /* static_assert (std::convertible_to <chino::never, const result::failure <int> &&>); */
  /* static_assert (std::convertible_to <chino::never, result::failure <int> *>); */
  /* static_assert (std::convertible_to <chino::never, const result::failure <int> *>); */
  /* static_assert (std::convertible_to <chino::never, const result::failure <int> * const>); */

  /* static_assert (std::convertible_to <result::success <int>, result::success <double>>); */
  static_assert (std::convertible_to <result::success <chino::never>, result::success <int>>);
  static_assert (std::convertible_to <result::success <chino::never>, result::failure <int>>);

  /* static_assert (std::convertible_to <chino::never &, result::failure <int>>); */
  /* static_assert (std::convertible_to <chino::never &, result::failure <int> &>); */
  /* static_assert (std::convertible_to <chino::never &, result::failure <int> &&>); */
  /* static_assert (std::convertible_to <chino::never &&, result::failure <int>>); */
  /* static_assert (std::convertible_to <chino::never &&, result::failure <int> &>); */
  /* static_assert (std::convertible_to <chino::never &&, result::failure <int> &&>); */

}
