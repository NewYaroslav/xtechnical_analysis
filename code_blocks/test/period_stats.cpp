#include <iostream>
#include "xtechnical_indicators.hpp"
#include <random>
#include <array>

int main(int argc, char* argv[]) {
    std::cout << "Hello world!" << std::endl;

    xtechnical::PeriodStatsV1<double> period_stats(60);

    period_stats.add(10, 10000);
    std::cout <<  "e " << period_stats.empty() << " mv " << period_stats.get_max_value() << std::endl;
    period_stats.add(10, 10001);
    std::cout <<  "e " << period_stats.empty() << " mv " << period_stats.get_max_value() << std::endl;
    period_stats.add(10, 10002);
    std::cout <<  "e " << period_stats.empty() << " mv " << period_stats.get_max_value() << std::endl;
    period_stats.add(12, 10010);
    std::cout <<  "e " << period_stats.empty() << " mv " << period_stats.get_max_value() << std::endl;
    period_stats.add(15, 10020);
    std::cout <<  "e " << period_stats.empty() << " mv " << period_stats.get_max_value() << std::endl;
    period_stats.add(30, 10030);
    std::cout <<  "e " << period_stats.empty() << " mv " << period_stats.get_max_value() << std::endl;
    period_stats.add(10, 10050);
    std::cout <<  "e " << period_stats.empty() << " mv " << period_stats.get_max_value() << std::endl;
    period_stats.add(15, 10059);
    std::cout <<  "e " << period_stats.empty() << " mv " << period_stats.get_max_value() << std::endl;
    period_stats.add(12, 10089);
    std::cout <<  "e " << period_stats.empty() << " mv " << period_stats.get_max_value() << std::endl;
    period_stats.add(12, 10090);
    std::cout <<  "e " << period_stats.empty() << " mv " << period_stats.get_max_value() << std::endl;
    period_stats.add(11, 10060);
    std::cout <<  "e " << period_stats.empty() << " mv " << period_stats.get_max_value() << std::endl;
    period_stats.add(11, 10119);
    std::cout <<  "e " << period_stats.empty() << " mv " << period_stats.get_max_value() << std::endl;
    period_stats.add(5, 10120);
    std::cout <<  "e " << period_stats.empty() << " mv " << period_stats.get_max_value() << std::endl;
    period_stats.add(11, 10121);
    std::cout <<  "e " << period_stats.empty() << " mv " << period_stats.get_max_value() << std::endl;
    std::cout <<  "w " << period_stats.get_max_weight() << " c " << period_stats.get_center_mass() << std::endl;

    std::system("pause");
    return 0;
}
