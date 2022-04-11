#include <iostream>
#include "xtechnical_indicators.hpp"
#include <random>
#include <array>

int main(int argc, char* argv[]) {
    std::cout << "Hello world!" << std::endl;
    std::array<double, 20> test_data = {0,12,2,91,42,54,84,74,88,98,101,111,122,132,143,153,126,117,118,119};

    xtechnical::Fractals<double> fractals;

    for(size_t i = 0; i < test_data.size(); ++i) {
        fractals.update(test_data[i], test_data[i] - 1);
        std::cout
            << "update " << test_data[i]
            << " up " << fractals.get_up()
            << " dn " << fractals.get_dn()
            << std::endl;
    }

    /* проверяем методы тест */
    std::cout << "test(20, 19)" << std::endl;
    fractals.test(20, 19);
    std::cout << "up " << fractals.get_up() << " dn " << fractals.get_dn() << std::endl;

    std::cout << "test(21, 18)" << std::endl;
    fractals.test(21, 18);
    std::cout << "up " << fractals.get_up() << " dn " << fractals.get_dn() << std::endl;

    std::cout << "test(10, 8)" << std::endl;
    fractals.test(10, 8);
    std::cout << "up " << fractals.get_up() << " dn " << fractals.get_dn() << std::endl;

    for(size_t i = test_data.size() - 1; i > 0; --i) {
        fractals.update(test_data[i], test_data[i] - 2);
        std::cout
            << "update " << test_data[i]
            << " up " << fractals.get_up()
            << " dn " << fractals.get_dn()
            << std::endl;
    }

    for(size_t i = 0; i < test_data.size(); ++i) {
        fractals.update(test_data[i], test_data[i] - 1);
        std::cout
            << "update " << test_data[i]
            << " up " << fractals.get_up()
            << " dn " << fractals.get_dn()
            << std::endl;
    }

    std::system("pause");
    return 0;
}
