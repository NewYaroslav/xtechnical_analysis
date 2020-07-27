#include <iostream>
#include "xtechnical_statistics.hpp"
#include <array>
#include <vector>

int main() {
    std::cout << "Hello world!" << std::endl;
    std::array<double,6> test_data = {3,8,10,17,24,27};

    double median_absolute_deviation_value = xtechnical_statistics::calc_median_absolute_deviation<double>(test_data);
    std::cout << "median absolute deviation value: " << median_absolute_deviation_value << std::endl;

    double median_value = xtechnical_statistics::calc_median<double>(test_data);
    std::cout << "median value: " << median_value << std::endl;

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

    double integral_laplace = xtechnical_statistics::calc_integral_laplace<double>(0.01, 0.00001);
    std::cout << "integral_laplace: " << integral_laplace << std::endl;

    double integral_laplace2 = xtechnical_statistics::calc_integral_laplace<double>(1.51, 0.01);
    std::cout << "integral_laplace2: " << integral_laplace2 << std::endl;

    double p_bet = xtechnical_statistics::calc_probability_winrate<double>(0.6, 31, 44);
    std::cout << "p_bet: " << p_bet << std::endl; // получим ответ 93.6

    p_bet = xtechnical_statistics::calc_probability_winrate<double>(0.56, 5700, 10000);
    std::cout << "p_bet: " << p_bet << std::endl;

    p_bet = xtechnical_statistics::calc_probability_winrate<double>(0.54, 5700, 10000);
    std::cout << "p_bet: " << p_bet << std::endl;

    p_bet = xtechnical_statistics::calc_probability_winrate<double>(0.57, 1, 1);
    std::cout << "p_bet (1): " << p_bet << std::endl;
    return 0;
}
