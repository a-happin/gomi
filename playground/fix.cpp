#include <type_traits>
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <iostream>


template <typename T, typename U>
struct Located
{
  T pos;
  U value;
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"

template <template <typename> class F>
struct Fix : Located <std::size_t, F <Fix <F>>>
{
  using Located <std::size_t, F <Fix <F>>>::Located;
};

template <typename T>
struct JSON_F : std::variant <std::monostate, bool, double, std::string, std::vector <T>, std::unordered_map <std::string, T>>
{
  using std::variant <std::monostate, bool, double, std::string, std::vector <T>, std::unordered_map <std::string, T>>::variant;
};

#pragma GCC diagnostic pop


using JSON = Fix <JSON_F>;

auto main () -> int
{
  JSON a;
  std::cout << std::boolalpha << std::holds_alternative <std::monostate> (a.value) << "\n";
}
