#include "code.cpp"
#include <iostream>
#include <string>

int main() {
    sjtu::int2048 a("123456789012345678901234567890");
    sjtu::int2048 b("-99999999999999999999");
    std::cout << (a + b) << '\n';
    std::cout << (a - b) << '\n';
    std::cout << (a * b) << '\n';
    std::cout << (a / b) << '\n';
    std::cout << (a % b) << '\n';
    sjtu::int2048 c;
    std::cin >> c;
    std::cout << c << '\n';
    return 0;
}
