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
#include "xta/indicators/usdx.hpp"
#include "xta/indicators/mfcsi.hpp"
#include <gtest/gtest.h>

// Тест режима буфера в режиме скользящего окна
void test_usdx_1() {
    const size_t timeframe = 60;
    xta::USDX<double> usdx(timeframe, false);

    EXPECT_FALSE(usdx.is_ready());
    for (size_t i = 0; i < 3; ++i) {
        const uint64_t timestamp = i * 30;
        const double price = 1.1;
        for (size_t s = 0; s < 6; ++s) {
            usdx.update(s, price, timestamp);
        }
        EXPECT_TRUE(usdx.calc());
        EXPECT_TRUE(usdx.is_ready());
        std::cout << usdx.get() << std::endl;
    }
}

void test_usdx_2() {
    const size_t timeframe = 60;
    xta::USDX<double> usdx(timeframe, true);

    EXPECT_FALSE(usdx.is_ready());
    for (size_t i = 0; i < 3; ++i) {
        const uint64_t timestamp = i * 30;
        const double price = 1.1;
        for (size_t s = 0; s < 6; ++s) {
            usdx.update(s, price, timestamp);
        }
        EXPECT_TRUE(usdx.is_ready());
        std::cout << usdx.get() << std::endl;
    }
}

void test_mfcsi_twi8_1() {
    const size_t bufsize = 5;
    const size_t timeframe = 60;
    xta::MFCSI_TWI8<double> mfcsi(bufsize, timeframe, false, false, true);

    EXPECT_FALSE(mfcsi.is_ready());
    for (size_t i = 0; i < bufsize*2; ++i) {
        const uint64_t timestamp = i * 30;
        for (size_t s = 0; s < 7; ++s) {
            const double price = 1.1 + i * 0.5 + (s % 3) * 2.0;
            mfcsi.update(s, price, timestamp);
        }
        if (i >= bufsize*2-2) {
            EXPECT_TRUE(mfcsi.calc());
            EXPECT_TRUE(mfcsi.is_ready());
            for (size_t s = 0; s < 8; ++s) {
                std::cout << mfcsi.get(s) << std::endl;
            }
        } else {
            EXPECT_FALSE(mfcsi.calc());
            EXPECT_FALSE(mfcsi.is_ready());
        }
    }
}

void test_mfcsi_abs8_1() {
    const size_t timeframe = 60;
    xta::MFCSI_ABS8<double> mfcsi(timeframe, true);

    EXPECT_FALSE(mfcsi.is_ready());
    for (size_t i = 0; i < 3; ++i) {
        const uint64_t timestamp = i * 30;
        for (size_t s = 0; s < 7; ++s) {
            const double price = 1.1 + i * 0.5 + (s % 3) * 2.0;
            mfcsi.update(s, price, timestamp);
        }
        EXPECT_TRUE(mfcsi.calc());
        EXPECT_TRUE(mfcsi.is_ready());
        for (size_t s = 0; s < 8; ++s) {
            std::cout << mfcsi.get(s) << std::endl;
        }
    }
}


TEST(USDX_Test, TestIndices) {
    test_usdx_1();
    test_usdx_2();
}

TEST(MFCSI_TWI8_Test, TestIndices) {
    test_mfcsi_twi8_1();
    test_mfcsi_abs8_1();
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
