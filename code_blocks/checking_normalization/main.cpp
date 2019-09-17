#include <iostream>
#include "xtechnical_indicators.hpp"
#include <array>

int main() {
    std::cout << "Hello world!" << std::endl;
    std::array<double,7> test_data = {1,2,3,1,0.5,6,6};
    std::array<double,7> out_data;
    xtechnical_normalization::calculate_min_max(test_data, out_data, xtechnical_common::MINMAX_UNSIGNED);
    std::cout << "calculate_min_max (MINMAX_UNSIGNED):" << std::endl;
    for(size_t i = 0; i < out_data.size(); ++i) {
        std::cout << out_data[i] << std::endl;
    }

    xtechnical_normalization::calculate_min_max(test_data, out_data, xtechnical_common::MINMAX_SIGNED);
    std::cout << "calculate_min_max (MINMAX_SIGNED):" << std::endl;
    for(size_t i = 0; i < out_data.size(); ++i) {
        std::cout << out_data[i] << std::endl;
    }

    xtechnical_normalization::calculate_zscore(test_data, out_data);
    std::cout << "calculate_zscore:" << std::endl;
    for(size_t i = 0; i < out_data.size(); ++i) {
        std::cout << out_data[i] << std::endl;
    }

    std::array<double,6> out_data_2;
    xtechnical_normalization::calculate_difference(test_data, out_data_2);
    std::cout << "calculate_difference:" << std::endl;
    for(size_t i = 0; i < out_data_2.size(); ++i) {
        std::cout << out_data_2[i] << std::endl;
    }


    xtechnical_normalization::normalize_amplitudes(test_data, out_data, (double)1.0);
    std::cout << "normalize_amplitudes:" << std::endl;
    for(size_t i = 0; i < out_data.size(); ++i) {
        std::cout << out_data[i] << std::endl;
    }

    xtechnical_normalization::calculate_log(test_data, out_data);
    std::cout << "calculate_log:" << std::endl;
    for(size_t i = 0; i < out_data.size(); ++i) {
        std::cout << out_data[i] << std::endl;
    }


    xtechnical_normalization::calc_automatic_gain_control<xtechnical_indicators::LowPassFilter<double>>(test_data, out_data, 3);
    std::cout << "automatic_gain_control:" << std::endl;
    for(size_t i = 0; i < out_data.size(); ++i) {
        std::cout << out_data[i] << std::endl;
    }
    return 0;
}
