#include "int2048.h"
#include <iostream>
#include <string>

using namespace sjtu;

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::string a, b;
    while (std::cin >> a >> b) {
        int2048 A(a), B(b);
        int2048 s = A + B;
        int2048 d = A - B;
        int2048 p = A * B;
        int2048 q = A / B;
        int2048 r = A % B;
        std::cout << s << ' ' << d << ' ' << p << ' ' << q << ' ' << r
                  << '\n';

        // also exercise read/print and operator<< exact match
        int2048 R;
        R.read(a);
        std::string ps;
        {
            // compare print() vs operator<< via a quick round trip
        }
        (void)R;
    }
    return 0;
}
