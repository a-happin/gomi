#include <at.hpp>
#include <type_traits>

template <typename ... Ts>
auto f ()
{
  using namespace gomi;
  using a = typename at <0 , Ts ...>::type;
  using b = typename at <1 , Ts ...>::type;
  using c = typename at <2 , Ts ...>::type;
  using d = typename at <3 , Ts ...>::type;
  using e = typename at <4 , Ts ...>::type;
  using z = typename at <5 , Ts ...>::type;
  // using zz = typename at <6 , Ts ...>::type; // static_assert failed
  static_assert (std::is_same <a , int> {} , "");
  static_assert (std::is_same <b , const int> {} , "");
  static_assert (std::is_same <c , volatile int> {} , "");
  static_assert (std::is_same <d , int *> {} , "");
  static_assert (std::is_same <e , int &> {} , "");
  static_assert (std::is_same <z , int &&> {} , "");
}

auto main () -> int
{
  f <int , const int , volatile int , int * , int & , int &&> ();
}
