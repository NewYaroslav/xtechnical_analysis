#include <iostream>
#include "xtechnical_statistics.hpp"
#include <array>

int main() {
    std::cout << "Hello world!" << std::endl;
    std::array<double,6> test_data = {3,8,10,17,24,27};

    double harmonic_mean = xtechnical_statistics::calc_harmonic_mean<double>(test_data);
    std::cout << "harmonic mean: " << harmonic_mean << std::endl;
    double mean_value = xtechnical_statistics::calc_mean_value<double>(test_data);
    std::cout << "mean value: " << mean_value << std::endl;


    double skewness = xtechnical_statistics::calc_skewness<double>(test_data);
    std::cout << "skewness: " << skewness << std::endl;

    double snr = xtechnical_statistics::calc_signal_to_noise_ratio<double>(test_data);
    std::cout << "snr: " << snr << std::endl;


    std::array<double,6> test_data2 = {1.1,1.1,1.1,1.1,1.1,1.1};
    double geometric_mean = xtechnical_statistics::calc_geometric_mean<double>(test_data2);
    std::cout << "geometric mean: " << geometric_mean << std::endl;
    return 0;
}
