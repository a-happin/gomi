#include <fold.hpp>
#include <iostream>
#include <array>

struct plus_t {
  template < typename T , typename U>
  constexpr auto operator () (T a , U b)
    -> decltype (a + b)
  {
    return a + b ;
  }
};

struct minus_t {
  template < typename T , typename U>
  constexpr auto operator () (T a , U b) const
    -> decltype (a - b)
  {
    return a - b ;
  }
};

constexpr auto double_equal (double a , double b) -> bool
{
  return ((a < b) ? (b - a) : (a - b)) < 1e-10;
}

template <gomi::size_t ... Indices>
constexpr auto f (gomi::index_sequence <Indices ...>)
{
  return gomi::foldr (plus_t {} , static_cast <gomi::size_t> (0) , Indices ...);
}

struct cons_t
{
  template <typename T , std::size_t N>
  constexpr auto operator () (T && x , std::array <T , N> xs)
  {
    std::array <T , N + 1> res;
    for (std::size_t i = 0; i < N; ++ i)
    {
        res [i + 1] = xs [i];
    }
    res [0] = std::forward <T> (x);
    return res;
  }
};


template <gomi::size_t ... Indices>
constexpr auto guu (gomi::index_sequence <Indices ...>)
{
  return gomi::foldr (cons_t {} , std::array <gomi::size_t , 0u> {} , Indices ...);
}

auto main () -> int
{
  using namespace gomi;
  constexpr auto a = foldl (plus_t {} , 0 , 1 , 2 , 3 , 4 , 5);
  static_assert (a == 15 , "") ;

  constexpr auto b = foldr (minus_t {} , 0 , 1 , 2 , 3 , 4 , 5);
  static_assert (b == 3 , "") ;

  constexpr auto c = foldl (plus_t {} , 0.0 , .1 , .2 , .3 , .4 , .5);
  static_assert (double_equal (c , 1.5) , "") ;

  constexpr auto d = foldr (minus_t {} , 0.0 , .1 , .2 , .3 , .4 , .5);
  static_assert (double_equal (d , 0.3) , "") ;

  std::cout << a << std::endl;
  std::cout << b << std::endl;
  std::cout << c << std::endl;
  std::cout << d << std::endl;

  std::cout << f (make_index_sequence <1000>{}) << std::endl;

  std::cout << foldr ([](int x , double y){return x * y;} , 0.1 , 1 , 2 , 3 , 4) << std::endl;
  auto arr = guu (make_index_sequence <1000>{});
  for (auto && elem : arr)
  {
      std::cout << elem << ", ";
  }
  std::cout << std::endl;
  return 0 ;
}
