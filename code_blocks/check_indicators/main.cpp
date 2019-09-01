#include <iostream>
#include "xtechnical_indicators.hpp"

using namespace std;

int main() {
    cout << "Hello world!" << endl;
    xtechnical_indicators::MW<double> iMW(30);

    for(int i = 1; i <= 50; ++i) {
        int temp = i % 4;
        iMW.update(temp);
        std::vector<double> rsi;
        std::vector<double> sma;
        std::vector<double> std_dev;
        iMW.get_rsi_data(rsi, 3, 15, 3);
        iMW.get_average_and_std_data(sma, std_dev, 10, 22, 3);

        std::cout << "step: " << i << " temp " << temp <<  std::endl;
        for(size_t i = 0; i < sma.size(); ++i) {
            std::cout << "rsi: " << rsi[i] << " sma: " << sma[i] << " std_dev: " << std_dev[i] << std::endl;
        }
    }
    return 0;
}
