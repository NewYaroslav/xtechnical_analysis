#include <iostream>
#include <array>
#include "xtechnical_indicators.hpp"

using namespace std;

int main() {
    cout << "Hello world!" << endl;
    std::array<double, 20> test_data = {0,1,2,4,8,5,6,7,8,16,10,12,14,16,14,15,16,17,18,19};
    xtechnical::CRSI<double,xtechnical::SMA<double>> crsi(3,5,10);
    std::cout << "-0-" << std::endl;
    for(size_t i = 0; i < test_data.size(); ++i) {
        crsi.update(test_data[i]);
        std::cout
            << "i " << test_data[i]
            << " crsi " << crsi.get()
            << std::endl;
    }
    std::cout << "-1-" << std::endl;
    for(size_t i = 0; i < test_data.size(); ++i) {
        crsi.test(test_data[i]);
        std::cout
            << "i " << test_data[i]
            << " crsi " << crsi.get()
            << std::endl;
    }
    std::cout << "-2-" << std::endl;
    for(size_t i = 0; i < test_data.size(); ++i) {
        crsi.update(test_data[i]);
        std::cout
            << "i " << test_data[i]
            << " crsi " << crsi.get()
            << std::endl;
    }
    std::cout << "-3-" << std::endl;
    crsi.clear();
    for(size_t i = 0; i < test_data.size(); ++i) {
        crsi.update(test_data[i]);
        std::cout
            << "i " << test_data[i]
            << " crsi " << crsi.get()
            << std::endl;
    }
    return 0;
}
