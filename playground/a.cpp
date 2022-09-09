#include <iostream>
#include <sstream>
#include <bitset>

template <std::integral T>
auto to_bin (T x)
{
  std::ostringstream s;
  s << std::bitset <8 * sizeof (T)> (x);
  return std::move (s).str ();
}

auto main () -> int
{
  using std::cout, std::endl;
  char8_t buf[] = {0xED, 0xBF, 0xBF};
  for (auto && elem : buf)
  {
    cout << to_bin (elem) << ' ';
  }
}
