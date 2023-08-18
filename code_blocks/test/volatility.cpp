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
#include "xta/indicators/atr.hpp"
#include "xta/indicators/std_dev.hpp"
#include "xta/indicators/percent_volatility.hpp"
#include <gtest/gtest.h>

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


TEST(StdDevTest, TestGetAndUpdate) {
    test_sd();
}

TEST(PercentVolatilityTest, TestGetAndUpdate) {
    test_percent_volatility();
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
