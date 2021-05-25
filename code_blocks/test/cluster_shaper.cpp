#include <iostream>
#include "xtechnical_indicators.hpp"
#include "easy_plot.hpp"
#include <random>
#include <ctime>
#include <iterator>

void print_map(const std::map<int, int> &distribution) {
    for (auto it = distribution.begin(); it != distribution.end(); ++it) {
        std::cout << it->first << " " << it->second << std::endl;
    }
}

int main(int argc, char* argv[]) {
    ep::init(&argc, argv);
    std::cout << "Hello world!" << std::endl;
    // 0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0
    xtechnical::ClusterShaper cluster_shaper(60, 0.0001d);
    cluster_shaper.on_close_bar = [&](const xtechnical::ClusterShaper::Cluster &cluster) {
        auto normalized_cluster = cluster.get_normalized_array();
        auto no_normalized_cluster = cluster.get_array();

        if (false) {
            std::copy(normalized_cluster.begin(), normalized_cluster.end(), std::ostream_iterator<double>(std::cout, " "));
            std::cout << std::endl;
            std::copy(no_normalized_cluster.begin(), no_normalized_cluster.end(), std::ostream_iterator<double>(std::cout, " "));
            std::cout << std::endl;
            print_map(cluster.distribution);
        }

        std::cout << "o: " << cluster.open << " c: " << cluster.close << " m: " << cluster.get_center_mass() << std::endl;
        const double center = cluster.get_center_mass_norm();
        std::cout << "center: " << center << std::endl;
        if (center < 0.38) {
            std::copy(no_normalized_cluster.begin(), no_normalized_cluster.end(), std::ostream_iterator<double>(std::cout, " "));
            std::cout << std::endl;
            ep::WindowSpec wstyle;
            ep::plot<double>("center", wstyle, (int)1,
                normalized_cluster, ep::LineSpec(0,1,0));
            std::system("pause");
        }

        //auto triangular_distribution = xtechnical::ClusterShaper::get_triangular_distribution(normalized_cluster.size(), normalized_cluster.size()/2);

        auto triangular_distribution = xtechnical::ClusterShaper::get_triangular_distribution(normalized_cluster.size(), normalized_cluster.size() * 0.0);
        double similarity = xtechnical::ClusterShaper::get_cosine_similarity(normalized_cluster, triangular_distribution);
        double dist = xtechnical::ClusterShaper::get_euclidean_distance(normalized_cluster, triangular_distribution);
        std::cout << "similarity: " << similarity << std::endl;
        if (similarity > 0.55) {
            std::copy(no_normalized_cluster.begin(), no_normalized_cluster.end(), std::ostream_iterator<double>(std::cout, " "));
            std::cout << std::endl;
            ep::WindowSpec wstyle;
            ep::plot<double>("similarity", wstyle, (int)2,
                normalized_cluster, ep::LineSpec(0,1,0),
                triangular_distribution, ep::LineSpec(0,0,1));
            std::system("pause");
        }
        std::cout << "dist: " << dist << std::endl;
        if (dist < 0.02) {
            std::copy(no_normalized_cluster.begin(), no_normalized_cluster.end(), std::ostream_iterator<double>(std::cout, " "));
            std::cout << std::endl;
            ep::WindowSpec wstyle;
            ep::plot<double>("euclidean", wstyle, (int)2,
                normalized_cluster, ep::LineSpec(0,1,0),
                triangular_distribution, ep::LineSpec(0,0,1));
            std::system("pause");
        }

    };
    cluster_shaper.on_unformed_bar = [&](const xtechnical::ClusterShaper::Cluster &cluster) {
        auto normalized_cluster = cluster.get_normalized_array();
        //std::copy(normalized_cluster.begin(), normalized_cluster.end(), std::ostream_iterator<double>(std::cout, " "));
    };

    std::mt19937 gen(time(0));
    //std::normal_distribution<> urd(-0.5, 1.0);
    //std::lognormal_distribution<> urd(-0.5, 1.0);

    std::uniform_real_distribution<> urd(-0.5, 1.0);

    const std::size_t price_size = 5000000;
    std::vector<double> prices(price_size);
    double last_price = 10 + urd(gen);
    std::generate(prices.begin(), prices.begin() + price_size, [&]() -> double {
        last_price = last_price + 0.001d* urd(gen);
        return last_price;
    });

    for (size_t i = 0; i < price_size; ++i) {
        //std::cout << "price " << prices[i] << " " << i << std::endl;
        cluster_shaper.update(prices[i], i/2);
    }

    {
        auto triangular_distribution = xtechnical::ClusterShaper::get_triangular_distribution(10, 5);
        std::copy(triangular_distribution.begin(), triangular_distribution.end(), std::ostream_iterator<double>(std::cout, " "));
        std::cout << std::endl;
        std::system("pause");
    }
    {
        auto triangular_distribution = xtechnical::ClusterShaper::get_triangular_distribution(3, 1);
        std::copy(triangular_distribution.begin(), triangular_distribution.end(), std::ostream_iterator<double>(std::cout, " "));
        std::cout << std::endl;
        std::system("pause");
    }
    {
        auto triangular_distribution = xtechnical::ClusterShaper::get_triangular_distribution(5, 0);
        std::copy(triangular_distribution.begin(), triangular_distribution.end(), std::ostream_iterator<double>(std::cout, " "));
        std::cout << std::endl;
        std::system("pause");
    }
    {
        auto triangular_distribution = xtechnical::ClusterShaper::get_triangular_distribution(5, 4);
        std::copy(triangular_distribution.begin(), triangular_distribution.end(), std::ostream_iterator<double>(std::cout, " "));
        std::cout << std::endl;
        std::system("pause");
    }

    std::system("pause");
    return 0;
}
