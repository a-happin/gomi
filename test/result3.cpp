#include <chino/parser/result.hpp>
#include <string_view>

namespace result = chino::parser::result;

inline constexpr auto my_div (int a, int b) -> result::make_result2 <int, std::string_view>
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

auto main () -> int
{
  constexpr std::same_as <std::variant <result::success <int>, result::failure <std::string_view>>> auto a = my_div (1, 0);
  constexpr auto b = catch_error (std::move (a), [] (auto &&) constexpr noexcept { return result::success {0}; });
  constexpr auto c = catch_error (std::move (b), [] (auto &&) constexpr noexcept { return result::success {0}; });

  static_cast <void> (c);

  static_assert (std::convertible_to <chino::never, result::failure <int>>);
  static_assert (std::convertible_to <chino::never, result::failure <int> &>);
  static_assert (std::convertible_to <chino::never, result::failure <int> &&>);
  static_assert (std::convertible_to <chino::never, const result::failure <int>>);
  static_assert (std::convertible_to <chino::never, const result::failure <int> &>);
  static_assert (std::convertible_to <chino::never, const result::failure <int> &&>);
  static_assert (std::convertible_to <chino::never, result::failure <int> *>);
  static_assert (std::convertible_to <chino::never, const result::failure <int> *>);
  static_assert (std::convertible_to <chino::never, const result::failure <int> * const>);

  /* static_assert (std::convertible_to <chino::never &, result::failure <int>>); */
  /* static_assert (std::convertible_to <chino::never &, result::failure <int> &>); */
  /* static_assert (std::convertible_to <chino::never &, result::failure <int> &&>); */
  /* static_assert (std::convertible_to <chino::never &&, result::failure <int>>); */
  /* static_assert (std::convertible_to <chino::never &&, result::failure <int> &>); */
  /* static_assert (std::convertible_to <chino::never &&, result::failure <int> &&>); */

}
