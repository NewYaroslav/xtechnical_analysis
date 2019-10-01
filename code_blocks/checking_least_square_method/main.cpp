#include <iostream>
#include "xtechnical_regression_analysis.hpp"
#include <array>
#include <vector>

int main() {
    std::cout << "Hello world!" << std::endl;

    double coeff[3];
    double test_data[6][2] = {
        {1,2},
        {2,3},
        {3,4},
        {4,5},
        {5,6},
        {6,7},
    };

    std::array<double[2],6> test_data2 = {
        1,2,3,4,5,6,
        2,3,4,5,6,7
    };

    std::vector<std::array<double,2>> test_data3;
    test_data3.resize(6);
    test_data3[0][0] = 1;
    test_data3[0][1] = 2;
    test_data3[1][0] = 2;
    test_data3[1][1] = 3;
    test_data3[2][0] = 3;
    test_data3[2][1] = 4;
    test_data3[3][0] = 4;
    test_data3[3][1] = 5;
    test_data3[4][0] = 5;
    test_data3[4][1] = 6;
    test_data3[5][0] = 6;
    test_data3[5][1] = 7;

    xtechnical_regression_analysis::calc_least_squares_method(coeff, test_data, 6, xtechnical_regression_analysis::LSM_PARABOLA);
    double in_data = 7;
    double out_data = xtechnical_regression_analysis::calc_line(coeff, in_data, xtechnical_regression_analysis::LSM_PARABOLA);
    std::cout << "out_data " << out_data << std::endl;

    xtechnical_regression_analysis::calc_least_squares_method(coeff, test_data2, 6, xtechnical_regression_analysis::LSM_PARABOLA);
    out_data = xtechnical_regression_analysis::calc_line(coeff, in_data, xtechnical_regression_analysis::LSM_PARABOLA);
    std::cout << "out_data " << out_data << std::endl;

    xtechnical_regression_analysis::calc_least_squares_method(coeff, test_data3, 6, xtechnical_regression_analysis::LSM_PARABOLA);
    out_data = xtechnical_regression_analysis::calc_line(coeff, in_data, xtechnical_regression_analysis::LSM_PARABOLA);
    std::cout << "out_data " << out_data << std::endl;
    return 0;
}
