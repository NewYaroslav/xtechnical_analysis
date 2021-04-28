#include <iostream>
#include <array>
#include "xtechnical_indicators.hpp"

using namespace std;

int main() {
    cout << "Hello world!" << endl;
    std::array<double, 20> test_data = {0,1,2,4,8,5,6,7,8,16,10,12,14,16,14,15,16,17,18,19};
    xtechnical_indicators::Stochastics<double,xtechnical_indicators::SMA<double>> stochastics(10,3,5);
    std::cout << "-0-" << std::endl;
    for(size_t i = 0; i < test_data.size(); ++i) {
        stochastics.update(test_data[i]);
        std::cout
            << "i " << test_data[i]
            << " k " << stochastics.get()
            << " %k " << stochastics.get_k()
            << " %d " << stochastics.get_d()
            << std::endl;
    }
    std::cout << "-1-" << std::endl;
    for(size_t i = 0; i < test_data.size(); ++i) {
        stochastics.test(test_data[i]);
        std::cout
            << "i " << test_data[i]
            << " k " << stochastics.get()
            << " %k " << stochastics.get_k()
            << " %d " << stochastics.get_d()
            << std::endl;
    }
    std::cout << "-2-" << std::endl;
    for(size_t i = 0; i < test_data.size(); ++i) {
        stochastics.update(test_data[i]);
        std::cout
            << "i " << test_data[i]
            << " k " << stochastics.get()
            << " %k " << stochastics.get_k()
            << " %d " << stochastics.get_d()
            << std::endl;
    }
    std::cout << "-3-" << std::endl;
    stochastics.clear();
    for(size_t i = 0; i < test_data.size(); ++i) {
        stochastics.update(test_data[i]);
        std::cout
            << "i " << test_data[i]
            << " k " << stochastics.get()
            << " %k " << stochastics.get_k()
            << " %d " << stochastics.get_d()
            << std::endl;
    }
    return 0;
}
