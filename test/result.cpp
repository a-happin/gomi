#include <string>
#include <chino/result.hpp>

using namespace std::literals::string_view_literals;

template <std::floating_point T>
constexpr auto double_equal (T a, T b)
{
  constexpr T eps = 1e-9;
  return (a < b ? b - a : a - b) < eps;
}

constexpr auto divide (double a, double b) noexcept -> chino::result_t <double, void *>
{
  if (double_equal (b, 0.0))
  {
    return chino::failure <void *> (nullptr);
  }
  return chino::success (a / b);
}

auto main () -> int
{
  {
    constexpr auto a = divide (1, 0);
    static_assert (not chino::is_success (a));
    static_assert (chino::is_failure (a));
    static_assert (chino::get_failure (a) == nullptr);
  }

  {
    constexpr auto a = divide (1, 1);
    static_assert (chino::is_success (a));
    static_assert (not chino::is_failure (a));
    static_assert (double_equal (chino::get_success (a), 1.0));
    static_assert (std::is_same_v <decltype (chino::get_success (a)), const double &>);
    static_assert (std::is_same_v <decltype (chino::get_failure (a)), void * const &>);
    static_assert (std::is_same_v <decltype (chino::unwrap (a)), const double &>);
  }

  {
    constexpr auto a = divide (1000, 1);
    constexpr auto b = chino::match (a,
      [] (auto && x) { return double_equal (x, 0.0); },
      [] (auto && x) { return x == nullptr; }
    );
    static_assert (std::is_same_v <decltype (b), const bool>);
  }

  {
    constexpr auto a = divide (22, 7);
    auto b = chino::map (a, [] (auto && x) { return std::to_string (x); });
    static_assert (std::is_same_v <decltype(b), chino::result_t <std::string, void *>>);

    auto c = chino::map (a, [] (auto && x) { return std::forward <decltype (x)> (x); });
    static_assert (std::is_same_v <decltype(c), chino::result_t <double, void *>>);
  }

  {
    constexpr auto a = divide (22, 0);
    auto b = chino::flat_map (a, [] (auto && x) -> chino::result_t <std::string, void *> { return chino::success (std::to_string (x)); });
    static_assert (std::is_same_v <decltype(b), chino::result_t <std::string, void *>>);

    auto c = chino::flat_map (a, [] (auto && x) -> chino::result_t <double, void*> { return chino::success (std::forward <decltype (x)> (x)); });
    static_assert (std::is_same_v <decltype(c), chino::result_t <double, void *>>);
  }
}
