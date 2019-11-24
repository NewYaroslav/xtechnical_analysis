#include <iostream>
#include "xtechnical_indicators.hpp"

using namespace std;

int main() {
    cout << "Hello world!" << endl;
    xtechnical_indicators::FreqHist<double>
        iFreqHist(100, xtechnical_dft::RECTANGULAR_WINDOW);
    const double MATH_PI = 3.14159265358979323846264338327950288;
    for(size_t i = 0; i < 1000; ++i) {
        double temp = std::cos(MATH_PI * 2 * (double)i / 10.0);

        std::vector<double> amplitude;
        std::vector<double> frequencies;
        iFreqHist.update(temp, amplitude, frequencies, 100);

        std::cout << "step: " << i << " temp " << temp << std::endl;
        for(size_t i = 0; i < frequencies.size(); ++i) {
            std::cout << "freq " << frequencies[i] << " " << amplitude[i] << std::endl;
        }
        std::cout << std::endl << std::endl;
    }

    return 0;
}
