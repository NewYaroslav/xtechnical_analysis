#include <iostream>
#include "xtechnical_indicators.hpp"
#include <array>

int main() {
    std::cout << "Hello world!" << std::endl;
    std::array<double, 20> test_data = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};

    const size_t period = 2;
    xtechnical_indicators::TrendDirectionForceIndex<double, xtechnical_indicators::EMA<double>> trend(period);

    for(size_t i = 0; i < test_data.size(); ++i) {
        trend.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " get " << trend.get()
            << std::endl;
    }

    /* проверяем методы тест */
    std::cout << "test(20)" << std::endl;
    trend.test(20);
    std::cout << "get " << trend.get() << std::endl;


    std::cout << "test(21)" << std::endl;
    trend.test(21);
    std::cout << "get " << trend.get() << std::endl;

    std::cout << "test(10)" << std::endl;
    trend.test(10);
    std::cout << "get " << trend.get() << std::endl;

    for(size_t i = test_data.size() - 1; i > 0; --i) {
        trend.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " get " << trend.get()
            << std::endl;
    }

    for(size_t i = 0; i < test_data.size(); ++i) {
        trend.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " get " << trend.get()
            << std::endl;
    }

    std::system("pause");
    return 0;
}
