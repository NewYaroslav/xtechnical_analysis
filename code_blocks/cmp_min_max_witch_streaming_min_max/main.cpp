#include <iostream>
#include <array>
#include <chrono>
#include "xtechnical_indicators.hpp"
#include "../../include/xtechnical_streaming_min_max.hpp"

using namespace std;

int main() {
    cout << "Hello world!" << endl;

    std::vector<double> test_data;
    for(size_t i = 0; i < 100; ++i) {
        test_data.push_back(i);
    }
    for(size_t i = 0; i < 30; ++i) {
        test_data.push_back(100);
    }
    for(int i = 100; i >= 0; --i) {
        test_data.push_back(i);
    }
    for(size_t i = 0; i < 30; ++i) {
        test_data.push_back(0);
    }

    xtechnical_indicators::MinMax<double> min_max(30, 0);
    xtechnical_indicators::FastMinMax<double> fast_min_max(30, 0);
    xtechnical::StreamingMaximumMinimumFilter<double> streaming_min_max(30);

    std::cout << "-1-" << std::endl;
    for(size_t n = 0; n < 1000; ++n)
    for(size_t i = 0; i < test_data.size(); ++i) {
        min_max.update(test_data[i]);
        streaming_min_max.update(test_data[i]);
        fast_min_max.update(test_data[i]);
        //std::cout << "in " << test_data[i] << " min " << min_max.get_min() << " smin " << streaming_min_max.get_min() << std::endl;
        if (!std::isnan(min_max.get_min()) && !std::isnan(streaming_min_max.get_min()) && min_max.get_min() != streaming_min_max.get_min()) {
            std::cout << "error! index " << i << " in " << test_data[i] << " min " << min_max.get_min() << " s min " << streaming_min_max.get_min() << std::endl;
            return 0;
        }
        if (!std::isnan(min_max.get_max()) && !std::isnan(streaming_min_max.get_max()) && min_max.get_max() != streaming_min_max.get_max()) {
            std::cout << "error! index " << i << " in " << test_data[i] << " max " << min_max.get_min() << " s max " << streaming_min_max.get_max() << std::endl;
            return 0;
        }
        if (!std::isnan(min_max.get_min()) && !std::isnan(fast_min_max.get_min()) && min_max.get_min() != fast_min_max.get_min()) {
            std::cout << "error! index " << i << " in " << test_data[i] << " min " << min_max.get_min() << " s min " << fast_min_max.get_min() << std::endl;
            return 0;
        }
    }
    std::cout << "ok" << std::endl;

    {
        xtechnical_indicators::MinMax<double> min_max(30, 2);
        xtechnical_indicators::FastMinMax<double> fast_min_max(30, 2);
        std::cout << "-2-" << std::endl;
        for(size_t n = 0; n < 1000; ++n)
        for(size_t i = 0; i < test_data.size(); ++i) {
            min_max.update(test_data[i]);
            fast_min_max.update(test_data[i]);
            if (!std::isnan(min_max.get_min()) && !std::isnan(fast_min_max.get_min()) && min_max.get_min() != fast_min_max.get_min()) {
                std::cout << "error! index " << i << " in " << test_data[i] << " min " << min_max.get_min() << " s min " << fast_min_max.get_min() << std::endl;
                return 0;
            }
        }
        std::cout << "ok" << std::endl;
    }

    {
        xtechnical_indicators::MinMax<double> min_max(60, 0);
        auto begin = std::chrono::steady_clock::now();
        for(size_t n = 0; n < 100000; ++n)
        for(size_t i = 0; i < test_data.size(); ++i) {
            min_max.update(test_data[i]);
        }
        auto end = std::chrono::steady_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
        std::cout << "min max time: " << elapsed_ms.count() << " ms\n";
    }
    {
        xtechnical::StreamingMaximumMinimumFilter<double> streaming_min_max(60);
        auto begin = std::chrono::steady_clock::now();
        for(size_t n = 0; n < 100000; ++n)
        for(size_t i = 0; i < test_data.size(); ++i) {
            streaming_min_max.update(test_data[i]);
        }
        auto end = std::chrono::steady_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
        std::cout << "streaming min max time: " << elapsed_ms.count() << " ms\n";
    }
    {
        xtechnical_indicators::FastMinMax<double> min_max(60, 0);
        auto begin = std::chrono::steady_clock::now();
        for(size_t n = 0; n < 100000; ++n)
        for(size_t i = 0; i < test_data.size(); ++i) {
            streaming_min_max.update(test_data[i]);
        }
        auto end = std::chrono::steady_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
        std::cout << "fast min max time: " << elapsed_ms.count() << " ms\n";
    }
    return 0;
}
