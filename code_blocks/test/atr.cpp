#include <iostream>
#include "xtechnical_indicators.hpp"
#include <random>
#include <array>

int main(int argc, char* argv[]) {
    std::cout << "Hello world!" << std::endl;
    std::array<double, 20> test_data = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};

    const size_t period = 2;
    xtechnical::ATR<double, xtechnical::SMA<double>> atr(period);

    for(size_t i = 0; i < test_data.size(); ++i) {
        atr.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " get " << atr.get()
            << std::endl;
    }

    /* проверяем методы тест */
    std::cout << "test(20)" << std::endl;
    atr.test(20);
    std::cout << "get " << atr.get() << std::endl;


    std::cout << "test(21)" << std::endl;
    atr.test(21);
    std::cout << "get " << atr.get() << std::endl;

    std::cout << "test(10)" << std::endl;
    atr.test(10);
    std::cout << "get " << atr.get() << std::endl;

    for(size_t i = test_data.size() - 1; i > 0; --i) {
        atr.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " get " << atr.get()
            << std::endl;
    }

    for(size_t i = 0; i < test_data.size(); ++i) {
        atr.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " get " << atr.get()
            << std::endl;
    }

    std::system("pause");
    return 0;
}
