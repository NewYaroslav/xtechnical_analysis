#include <iostream>
#include <array>
#include "xtechnical_indicators.hpp"

using namespace std;

int main() {
    cout << "Hello world!" << endl;
    std::array<double, 20> test_data = {0,1,2,4,8,5,6,7,8,16,10,12,14,16,14,15,16,17,18,19};
    xtechnical_indicators::PRI<double> pri(4);
    std::cout << "---" << std::endl;
    for(size_t i = 0; i < test_data.size(); ++i) {
        pri.update(test_data[i]);
        std::cout
            << "date " << test_data[i]
            << " get " << pri.get()
            << std::endl;
    }
    for(size_t i = 0; i < test_data.size(); ++i) {
        pri.test(test_data[i]);
        std::cout
            << "date test " << test_data[i]
            << " get " << pri.get()
            << std::endl;
    }
    for(size_t i = 0; i < test_data.size(); ++i) {
        pri.update(test_data[i]);
        std::cout
            << "date " << test_data[i]
            << " get " << pri.get()
            << std::endl;
    }
    pri.clear();
    for(size_t i = 0; i < test_data.size(); ++i) {
        pri.update(test_data[i]);
        std::cout
            << "date " << test_data[i]
            << " get " << pri.get()
            << std::endl;
    }
    return 0;
}
