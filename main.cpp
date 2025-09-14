#include <iostream>

#include "Tuple.h"

int main()
{
    tuple<int, double, char> t{ 42, 3.14, 'a' };

    std::cout << get<0>(t) << '\n'; // 42
    std::cout << get<1>(t) << '\n'; // 3.14
    std::cout << get<2>(t) << '\n'; // a

    auto t2 = make_tuple(10, 20.5, 'x');

    if (t == t2)
        std::cout << "t and t2 is equal\n";
    else
        std::cout << "t and t2 is no equal\n";

    return 0;
}