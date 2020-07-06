#include <iostream>
#include <array>
#include "xtechnical_indicators.hpp"

using namespace std;

int main() {
    cout << "Hello world!" << endl;
    std::array<double, 20> test_data = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
    xtechnical_indicators::MinMax<double> min_max(4, 1);
    std::cout << "---" << std::endl;
    for(size_t i = 0; i < test_data.size(); ++i) {
        min_max.update(test_data[i]);
        std::cout
            << "date " << test_data[i]
            << " min " << min_max.get_min()
            << " max " << min_max.get_max()
            << std::endl;
    }
    for(size_t i = 0; i < test_data.size(); ++i) {
        min_max.test(test_data[i]);
        std::cout
            << "date test " << test_data[i]
            << " min " << min_max.get_min()
            << " max " << min_max.get_max()
            << std::endl;
    }
    for(size_t i = 0; i < test_data.size(); ++i) {
        min_max.update(test_data[i]);
        std::cout
            << "date " << test_data[i]
            << " min " << min_max.get_min()
            << " max " << min_max.get_max()
            << std::endl;
    }
    min_max.clear();
    for(size_t i = 0; i < test_data.size(); ++i) {
        min_max.update(test_data[i]);
        std::cout
            << "date " << test_data[i]
            << " min " << min_max.get_min()
            << " max " << min_max.get_max()
            << std::endl;
    }
    return 0;
}
