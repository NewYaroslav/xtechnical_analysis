#include <iostream>
#include "xtechnical_indicators.hpp"
#include <array>

int main() {
    std::cout << "Hello world!" << std::endl;
    std::array<double, 20> test_data = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};

    xtechnical_indicators::RSHILLMA<double, xtechnical_indicators::SMA<double>, xtechnical_indicators::SMA<double>> iRSHILLMA(5, 10, 100, 1.5);

    for(size_t j = 0; j < 100; ++j)
    for(size_t i = 0; i < test_data.size(); ++i) {
        iRSHILLMA.update(test_data[i]);
    }

    for(size_t i = 0; i < test_data.size(); ++i) {
        iRSHILLMA.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " get " << iRSHILLMA.get()
            << std::endl;
    }

    /* проверяем методы тест */
    std::cout << "test(20)" << std::endl;
    iRSHILLMA.test(20);
    std::cout << "get " << iRSHILLMA.get() << std::endl;


    std::cout << "test(21)" << std::endl;
    iRSHILLMA.test(21);
    std::cout << "get " << iRSHILLMA.get() << std::endl;

    std::cout << "test(10)" << std::endl;
    iRSHILLMA.test(10);
    std::cout << "get " << iRSHILLMA.get() << std::endl;

    for(size_t i = test_data.size() - 1; i > 0; --i) {
        iRSHILLMA.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " get " << iRSHILLMA.get()
            << std::endl;
    }

    for(size_t i = 0; i < test_data.size(); ++i) {
        iRSHILLMA.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " get " << iRSHILLMA.get()
            << std::endl;
    }

    std::system("pause");
    return 0;
}
