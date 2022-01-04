#include <iostream>
#include <chino/to_string.hpp>

template <typename T>
auto print (const T & x)
{
  std::cout << chino::to_string <char> (x) << std::endl;
}

template <typename T>
auto wprint (const T & x)
{
  std::wcout << chino::to_string <wchar_t> (x) << std::endl;
}

template <typename T, std::size_t N>
using array = T[N];

auto main () -> int
{
  using namespace std::literals;
  print (true);
  print (false);
  print (nullptr);
  print (std::tuple ('a', static_cast <short> (1), 2, static_cast <long> (3), static_cast <long long> (4)));
  print (std::tuple (static_cast <unsigned char> ('a'), static_cast <unsigned short> (1), static_cast <unsigned int> (2), static_cast <unsigned long> (3), static_cast <unsigned long long> (4)));
  print (std::tuple (1.0, 2.0f, 3.0L));
  print (std::tuple {std::optional <int> {6}, std::optional <signed char> {}});
  print ('a');
  print ("cstr");
  print ("str"s);
  print ("str view"sv);
  wprint (U'a');
  wprint (L"cstr");
  wprint (L"str"s);
  wprint (L"str view"sv);
  print (std::pair (5, 9.0f));
  print (std::optional {std::vector <char> {'h', 'e', 'l', 'l', 'o'}});
  print (std::array {'h', 'e', 'l', 'l', 'o'});
  array <char, 5> hello = {'h', 'e', 'l', 'l', 'o'};
  /* static_assert (std::is_same_v <decltype (hello), char[6]>); */
  print (hello);
  print (std::tuple {});
  std::variant <std::monostate, short, long double> v;
  print (v);
  print (std::valarray <int> {1, 2, 3});
  print (std::list <int> {1, 2, 3});
  print (std::forward_list <int> {1, 2, 3});
  array <int, 3> arr = {1, 2, 3};
  print (arr);
  print (std::initializer_list <double> {1.0, 2.0});
  print (std::set <double> {1.0, 2.0});
  print (std::map <std::string, double> {{"ok", 2.0}, {"bad", 404}});
}
