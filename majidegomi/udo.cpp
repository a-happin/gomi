#include <utility>
#include <iostream>
#include <cmath>
#include <cstdio>

template < typename T , typename F >
struct udo_class2
{
private :
    T x ;
    F f ;
public :
    constexpr udo_class2 ( T && arg1 , F func )
        : x { std::forward < T > ( arg1 ) }
        , f { func }
    {}
    template < typename U >
    constexpr auto operator | ( U && y )
    {
        return f ( std::move ( x ) , std::forward < U > ( y ) ) ;
    }
} ;

template < typename F >
struct udo_class
{
private :
    F f ;
public :
    constexpr udo_class ( F func )
        : f { std::move ( func ) }
    {}
    template < typename T >
    friend constexpr auto operator | ( T && x , udo_class me )
    {
        return udo_class2 < T , F > ( std::forward < T > ( x ) , std::move ( me.f ) ) ;
    }
} ;

template < typename F >
constexpr auto udo ( F f )
{
    return udo_class < F > { f } ;
}

constexpr auto plus_int ( int a , int b )
{
    return a + b ;
}

#define のフォーマットで表示する |udo(std::printf)|
#define たす |udo(plus_int)|

int main ()
{
    "%d\n" のフォーマットで表示する 0 ;
    constexpr auto a = 1 たす 6 ;
    "%d\n" のフォーマットで表示する a ;
}