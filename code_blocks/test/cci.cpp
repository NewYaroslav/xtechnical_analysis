#include <iostream>
#include "xtechnical_indicators.hpp"
#include <random>
#include <array>

int main(int argc, char* argv[]) {
    std::cout << "Hello world!" << std::endl;
    std::array<double, 20> test_data = {0,1,2,9,4,5,8,7,8,9,10,11,12,13,14,15,16,17,18,19};

    const size_t period = 4;
    xtechnical::CCI<double, xtechnical::SMA<double>> cci(period);

    for(size_t i = 0; i < test_data.size(); ++i) {
        cci.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " get " << cci.get()
            << std::endl;
    }

    /* проверяем методы тест */
    std::cout << "test(20)" << std::endl;
    cci.test(20);
    std::cout << "get " << cci.get() << std::endl;


    std::cout << "test(21)" << std::endl;
    cci.test(21);
    std::cout << "get " << cci.get() << std::endl;

    std::cout << "test(10)" << std::endl;
    cci.test(10);
    std::cout << "get " << cci.get() << std::endl;

    for(size_t i = test_data.size() - 1; i > 0; --i) {
        cci.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " get " << cci.get()
            << std::endl;
    }

    for(size_t i = 0; i < test_data.size(); ++i) {
        cci.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " get " << cci.get()
            << std::endl;
    }

    std::system("pause");
    return 0;
}
