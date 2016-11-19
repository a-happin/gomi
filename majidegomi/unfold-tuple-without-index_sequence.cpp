#include <utility>
#include <iostream>
#include <tuple>
#include <experimental/optional>

using std::experimental::optional ;
using std::experimental::nullopt ;

template < typename F , typename ... Ts >
auto apply_impl ( F f , const std::tuple < Ts ... > & t , optional < Ts > && ... args )
{
    std::tie ( args ... ) = t ;
    return f ( std::move ( * args ) ... ) ;
}

template < typename F , typename ... Ts >
auto apply ( F f , const std::tuple < Ts ... > & t )
{
    return apply_impl ( f , t , static_cast < optional < Ts > > ( nullopt ) ... ) ;
}

struct X
{
    int a ;
    constexpr X ( int x )
        : a { x }
    {
    }
} ;

auto print ( int n , const char * s , double d , optional < int > opt , X x )
{
    std::cout << n << std::endl ;
    std::cout << s << std::endl ;
    std::cout << d << std::endl ;
    if ( opt )
    {
        std::cout << * opt << std::endl ;
    }
    else
    {
        std::cout << "nullopt" << std::endl ;
    }
    std::cout << "X " << x.a << std::endl ;
}

auto main () -> int
{
    auto t = std::make_tuple ( 0 , "Hello World!" , 3.14 , optional < int > ( nullopt ) , X { 1 } ) ;
    apply ( print , t ) ;
}