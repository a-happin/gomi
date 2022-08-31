#include <chino/result.hpp>
#include <iostream>
using namespace std;
using namespace chino;

inline constexpr auto mydiv (int32_t a, int32_t b) -> result::result <int32_t, std::string_view>
{
  if (b == 0)
  {
    return result::failure {std::string_view {"div by 0"}};
  }
  return result::success {a / b};
}

[[noreturn]] inline auto operator << (std::ostream &, const never &) -> std::ostream &
{
  chino::unreachable ();
  /* static_cast <std::ostream &> (x); */
}

template <typename T>
concept printable = requires (std::ostream & stream, const T & x)
{
  {stream << x};
};

template <result::Result R>
auto print (R && r)
{
  if (is_success (r))
  {
    cout << "Success: " << get_success (std::forward <R> (r)) << endl;
  }
  else
  {
    cout << "Error: " << get_failure (std::forward <R> (r)) << endl;
  }
}

auto main () -> int
{
  using namespace std::literals;

  constexpr auto a = mydiv (16, 2);
  print (a);

  constexpr auto b = mydiv (16, 0);
  print (b);

  constexpr auto c = result::failure {"always failed."sv};
  print (std::move (c));

  constexpr auto d = result::success {"always success"sv};
  print (d);

  constexpr auto e = result::map (a, [] (auto x) { return x + 2; });
  print (e);

  constexpr auto f = result::map (b, [] (auto x) { return x + 2; });
  print (f);

  constexpr auto g = result::catch_error (b, [] (auto &&) { return result::success {0}; });
  print (g);

  constexpr auto h = result::and_then (g, [] (auto &&) { return result::failure {"草"sv}; });
  print (h);
}
