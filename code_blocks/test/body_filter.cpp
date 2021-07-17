#include <iostream>
#include "xtechnical_indicators.hpp"
#include <random>
#include <array>

int main(int argc, char* argv[]) {
    std::cout << "Hello world!" << std::endl;

    {
        xtechnical::BodyFilter<double, xtechnical::SMA<double>> body_filter(1);
        body_filter.update(2,5,1,3);
        std::cout << body_filter.get() << std::endl;
    }
    {
        xtechnical::BodyFilter<double, xtechnical::SMA<double>> body_filter(3);
        body_filter.update(2,5,1,3);
        body_filter.update(3,5,1,3);
        body_filter.update(2,4,2,4);
        body_filter.update(2,4,2,4);
        std::cout << body_filter.get() << std::endl;
    }
    std::system("pause");
    return 0;
}
