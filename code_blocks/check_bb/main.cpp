#include <iostream>
#include <array>
#include "xtechnical_indicators.hpp"
#include "xtechnical_normalization.hpp"

using namespace std;

int main() {
    cout << "Hello world!" << endl;
    std::array<double, 20> test_data = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
    xtechnical_indicators::BollingerBands<double> bb(3,2,1);
    for(size_t i = 0; i < test_data.size(); ++i) {
        bb.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " tl " << bb.get_tl()
            << " ml " << bb.get_ml()
            << " bl " << bb.get_bl()
            << std::endl;
    }
    for(size_t i = 0; i < test_data.size(); ++i) {
        bb.test(test_data[i]);
        std::cout
            << "test " << test_data[i]
            << " tl " << bb.get_tl()
            << " ml " << bb.get_ml()
            << " bl " << bb.get_bl()
            << std::endl;
    }
    for(size_t i = 0; i < test_data.size(); ++i) {
        bb.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " tl " << bb.get_tl()
            << " ml " << bb.get_ml()
            << " bl " << bb.get_bl()
            << std::endl;
    }
    bb.clear();
    for(size_t i = 0; i < test_data.size(); ++i) {
        bb.update(test_data[i]);
        std::cout
            << "update " << test_data[i]
            << " tl " << bb.get_tl()
            << " ml " << bb.get_ml()
            << " bl " << bb.get_bl()
            << std::endl;
    }
    return 0;
}
