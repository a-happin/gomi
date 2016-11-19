#include <iostream>

int main ()
{
    constexpr auto size = 10000u ;
    constexpr auto p = new int [size];
    for (int i = 0; i < size; ++ i)
    {
        p [i] = i;
        std::cout << p [i] << std::endl ; // コンパイル時にnewした配列がコンパイル終了時にdeleteされるならこのアクセスはどうなる？
    }
    static_assert (* p == 0 , "");
    delete [] p ;
}