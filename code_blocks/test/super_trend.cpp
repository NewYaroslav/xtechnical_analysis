#include <iostream>
#include "xtechnical_indicators.hpp"
#include <random>
#include <array>

int main(int argc, char* argv[]) {
    std::cout << "Hello world!" << std::endl;
    std::array<double, 20> test_data = {0,1,2,9,4,5,8,7,8,9,10,11,12,13,14,15,16,17,18,19};

    xtechnical::SuperTrend<double, xtechnical::SMA<double>> super_trend(50,5);

    for(size_t n = 0; n < 10; ++n)
    for(size_t i = 0; i < test_data.size(); ++i) {
        super_trend.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " get " << super_trend.get()
            << std::endl;
    }

    /* проверяем методы тест */
    std::cout << "test(20)" << std::endl;
    super_trend.test(20);
    std::cout << "get " << super_trend.get() << std::endl;


    std::cout << "test(21)" << std::endl;
    super_trend.test(21);
    std::cout << "get " << super_trend.get() << std::endl;

    std::cout << "test(10)" << std::endl;
    super_trend.test(10);
    std::cout << "get " << super_trend.get() << std::endl;

    for(size_t i = test_data.size() - 1; i > 0; --i) {
        super_trend.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " get " << super_trend.get()
            << std::endl;
    }

    for(size_t i = 0; i < test_data.size(); ++i) {
        super_trend.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " get " << super_trend.get()
            << std::endl;
    }

    std::system("pause");
    return 0;
}
