/*
* xtechnical_analysis - Technical analysis C++ library
*
* Copyright (c) 2018-2023 Elektro Yar. Email: git.electroyar@gmail.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include <iostream>
#include <random>
#include "xta/indicators/demarker.hpp"
#include "xta/indicators/mad.hpp"
#include "xta/indicators/std_dev.hpp"
#include "xta/indicators/zscore.hpp"
#include "xta/indicators/cci.hpp"
#include "xta/indicators/rsi.hpp"
#include "xta/indicators/momentum.hpp"
#include "xta/indicators/macd.hpp"
#include "xta/indicators/stochastic.hpp"
#include "xta/indicators/wpr.hpp"
#include "xta/indicators/percent_volatility.hpp"
#include "xta/indicators/price_change.hpp"
#include "xta/indicators/rpcd.hpp"
#include <gtest/gtest.h>

void fill_rnd_values(std::vector<double>& vec, const double min_value, const double max_value) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<double> distribution(min_value, max_value);
    for (auto& number : vec) {
        number = distribution(generator);
    }
}

void test_demarker(const size_t period, const bool is_print = false) {
    xta::DeMarker<double> os(period);
    std::vector<double> data(period * 2);
    fill_rnd_values(data, 0.1, 1.0);
    for (auto &item : data) {
        os.update(item, xta::PriceType::IntraBar);
        os.update(item, xta::PriceType::Close);
        double minRange = 0.0;
        double maxRange = 1.0;
        if (os.is_ready()) {
            if (is_print) std::cout << "os: " << os.get() << std::endl;
            ASSERT_TRUE(os.get() >= minRange && os.get() <= maxRange);
        }
    }
}


void test_mad() {
    xta::MAD<double> os(4);
    std::vector<double> data = {
        5,8,9,10,
        5,8,9,10,
        5,8,9,10,
        5,8,9,10,
        5,8,9,10,
    };
    size_t index = 0;
    for (auto &item : data) {
        ++index;
        os.update(item, xta::PriceType::IntraBar);
        os.update(item, xta::PriceType::IntraBar);
        if (index >= 4) {
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        os.update(item, xta::PriceType::Close);
        if (index >= 4) {
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
    }
    double tolerance = 0.001;
    double res = 1.5;
    EXPECT_NEAR(os.get(), res, tolerance);
    os.update(data[data.size()-4], xta::PriceType::Close);
    os.update(data[data.size()-3], xta::PriceType::Close);
    os.update(data[data.size()-2], xta::PriceType::Close);
    os.update(100, xta::PriceType::IntraBar);
    os.update(data[data.size()-1], xta::PriceType::IntraBar);
    EXPECT_NEAR(os.get(), res, tolerance);
}

void test_sd() {
    xta::StdDev<double> os(5);
    std::vector<double> in_data = {
        1,2,3,4,5,
        1,2,3,4,5,
    };
    std::vector<double> out_data = {
        0,
        0.707106781,
        1.0,
        1.290994449,
        1.58113883,

        1.58113883,
        1.58113883,
        1.58113883,
        1.58113883,
        1.58113883,
    };
    size_t index = 0;
    for (auto &item : in_data) {
        os.update(100, xta::PriceType::IntraBar);
        os.update(item, xta::PriceType::IntraBar);
        if (index > 0) {
            double tolerance = 0.001;
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
        }
        if ((index + 1) >= 5) {
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        os.update(item, xta::PriceType::Close);
        if ((index + 1) >= 5) {
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        if (index > 0) {
            double tolerance = 0.001;
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
        }
        ++index;
    }
    for (size_t i = 0; i < 10000; ++i) {
        for (auto &item : in_data) {
            os.update(item, xta::PriceType::Close);
            double tolerance = 0.00001;
            double cmp = 1.58113883;
            EXPECT_NEAR(os.get(), cmp, tolerance);
        }
    }
}

void test_zscore() {
    xta::Zscore<double> os(5);
    std::vector<double> in_data = {
        1,2,3,4,5,
        1,2,3,4,5,
    };
    std::vector<double> out_data = {
        0.0,
        0.707106781,
        1.0,
        1.161895004,
        1.264911064,
        -1.264911064,
        -0.632455532,
        0.0,
        0.632455532,
        1.264911064,
    };
    size_t index = 0;
    for (auto &item : in_data) {
        os.update(100, xta::PriceType::IntraBar);
        os.update(item, xta::PriceType::IntraBar);
        if (index > 0) {
            double tolerance = 0.001;
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
        }
        if ((index + 1) >= 5) {
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        os.update(item, xta::PriceType::Close);
        if ((index + 1) >= 5) {
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        if (index > 0) {
            double tolerance = 0.001;
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
        }
        ++index;
    }
    for (size_t i = 0; i < 10000; ++i) {
        for (auto &item : in_data) {
            os.update(item, xta::PriceType::Close);
        }
        double tolerance = 0.00001;
        double cmp = 1.264911064;
        EXPECT_NEAR(os.get(), cmp, tolerance);
    }
}

void test_cci() {
    xta::CCI<double> os(4);
    std::vector<double> data = {
        5,8,9,10,
        5,8,9,10,
        5,8,9,10,
        5,8,9,10,
        5,8,9,10,
    };
    size_t index = 0;
    for (auto &item : data) {
        ++index;
        os.update(item, xta::PriceType::IntraBar);
        os.update(item, xta::PriceType::IntraBar);
        if (index >= 4) {
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        os.update(item, xta::PriceType::Close);
        if (index >= 4) {
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
    }
    double tolerance = 0.001;
    double res = 88.88888889;
    EXPECT_NEAR(os.get(), res, tolerance);
    os.update(data[data.size()-4], xta::PriceType::Close);
    os.update(data[data.size()-3], xta::PriceType::Close);
    os.update(data[data.size()-2], xta::PriceType::Close);
    os.update(100, xta::PriceType::IntraBar);
    os.update(data[data.size()-1], xta::PriceType::IntraBar);
    EXPECT_NEAR(os.get(), res, tolerance);
}

void test_rsi() {
    xta::RSI<double> os(4);
    std::vector<double> data_up = {
        1,2,3,4,
        5,6,7,8,
    };
    std::vector<double> data_dn = {
        8,7,6,5,
        4,3,2,1,
    };
    std::vector<double> data_nn = {
        5,5,5,5,
        5,5,5,5,
    };

    {
        size_t index = 0;
        for (auto &item : data_up) {
            ++index;
            os.update(item, xta::PriceType::IntraBar);
            os.update(item, xta::PriceType::IntraBar);
            if (index >= 5) {
                EXPECT_TRUE(os.is_ready());
            } else {
                EXPECT_FALSE(os.is_ready());
            }
            os.update(item, xta::PriceType::Close);
            if (index >= 5) {
                EXPECT_TRUE(os.is_ready());
            } else {
                EXPECT_FALSE(os.is_ready());
            }
        }
        double tolerance = 0.00001;
        double res = 100.0;
        EXPECT_NEAR(os.get(), res, tolerance);
    }
    os.reset();
    {
        size_t index = 0;
        for (auto &item : data_dn) {
            ++index;
            os.update(item, xta::PriceType::IntraBar);
            os.update(item, xta::PriceType::IntraBar);
            if (index >= 5) {
                EXPECT_TRUE(os.is_ready());
            } else {
                EXPECT_FALSE(os.is_ready());
            }
            os.update(item, xta::PriceType::Close);
            if (index >= 5) {
                EXPECT_TRUE(os.is_ready());
            } else {
                EXPECT_FALSE(os.is_ready());
            }
        }
        double tolerance = 0.00001;
        double res = 0.0;
        EXPECT_NEAR(os.get(), res, tolerance);
    }
    os.reset();
    {
        size_t index = 0;
        for (auto &item : data_nn) {
            ++index;
            os.update(item, xta::PriceType::IntraBar);
            os.update(item, xta::PriceType::IntraBar);
            if (index >= 5) {
                EXPECT_TRUE(os.is_ready());
            } else {
                EXPECT_FALSE(os.is_ready());
            }
            os.update(item, xta::PriceType::Close);
            if (index >= 5) {
                EXPECT_TRUE(os.is_ready());
            } else {
                EXPECT_FALSE(os.is_ready());
            }
        }
        double tolerance = 0.00001;
        double res = 50.0;
        EXPECT_NEAR(os.get(), res, tolerance);
    }
}

void test_momentum() {
    xta::Momentum<double> os(2);
    std::vector<double> in_data = {
        1,2,3,4,5,
        1,2,3,4,5,
    };
    std::vector<double> out_data = {
        0,0,2,2,2,
        -3,-3,2,2,2,
    };
    size_t index = 0;
    double tolerance = 0.00001;
    for (auto &item : in_data) {
        os.update(100, xta::PriceType::IntraBar);
        os.update(item, xta::PriceType::IntraBar);
        if ((index + 1) > 2) {
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        os.update(item, xta::PriceType::Close);
        if ((index + 1) > 2) {
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        ++index;
    }
}

void test_macd_1() {
    xta::MACD<double,xta::SMA> os(3,5,0);
    std::vector<double> in_data = {
        1,2,3,4,5,
        1,2,3,4,5,
    };
    std::vector<double> macd_data = {
        0,0,0,0,1,
        0.333333333,-0.333333333,-1,0,1,
    };
    size_t index = 0;
    double tolerance = 0.00001;
    for (auto &item : in_data) {
        os.update(100, xta::PriceType::IntraBar);
        os.update(item, xta::PriceType::IntraBar);
        if ((index + 1) >= 5) {
            EXPECT_NEAR(os.get(), macd_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        os.update(item, xta::PriceType::Close);
        if ((index + 1) >= 5) {
            EXPECT_NEAR(os.get(), macd_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        ++index;
    }
}

void test_macd_2() {
    xta::MACD<double,xta::SMA> os(3,5,2);
    std::vector<double> in_data = {
        1,2,3,4,5,
        1,2,3,4,5,
    };
    std::vector<double> signal_data = {
        0,0,0,0,0,
        0.666666667,0.0,-0.666666667,-0.5,0.5,
    };
    size_t index = 0;
    double tolerance = 0.00001;
    for (auto &item : in_data) {
        os.update(100, xta::PriceType::IntraBar);
        os.update(item, xta::PriceType::IntraBar);
        if ((index + 1) >= 6) {
            EXPECT_NEAR(os.get(xta::MACDLineType::SignalLine), signal_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        os.update(item, xta::PriceType::Close);
        if ((index + 1) >= 6) {
            EXPECT_NEAR(os.get(xta::MACDLineType::SignalLine), signal_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        ++index;
    }
}

void test_stochastic_1() {
    xta::Stochastic<double> os(5);
    std::vector<double> in_data = {
        1,2,3,4,5,
        1,2,3,4,5,
    };
    std::vector<double> out_data = {
        0,0,0,0,100,
        0,25,50,75,100,
    };
    size_t index = 0;
    double tolerance = 0.00001;
    for (auto &item : in_data) {
        os.update(100, xta::PriceType::IntraBar);
        os.update(item, xta::PriceType::IntraBar);
        if ((index + 1) >= 5) {
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        os.update(item, xta::PriceType::Close);
        if ((index + 1) >= 5) {
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        ++index;
    }
}

void test_stochastic_2() {
    xta::Stochastic<double> os(5,2);
    std::vector<double> in_data = {
        1,2,3,4,5,
        1,2,3,4,5,
    };
    std::vector<double> out_data = {
        0,0,0,0,0,
        50.0,
        12.5,
        37.5,
        62.5,
        87.5,
    };
    size_t index = 0;
    double tolerance = 0.00001;
    for (auto &item : in_data) {
        os.update(100, xta::PriceType::IntraBar);
        os.update(item, xta::PriceType::IntraBar);
        if ((index + 1) >= 6) {
            EXPECT_NEAR(os.get(xta::StochasticLineType::DLine), out_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        os.update(item, xta::PriceType::Close);
        if ((index + 1) >= 6) {
            EXPECT_NEAR(os.get(xta::StochasticLineType::DLine), out_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        ++index;
    }
}

void test_wpr() {
    xta::WPR<double> os(5);
    std::vector<double> in_data = {
        1,2,3,4,5,
        1,2,3,4,5,
    };
    std::vector<double> out_data = {
        0,0,0,0,0,
        -100,-75,-50,-25,0,
    };
    size_t index = 0;
    double tolerance = 0.00001;
    for (auto &item : in_data) {
        os.update(100, xta::PriceType::IntraBar);
        os.update(item, xta::PriceType::IntraBar);
        if ((index + 1) >= 5) {
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        os.update(item, xta::PriceType::Close);
        if ((index + 1) >= 5) {
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        ++index;
    }
}

void test_percent_volatility() {
    xta::PercentVolatility<double> os(3);
    std::vector<double> in_data = {
        1,2,3,4,5,
        1,2,3,4,5,
    };
    std::vector<double> out_data = {
        0,0,
        200,
        100,
        66.66666667,
        400,
        400,
        200,
        100,
        66.66666667,
    };
    size_t index = 0;
    double tolerance = 0.00001;
    for (auto &item : in_data) {
        os.update(100, xta::PriceType::IntraBar);
        os.update(item, xta::PriceType::IntraBar);
        if ((index + 1) >= 3) {
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        os.update(item, xta::PriceType::Close);
        if ((index + 1) >= 3) {
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        ++index;
    }
}

void test_price_change() {
    xta::PriceChange<double> os(2);
    std::vector<double> in_data = {
        1,2,3,4,5,
        1,2,3,4,5,
    };
    std::vector<double> out_data = {
        0,0,
        200,
        100,
        66.66666667,
        -75,
        -60,
        200,
        100,
        66.66666667,
    };
    size_t index = 0;
    double tolerance = 0.00001;
    for (auto &item : in_data) {
        os.update(100, xta::PriceType::IntraBar);
        os.update(item, xta::PriceType::IntraBar);
        if ((index + 1) >= 3) {
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        os.update(item, xta::PriceType::Close);
        if ((index + 1) >= 3) {
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        ++index;
    }
}

void test_rpcd_1() {
    xta::RPCD<double> os(1);
    std::vector<double> in_data = {
        1,2,3,4,5,
        1,2,3,4,5,
    };
    std::vector<double> out_data = {
        0,
        0,
        100,
        100,
        100,
        -400,
        25,
        100,
        100,
        100,
    };
    size_t index = 0;
    double tolerance = 0.00001;
    for (auto &item : in_data) {
        os.update(100, xta::PriceType::IntraBar);
        os.update(item, xta::PriceType::IntraBar);
        if ((index + 1) >= 3) {
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        os.update(item, xta::PriceType::Close);
        if ((index + 1) >= 3) {
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        ++index;
    }
}

void test_rpcd_2() {
    xta::RPCD<double> os(2);
    std::vector<double> in_data = {
        1,2,3,4,5,
        1,2,3,4,5,
    };
    std::vector<double> out_data = {
        0,
        0,
        0,
        0,
        100,
        -150,
        -150,
        66.66666667,
        66.66666667,
        100,
        -150,
        -150,
        66.66666667,
        66.66666667,
        100,
    };
    size_t index = 0;
    double tolerance = 0.00001;
    for (auto &item : in_data) {
        os.update(100, xta::PriceType::IntraBar);
        os.update(item, xta::PriceType::IntraBar);
        if ((index + 1) >= 5) {
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        os.update(item, xta::PriceType::Close);
        if ((index + 1) >= 5) {
            EXPECT_NEAR(os.get(), out_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        ++index;
    }
}

void test_rpcd_3() {
    xta::RPCD<double> os(2,3);
    std::vector<double> in_data = {
        1,2,3,4,5,
        1,2,3,4,5,
    };
    std::vector<double> out_data_1 = {
        0,
        0,
        0,
        0,
        0,
        0,
        -66.66666667,
        -77.77777778,
        -5.555555556,
        77.77777778,
    };
    std::vector<double> out_data_2 = {
        0,
        0,
        0,
        0,
        0,
        0,
        133.3333333,
        122.2222222,
        94.44444444,
        77.77777778,
    };
    size_t index = 0;
    double tolerance = 0.00001;
    for (auto &item : in_data) {
        os.update(100, xta::PriceType::IntraBar);
        os.update(item, xta::PriceType::IntraBar);
        if ((index + 1) >= 7) {
            EXPECT_NEAR(os.get(xta::RPCDLineType::SmoothedLine), out_data_1[index], tolerance);
            EXPECT_NEAR(os.get(xta::RPCDLineType::VolatilityLine), out_data_2[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        os.update(item, xta::PriceType::Close);
        if ((index + 1) >= 7) {
            EXPECT_NEAR(os.get(xta::RPCDLineType::SmoothedLine), out_data_1[index], tolerance);
            EXPECT_NEAR(os.get(xta::RPCDLineType::VolatilityLine), out_data_2[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        ++index;
    }
}

TEST(DeMarkerTest, TestGetAndUpdate) {
    test_demarker(10, true);
    test_demarker(16);
    test_demarker(1024);
}

TEST(MADTest, TestGetAndUpdate) {
    test_mad();
}

TEST(StdDevTest, TestGetAndUpdate) {
    test_sd();
}

TEST(ZscoreTest, TestGetAndUpdate) {
    test_zscore();
}

TEST(CCITest, TestGetAndUpdate) {
    test_cci();
}

TEST(RSITest, TestGetAndUpdate) {
    test_rsi();
}

TEST(MomentumTest, TestGetAndUpdate) {
    test_momentum();
}

TEST(MACDTest, TestGetAndUpdate) {
    test_macd_1();
    test_macd_2();
}

TEST(StochasticTest, TestGetAndUpdate) {
    test_stochastic_1();
    test_stochastic_2();
}

TEST(WPRTest, TestGetAndUpdate) {
    test_wpr();
}

TEST(PercentVolatilityTest, TestGetAndUpdate) {
    test_percent_volatility();
}

TEST(PriceChangeTest, TestGetAndUpdate) {
    test_price_change();
}

TEST(RPCDTest, TestGetAndUpdate) {
    test_rpcd_1();
    test_rpcd_2();
    test_rpcd_3();
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
