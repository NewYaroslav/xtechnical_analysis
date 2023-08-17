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
#include "indicators/circular_buffer.hpp"
#include <gtest/gtest.h>

void fill_rnd_values(std::vector<double>& vec, const double min_value, const double max_value) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<double> distribution(min_value, max_value);
    for (auto& number : vec) {
        number = distribution(generator);
    }
}

size_t get_size(const size_t period, const size_t num_data, const xtechnical::PriceType type = xtechnical::PriceType::Close) {
    xtechnical::CircularBuffer<double> buffer(period);
    std::vector<double> data(num_data);
    fill_rnd_values(data, 0.1, 1.0);
    for (auto &item : data) {
        buffer.update(item, type);
    }
    return buffer.size();
}

bool check_empty(const size_t period, const size_t num_data, const xtechnical::PriceType type = xtechnical::PriceType::Close) {
    xtechnical::CircularBuffer<double> buffer(period);
    std::vector<double> data(num_data);
    fill_rnd_values(data, 0.1, 1.0);
    for (auto &item : data) {
        buffer.update(item, type);
    }
    return buffer.empty();
}

bool check_full(const size_t period, const size_t num_data, const xtechnical::PriceType type = xtechnical::PriceType::Close) {
    xtechnical::CircularBuffer<double> buffer(period);
    std::vector<double> data(num_data);
    fill_rnd_values(data, 0.1, 1.0);
    for (auto &item : data) {
        buffer.update(item, type);
    }
    return buffer.full();
}

void test_buffer(const size_t period, const size_t num_data, const bool is_print = false) {
    xtechnical::CircularBuffer<double> buffer(period);

    EXPECT_EQ(buffer.size(), 0);
    EXPECT_TRUE(buffer.empty());

    std::vector<double> data(num_data);
    fill_rnd_values(data, 0.1, 1.0);
    size_t index = 0;
    for (auto &item : data) {
        buffer.update(item, xtechnical::PriceType::IntraBar);
        EXPECT_EQ(buffer.back(), item);

        if (index < period) {
            std::vector<double> data_intra = buffer.to<std::vector>();
            for (size_t i = 0; i < buffer.size(); ++i) {
                EXPECT_EQ(data[i], data_intra[i]);
                EXPECT_EQ(data[i], buffer[i]);
                EXPECT_EQ(data[i], buffer.at(i));
            }
        }

        buffer.update(item, xtechnical::PriceType::Close);
        EXPECT_EQ(buffer.back(), item);

        if (index < period) {
            std::vector<double> data_close = buffer.to<std::vector>();
            for (size_t i = 0; i < buffer.size(); ++i) {
                EXPECT_EQ(data[i], data_close[i]);
                EXPECT_EQ(data[i], buffer[i]);
                EXPECT_EQ(data[i], buffer.at(i));
            }
        }

        index++;
        if (index >= period) {
            EXPECT_EQ(buffer.front(), data[index - period]);
        } else {
            EXPECT_EQ(buffer.front(), data[0]);
        }
    }

    const size_t buffer_size = std::min(num_data, period);

    EXPECT_EQ(buffer.size(), buffer_size);
    EXPECT_FALSE(buffer.empty());

    const size_t start_index = num_data > period ? num_data - period : 0;

    std::vector<double> temp = buffer.to<std::vector>();
    if (is_print) {
        for (size_t i = 0; i < buffer_size; ++i) {
            std::cout << "at(" << i <<") = " << buffer.at(i) << "; value = " << data[i + start_index] << std::endl;
        }
        for (size_t i = 0; i < buffer_size; ++i) {
            std::cout << "vector[" << i <<"] = " << temp[i] << "; value = " << data[i + start_index] << std::endl;
        }
    }

    double sum = 0;
    for (size_t i = 0; i < buffer_size; ++i) {
        EXPECT_EQ(buffer.at(i), data[i + start_index]);
        EXPECT_EQ(buffer[i], data[i + start_index]);
        EXPECT_EQ(temp[i], data[i + start_index]);
        sum += temp[i];
    }
    EXPECT_EQ(buffer.sum(), sum);

    EXPECT_EQ(buffer.full(), (num_data >= period));
}

// Проверка метода size
TEST(AddTest, AddsNumbersCorrectly) {
    EXPECT_EQ(0, get_size(2, 0));
    EXPECT_EQ(1, get_size(1, 1));
    EXPECT_EQ(1, get_size(1, 2));
    EXPECT_EQ(2, get_size(2, 2));
    EXPECT_EQ(2, get_size(10, 2));
    EXPECT_EQ(10, get_size(10, 10));
    EXPECT_EQ(10, get_size(10, 11));
    EXPECT_EQ(100, get_size(100, 101));
    EXPECT_EQ(1000, get_size(1000, 1001));
    EXPECT_EQ(10000, get_size(10000, 10000));
}

TEST(AddTest, ReturnsTrueForEvenNumbers) {
    // Проверка метода empty
    EXPECT_TRUE(check_empty(2, 0));
    EXPECT_FALSE(check_empty(2, 1));
    // Проверка метода full
    EXPECT_TRUE(check_full(2, 2));
    EXPECT_FALSE(check_full(2, 1));
    EXPECT_TRUE(check_full(20, 20));
    EXPECT_FALSE(check_full(20, 19));
    EXPECT_TRUE(check_full(200, 200));
    EXPECT_FALSE(check_full(200, 199));
}

TEST(CircularBufferTest1, TestAtAndUpdate) {
    test_buffer(5,5);
    test_buffer(8,5, true);
    test_buffer(8,8);
    test_buffer(8,16);
    test_buffer(16,15);
    test_buffer(16,32);
    test_buffer(256,256*1);
    test_buffer(256,256*2);
    test_buffer(256,256*4);
    test_buffer(1024,1023);
    test_buffer(1024,1024*4);

}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
