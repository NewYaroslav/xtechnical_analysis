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
#include "indicators/date_buffer.hpp"
#include <gtest/gtest.h>

void fill_rnd_values(std::vector<double>& vec, const double min_value, const double max_value) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<double> distribution(min_value, max_value);
    for (auto& number : vec) {
        number = distribution(generator);
    }
}

// Тест режима буфера в режиме скользящего окна
void test_date_buffer_window_mode_1() {
    const size_t buffer_size = 5;
    const size_t timeframe = 60;
    const size_t symbols = 5;
    xtechnical::DateBuffer<double> date_buffer(buffer_size, timeframe, false, symbols);
    // Цикл по дате
    std::vector<double> prices;
    for (size_t i = 0; i < 20; ++i) {
        const uint64_t timestamp = i * 30;
        const double price = 100.0 + (double)i * 10.0;

        if (i % 2 == 0) {
            prices.push_back(price);
        } else {
            prices.back() = price;
        }

        // Цикл по всем валютных парам
        for (size_t s = 0; s < symbols; ++s) {
            date_buffer.update(s, price, timestamp);
        }
        if (timestamp >= (buffer_size * 60 - 60)) {
            EXPECT_TRUE(date_buffer.is_ready());
        } else {
            EXPECT_FALSE(date_buffer.is_ready());
        }
        if (!date_buffer.is_ready()) continue;

        const double tolerance = 0.00001;
        for (size_t s = 0; s < symbols; ++s) {
            std::vector<double> buf = date_buffer.to_vector(s);
            for (size_t j = 0; j < buffer_size; ++j) {
                const size_t index = prices.size() - buffer_size + j;
                EXPECT_NEAR(prices[index], buf[j], tolerance);
            }
        }
    }
}

void test_date_buffer_window_mode_2() {
    const size_t buffer_size = 1;
    const size_t timeframe = 60;
    const size_t symbols = 3;
    xtechnical::DateBuffer<double> date_buffer(buffer_size, timeframe, false, symbols);
    // Цикл по дате
    std::vector<double> prices;
    for (size_t i = 0; i < 5; ++i) {
        const uint64_t timestamp = i * 30;
        const double price = 100.0 + (double)i * 10.0;

        if (i % 2 == 0) {
            prices.push_back(price);
        } else {
            prices.back() = price;
        }

        // Цикл по всем валютных парам
        for (size_t s = 0; s < symbols; ++s) {
            date_buffer.update(s, price, timestamp);
            if (i % 2 == 0 && s == (symbols - 1)) {
                EXPECT_TRUE(date_buffer.is_ready());
            } else {
                if (i % 2 == 0) EXPECT_FALSE(date_buffer.is_ready());
            }
        }
        if (timestamp >= (buffer_size * 60 - 60)) {
            EXPECT_TRUE(date_buffer.is_ready());
        } else {
            EXPECT_FALSE(date_buffer.is_ready());
        }
        if (!date_buffer.is_ready()) continue;

        const double tolerance = 0.00001;
        for (size_t s = 0; s < symbols; ++s) {
            EXPECT_NEAR(date_buffer.get(s), price, tolerance);
            std::vector<double> buf = date_buffer.to_vector(s);
            for (size_t j = 0; j < buffer_size; ++j) {
                const size_t index = prices.size() - buffer_size + j;
                EXPECT_NEAR(prices[index], buf[j], tolerance);
            }
        }
    }
}

void test_date_buffer_period_mode_1() {
    const size_t buffer_size = 5;
    const size_t timeframe = 60;
    const size_t symbols = 5;
    xtechnical::DateBuffer<double> date_buffer(buffer_size, timeframe, true, symbols);
    // Цикл по дате
    std::vector<double> prices;
    for (size_t i = 0; i <= 20; ++i) {
        const uint64_t timestamp = i * 30;
        const double price = 100.0 + (double)i * 0.5;

        if ((i % 2) == 0) {
            if (timestamp % (buffer_size * timeframe) == 0) {
                // в начале каждого периода буфер обнуляется
                prices.clear();
            }
            prices.push_back(price);
        } else {
            prices.back() = price;
        }

        // Цикл по всем валютных парам
        for (size_t s = 0; s < symbols; ++s) {
            date_buffer.update(s, price, timestamp);
            if (s < (symbols-1) && (i % 2) == 0) EXPECT_FALSE(date_buffer.is_ready());
            else EXPECT_TRUE(date_buffer.is_ready());
        }
        EXPECT_TRUE(date_buffer.is_ready());
        if (!date_buffer.is_ready()) continue;

        const double tolerance = 0.00001;
        for (size_t s = 0; s < symbols; ++s) {
            std::vector<double> buf = date_buffer.to_vector(s);
            for (size_t j = 0; j < buf.size(); ++j) {
                const size_t index = prices.size() - buf.size() + j;
                EXPECT_NEAR(prices[index], buf[j], tolerance);
            }
        }
    }
}

TEST(DateBufferWindowModeTest, TestDateBuffer) {
    test_date_buffer_window_mode_1();
    test_date_buffer_window_mode_2();
}

TEST(DateBufferPeriodModeTest, TestDateBuffer) {
    test_date_buffer_period_mode_1();
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
