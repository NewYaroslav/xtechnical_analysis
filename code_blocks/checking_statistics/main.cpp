#include <iostream>
#include "xtechnical_statistics.hpp"
#include <array>
#include <vector>

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

    std::array<double,3> test_out;
    std::vector<double> test_data3 = {1.1,1.1,1.1,1.1,1.1,1.1};
    test_out[0] = xtechnical_statistics::calc_median<double>(test_data3);
    std::cout << "median: " << test_out[0] << std::endl;


    std::vector<double> test_data4 = {1,1,2,2,3,3,4,5,6,7};
    double excess = xtechnical_statistics::calc_excess<double>(test_data4);
    std::cout << "excess: " << excess << std::endl;


    std::vector<double> test_data5 = {1,1,1,0,0,1};
    double standard_error = xtechnical_statistics::calc_standard_error<double>(test_data5);
    std::cout << "standard_error: " << standard_error << std::endl;
    return 0;
}
