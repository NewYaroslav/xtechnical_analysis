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
#include "indicators/delay_line.hpp"
#include <gtest/gtest.h>

void fill_rnd_values(std::vector<double>& vec, const double min_value, const double max_value) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<double> distribution(min_value, max_value);
    for (auto& number : vec) {
        number = distribution(generator);
    }
}

void test_delay_line_1() {
    xtechnical::DelayLine<double> delay_line(1);
    EXPECT_FALSE(delay_line.is_ready());
    delay_line.update(100.0, xtechnical::PriceType::IntraBar);
    EXPECT_FALSE(delay_line.is_ready());
    delay_line.update(100.0, xtechnical::PriceType::Close);
    EXPECT_FALSE(delay_line.is_ready());
    delay_line.update(200.0, xtechnical::PriceType::Close);
    EXPECT_TRUE(delay_line.is_ready());
    EXPECT_EQ(delay_line.get(), 100.0);
    delay_line.update(150.0, xtechnical::PriceType::IntraBar);
    EXPECT_EQ(delay_line.get(), 200.0);
    delay_line.update(150.0, xtechnical::PriceType::Close);
    EXPECT_EQ(delay_line.get(), 200.0);
}

void test_delay_line_2(const size_t period, const size_t num_data, const bool is_print = false) {
    xtechnical::DelayLine<double> delay_line(period);
    std::vector<double> data(num_data);
    fill_rnd_values(data, 0.1, 1.0);
    size_t index = 0;
    for (auto &item : data) {
        delay_line.update(item, xtechnical::PriceType::Close);
        if (index >= period && delay_line.is_ready()) {
            EXPECT_EQ(delay_line.get(), data[index - period]);
        }
        index++;
    }
}


TEST(DelayLineTest1, TestGetAndUpdate) {
    test_delay_line_1();
}

TEST(DelayLineTest2, TestGetAndUpdate) {
    test_delay_line_2(1,20);
    test_delay_line_2(8,20);
    test_delay_line_2(10,20);
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
