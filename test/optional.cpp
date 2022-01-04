#include <chino/optional.hpp>
#include <chino/test.hpp>

inline auto test_map (chino::test::test & t)
{
  std::optional <int> a = 3;
  auto b = chino::map (a, [] (auto && x) { return std::forward <decltype (x)> (x) + 0.5; });
  t.assert_eq (b, std::optional {3.5});

  std::optional <short> c = 3;
  auto d = chino::map (c, [] (auto && x) { return std::forward <decltype (x)> (x) + 0.5f; });
  t.assert_eq (d, std::optional {3.5f});

  std::optional <int> e;
  auto f = chino::map (e, [] (auto && x) { return std::forward <decltype (x)> (x) + 0.5; });
  t.assert_eq (f, std::optional <double> {});
}

inline auto test_flat_map (chino::test::test & t)
{
  std::optional <int> a = 3;
  auto b = chino::flat_map (a, [] (auto && x) { return std::optional {std::forward <decltype (x)> (x) + 0.5}; });
  t.assert_eq (b, std::optional {3.5});

  std::optional <short> c = 3;
  auto d = chino::flat_map (c, [] (auto && x) { return std::optional {std::forward <decltype (x)> (x) + 0.5f}; });
  t.assert_eq (d, std::optional {3.5f});

  std::optional <int> e;
  auto f = chino::flat_map (e, [] (auto && x) { return std::optional {std::forward <decltype (x)> (x) + 0.5}; });
  t.assert_eq (f, std::optional <double> {});
}

inline auto test_match (chino::test::test & t)
{
  std::optional <int> a = 0;
  auto b = chino::match (std::move (a), [] (auto && x) { return x; }, [] () -> std::optional <int> { return std::nullopt; });
  t.assert_eq (b, std::optional {0});
}

auto main () -> int
{
  auto res = 0;
  res |= chino::test::add_test (test_map);
  res |= chino::test::add_test (test_flat_map);
  res |= chino::test::add_test (test_match);
  return res;
}
