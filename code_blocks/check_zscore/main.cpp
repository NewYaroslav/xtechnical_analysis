#include <iostream>
#include <array>
#include "xtechnical_indicators.hpp"
#include "xtechnical_normalization.hpp"

using namespace std;

int main() {
    cout << "Hello world!" << endl;

    std::array<double, 5> zscore_test_data = {1,2,3,4,5};
    std::array<double, 5> zscore_out;
    xtechnical_normalization::calculate_zscore(zscore_test_data, zscore_out, 1, 1000);
    for(size_t i = 0; i < zscore_out.size(); ++i) {
        std::cout
            << "zscore " << zscore_out[i]
            << std::endl;
    }

    std::system("pause");
    std::array<double, 20> test_data = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
    xtechnical_indicators::Zscore<double> zscore(5);
    for(size_t i = 0; i < test_data.size(); ++i) {
        zscore.update(test_data[i]);
        std::cout
            << "date " << test_data[i]
            << " get " << zscore.get()
            << std::endl;
    }
    for(size_t i = 0; i < test_data.size(); ++i) {
        zscore.test(test_data[i]);
        std::cout
            << "date " << test_data[i]
            << " get test " << zscore.get()
            << std::endl;
    }
    for(size_t i = 0; i < test_data.size(); ++i) {
        zscore.update(test_data[i]);
        std::cout
            << "date " << test_data[i]
            << " get " << zscore.get()
            << std::endl;
    }
    zscore.clear();
    for(size_t i = 0; i < test_data.size(); ++i) {
        zscore.update(test_data[i]);
        std::cout
            << "date " << test_data[i]
            << " get " << zscore.get()
            << std::endl;
    }
    return 0;
}
