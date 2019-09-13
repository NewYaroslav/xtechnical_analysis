#include <iostream>
#include "xtechnical_indicators.hpp"
#include <array>

int main() {
    std::cout << "Hello world!" << std::endl;
    std::array<double,7> test_data = {1,2,3,1,0.5,6,6};

    double temp = 0;
    xtechnical_indicators::calculate_sma(test_data, temp, 4, 1);
    std::cout << "calculate_sma:" << std::endl;
    std::cout << temp << std::endl;
    xtechnical_indicators::calculate_std_dev(test_data, temp, 4, 1);
    std::cout << "calculate_std_dev:" << std::endl;
    std::cout << temp << std::endl;
    double temp2 = 0;
    xtechnical_indicators::calculate_std_dev_and_mean(test_data, temp, temp2, 4, 1);
    std::cout << "calculate_std_dev_and_mean:" << std::endl;
    std::cout << temp << std::endl;
    std::cout << temp2 << std::endl;

    std::array<double,7> test_data2 = {100,101,99,97,110,105,99};
    std::array<double,7> out_tl, out_ml, out_bl;
    xtechnical_indicators::calc_ring_bollinger(test_data2, out_tl,  out_ml, out_bl, 4, 2);
    std::cout << "calc_ring_bollinger:" << std::endl;
    for(size_t i = 0; i < test_data2.size(); ++i) {
        std::cout << out_tl[i] << " " << test_data2[i] << " " << out_bl[i] << std::endl;
    }

    std::array<double,7> test_data3 = {100,101,99,97,110,105,99};
    std::array<double,7> out_rsi;
    xtechnical_indicators::calc_ring_rsi(test_data2, out_rsi, 4);
    std::cout << "calc_ring_rsi:" << std::endl;
    for(size_t i = 0; i < test_data3.size(); ++i) {
        std::cout << out_rsi[i] << " " << test_data3[i]  << std::endl;
    }

    return 0;
}
