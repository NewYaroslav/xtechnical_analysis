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
#include "indicators/quote_sync.hpp"
#include <gtest/gtest.h>

// Тест режима буфера в режиме скользящего окна
void test_quote_sync_1() {
    std::cout << "test_quote_sync_1" << std::endl;
    const size_t symbols = 3;
    const size_t timeframe = 60;
    xtechnical::QuoteSync<double> quote_sync(symbols, timeframe, false);

    quote_sync.on_update = [](
            const size_t index,
            const double &value,
            const uint64_t open_date,
            const uint64_t delay_ms,
            const xtechnical::PriceType type,
            const bool is_update){
        std::string update_text;
        if (type == xtechnical::PriceType::Close) update_text = "close";
        else update_text = "intrabar";
        std::cout
            << "s: " << index
            << " v: " << value
            << " od: " << open_date
            << " del: " << delay_ms
            << " type: " << update_text
            << " u: " << is_update << std::endl;
    };

    double price = 99.0;
    uint64_t time_ms = 60* 1000;

    // делаем плохой старт
    for (size_t s = 0; s < 1; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }
    quote_sync.calc();

    time_ms += 60* 1000;
    price += 1.0;
    for (size_t s = 0; s < 1; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }
    quote_sync.calc();


    time_ms += 60* 1000;
    price += 1.0;
    for (size_t s = 0; s < 1; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }
    quote_sync.calc();

    // первый бар, начало
    time_ms += 60* 1000;
    price += 1.0;
    for (size_t s = 0; s < symbols; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }
    quote_sync.calc();

    // первый бар, продолжение
    time_ms += 1000;
    price += 1.0;
    for (size_t s = 0; s < symbols; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }
    quote_sync.calc();

    // первый бар, один новый тик
    time_ms += 1000;
    price += 1.0;
    for (size_t s = 0; s < 1; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }
    quote_sync.calc();

    // первый бар, завершение, 1 шаг
    time_ms += 60* 1000;
    price += 1.0;
    for (size_t s = 0; s < 1; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }
    quote_sync.calc();

    // первый бар, завершение, 2 шаг
    time_ms += 1000;
    price += 1.0;
    for (size_t s = 1; s < symbols; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }
    quote_sync.calc();

    // второй бар
    time_ms += 1000;
    price += 1.0;
    for (size_t s = 0; s < symbols; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }
    quote_sync.calc();

    // заполняем только один бар
    time_ms += 60* 1000;
    price += 1.0;
    for (size_t s = 0; s < 1; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }
    quote_sync.calc();

    time_ms += 60* 1000;
    price += 1.0;
    for (size_t s = 0; s < 1; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }
    quote_sync.calc();

    time_ms += 60* 1000;
    price += 1.0;
    for (size_t s = 0; s < 1; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }
    quote_sync.calc();

    // обновляем все бары
    time_ms += 1000;
    price += 1.0;
    for (size_t s = 0; s < symbols; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }
    quote_sync.calc();
}

void test_quote_sync_2() {
    std::cout << "test_quote_sync_2" << std::endl;
    const size_t symbols = 3;
    const size_t timeframe = 60;
    xtechnical::QuoteSync<double> quote_sync(symbols, timeframe, true);

    quote_sync.on_update = [](
            const size_t index,
            const double &value,
            const uint64_t open_date,
            const uint64_t delay_ms,
            const xtechnical::PriceType type,
            const bool is_update){
        std::string update_text;
        if (type == xtechnical::PriceType::Close) update_text = "close";
        else update_text = "intrabar";
        std::cout
            << "s: " << index
            << " v: " << value
            << " od: " << open_date
            << " del: " << delay_ms
            << " type: " << update_text
            << " u: " << is_update << std::endl;
    };

    double price = 99.0;
    uint64_t time_ms = 60* 1000;

    // делаем плохой старт
    for (size_t s = 0; s < 1; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }

    time_ms += 60* 1000;
    price += 1.0;
    for (size_t s = 0; s < 1; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }

    time_ms += 60* 1000;
    price += 1.0;
    for (size_t s = 0; s < 1; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }

    // первый бар, начало
    time_ms += 60* 1000;
    price += 1.0;
    for (size_t s = 0; s < symbols; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }

    // первый бар, продолжение
    time_ms += 1000;
    price += 1.0;
    for (size_t s = 0; s < symbols; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }

    // первый бар, один новый тик
    time_ms += 1000;
    price += 1.0;
    for (size_t s = 0; s < 1; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }

    // первый бар, завершение, 1 шаг
    time_ms += 60* 1000;
    price += 1.0;
    for (size_t s = 0; s < 1; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }

    // первый бар, завершение, 2 шаг
    time_ms += 1000;
    price += 1.0;
    for (size_t s = 1; s < symbols; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }

    // второй бар
    time_ms += 1000;
    price += 1.0;
    for (size_t s = 0; s < symbols; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }

    // заполняем только один бар
    time_ms += 60* 1000;
    price += 1.0;
    for (size_t s = 0; s < 1; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }

    time_ms += 60* 1000;
    price += 1.0;
    for (size_t s = 0; s < 1; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }

    time_ms += 60* 1000;
    price += 1.0;
    for (size_t s = 0; s < 1; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }

    // обновляем все бары
    time_ms += 1000;
    price += 1.0;
    for (size_t s = 0; s < symbols; ++s) {
        std::cout << "add s: " << s << " time_ms: " << time_ms << std::endl;
        quote_sync.update(s, price, time_ms);
    }
}


TEST(Test1, TestQuoteSync) {
    test_quote_sync_1();
    test_quote_sync_2();
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
