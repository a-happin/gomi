#include <chino/optional.hpp>
#include <iostream>
#include <chino/showable.hpp>

namespace std
{
  template <chino::Showable T>
  inline auto operator << (std::ostream &stream, const std::optional <T> & opt) -> decltype (auto)
  {
    return opt ? stream << "Some (" << *opt << ")" : stream << "nullopt";
  }
}
#include <chino/test.hpp>

inline auto test_map (chino::test::test & t)
{
  std::optional <int> a = 3;
  auto b = chino::map (a, [] (auto && x) { return std::forward <decltype (x)> (x) + 0.5; });
  static_assert (chino::Showable <double>);
  static_assert (chino::Showable <std::optional <double>>);
  std::cout << a;
  static_assert (chino::Showable <chino::test::test>);
  /* t.assert_eq (b, std::optional {3.1}); */
  /* t.assert_eq (b, std::optional {3.1}); */
  t.assert_eq (b, std::optional {3.5});
}

auto main () -> int
{
  {
    constexpr std::optional <int> a = 3;
    auto b = chino::map (a, [] <typename T> (T && x) { return std::forward <T> (x) + 0.5; });
    static_assert (std::is_same_v <decltype (b), std::optional <double>>);
    constexpr std::optional <int> c = std::nullopt;
    constexpr auto d = chino::map (c, [] <typename T> (T && x) { return std::forward <T> (x) + 0.5; });
    static_assert (d == std::nullopt);
  }

  {
    constexpr std::optional <int> a = 3;
    auto b = chino::flat_map (a, [] <typename T> (T &&) -> std::optional <double> { return std::nullopt; });
    static_assert (std::is_same_v <decltype (b), std::optional <double>>);
    constexpr std::optional <int> c = std::nullopt;
    constexpr auto d = chino::flat_map (c, [] <typename T> (T &&) -> std::optional <int *> { return std::nullopt; });
    static_assert (d == std::nullopt);
  }

  {
    constexpr std::optional <int> a = 0;
    constexpr auto b = chino::match (std::move (a), [] (auto && x) { return x; }, [] () -> std::optional <int> { return std::nullopt; });
    static_assert (b == 0);
  }
  auto res = 0;
  res |= chino::test::add_test (test_map);
  return res;
}
