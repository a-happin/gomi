#include <type_traits>
#include <iostream>

using mytype = double ;

static std::aligned_storage_t < sizeof ( mytype ) , alignof ( mytype ) > storage ;

template < typename ... Args >
auto f ( Args && ... args )
{
    return new ( & storage ) mytype ( args ... ) ;
}

auto main ( ) -> int
{
    auto p = f ( 40 ) ;
    if ( *p == 40 )
    {
        std::cout << "OK" << std::endl ;
    }
    else
    {
        std::cout << "error" << std::endl ;
    }
    return 0 ;
}