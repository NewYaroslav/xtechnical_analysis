#include <iostream>
#include <random>
#include <array>
#include <vector>
#include <cmath>
#include <Eigen/Dense>
#include "xtechnical_indicators.hpp"
#include <chrono>

int main(int argc, char* argv[]) {
    std::cout << "Hello world!" << std::endl;
    Eigen::VectorXf x(11);
    x << 1, 2, 3, 2, 1, 2, 3, 2, 1, 2, 3;

    int M = 6; // number of ticks to forecast after the end of the time series
    int K = 5; // window length
    //int r = 0; // rank of the Hankel matrix

    std::cout << "res:\n" << xtechnical::SSA<float>::calc_ssa(x, M, K, 0, xtechnical::SSA<float>::SSAMode::OriginalSeriesAddition) << std::endl;

    xtechnical::SSA<float> SSA(11);

    for (size_t i = 0; i < x.size(); ++i) {
        SSA.update(x(i), xtechnical::common::PriceType::Close);
    }

    SSA.calc(M, K, 0, 0, xtechnical::SSA<float>::MetricType::RSquared, true, xtechnical::SSA<float>::SSAMode::RestoredSeriesAddition);

    std::cout << "get_last_forecast:\n" << SSA.get_last_forecast() << std::endl;

    std::cout << "get_forecast:\n";
    for (auto &item : SSA.get_forecast()) {
        std::cout << item << " ";

    }
    std::cout << std::endl;

    std::cout << "get_reconstructed:\n";
    for (auto &item : SSA.get_reconstructed()) {
        std::cout << item << " ";

    }
    std::cout << std::endl;

    std::cout << "get_metric:\n" << SSA.get_metric() << std::endl;

    return 0;
}
