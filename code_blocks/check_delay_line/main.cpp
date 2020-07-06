#include <iostream>
#include <array>
#include "xtechnical_indicators.hpp"

using namespace std;

int main() {
    cout << "Hello world!" << endl;
    std::array<double, 20> test_data = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
    xtechnical_indicators::DelayLine<double> delay_line(1);
    for(size_t i = 0; i < test_data.size(); ++i) {
        delay_line.update(test_data[i]);
        std::cout
            << "date " << test_data[i]
            << " get " << delay_line.get()
            << std::endl;
    }
    for(size_t i = 0; i < test_data.size(); ++i) {
        delay_line.test(test_data[i]);
        std::cout
            << "test date " << test_data[i]
            << " get test " << delay_line.get()
            << std::endl;
    }
    delay_line.clear();
    for(size_t i = 0; i < test_data.size(); ++i) {
        delay_line.update(test_data[i]);
        std::cout
            << "date " << test_data[i]
            << " get " << delay_line.get()
            << std::endl;
    }
    return 0;
}
