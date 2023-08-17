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
#include "indicators/min_max.hpp"
#include <gtest/gtest.h>

void test_min_max_1() {
    xtechnical::MinMax<double> os(4,0);
    std::vector<double> in_data = {
        1,2,3,4,5,
        1,2,3,4,5,
    };
    std::vector<double> max_data = {
        0,
        0,
        0,
        4,
        5,
        5,
        5,
        5,
        4,
        5,
    };
    std::vector<double> min_data = {
        0,
        0,
        0,
        1,
        2,
        1,
        1,
        1,
        1,
        2,
    };
    size_t index = 0;
    double tolerance = 0.00001;
    for (auto &item : in_data) {
        os.update(100, xtechnical::PriceType::IntraBar);
        os.update(item, xtechnical::PriceType::IntraBar);
        if ((index + 1) >= 4) {
            EXPECT_NEAR(os.get(xtechnical::enum_to_index(xtechnical::MinMaxLineType::MaxLine)), max_data[index], tolerance);
            EXPECT_NEAR(os.get(xtechnical::enum_to_index(xtechnical::MinMaxLineType::MinLine)), min_data[index], tolerance);
            EXPECT_NEAR(os.get(), max_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        os.update(item, xtechnical::PriceType::Close);
        if ((index + 1) >= 4) {
            EXPECT_NEAR(os.get(xtechnical::enum_to_index(xtechnical::MinMaxLineType::MaxLine)), max_data[index], tolerance);
            EXPECT_NEAR(os.get(xtechnical::enum_to_index(xtechnical::MinMaxLineType::MinLine)), min_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        ++index;
    }
}

void test_min_max_2() {
    xtechnical::MinMax<double> os(4,1);
    std::vector<double> in_data = {
        1,2,3,4,5,
        1,2,3,4,5,
    };
    std::vector<double> max_data = {
        0,
        0,
        0,
        0,
        4,
        5,
        5,
        5,
        5,
        4,
        5,
    };
    std::vector<double> min_data = {
        0,
        0,
        0,
        0,
        1,
        2,
        1,
        1,
        1,
        1,
        2,
    };
    size_t index = 0;
    double tolerance = 0.00001;
    for (auto &item : in_data) {
        os.update(100, xtechnical::PriceType::IntraBar);
        os.update(item, xtechnical::PriceType::IntraBar);
        if ((index + 1) >= 5) {
            EXPECT_NEAR(os.get(xtechnical::enum_to_index(xtechnical::MinMaxLineType::MaxLine)), max_data[index], tolerance);
            EXPECT_NEAR(os.get(xtechnical::enum_to_index(xtechnical::MinMaxLineType::MinLine)), min_data[index], tolerance);
            EXPECT_NEAR(os.get(), max_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        os.update(item, xtechnical::PriceType::Close);
        if ((index + 1) >= 5) {
            EXPECT_NEAR(os.get(xtechnical::enum_to_index(xtechnical::MinMaxLineType::MaxLine)), max_data[index], tolerance);
            EXPECT_NEAR(os.get(xtechnical::enum_to_index(xtechnical::MinMaxLineType::MinLine)), min_data[index], tolerance);
            EXPECT_TRUE(os.is_ready());
        } else {
            EXPECT_FALSE(os.is_ready());
        }
        ++index;
    }
}


TEST(MinMaxTest, TestGetAndUpdate) {
    test_min_max_1();
    test_min_max_2();
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
