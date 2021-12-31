#include <chino/optional.hpp>

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
}
