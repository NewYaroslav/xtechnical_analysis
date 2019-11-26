/*
* xtechnical_analysis - Technical analysis C++ library
*
* Copyright (c) 2018 Elektro Yar. Email: git.electroyar@gmail.com
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
#ifndef XTECHNICAL_DFT_HPP_INCLUDED
#define XTECHNICAL_DFT_HPP_INCLUDED

#include "xtechnical_common.hpp"
#include <vector>
#include <cmath>

namespace xtechnical_dft {

    /// Варианты окна для БПФ
    enum {
        RECTANGULAR_WINDOW = 0,     /**< Прямоугольное окно.
            Лучшее разрешение по частоте, худшее разрешение по амплитуде.
            Это по сути то же самое, что и без окна.
            Для чего используется:
            1) Переходные процессы или всплески, где уровень сигнала
            до и после события почти равны.
            2) Синусоиды равной амплитуды с частотами,
            которые являются очень близкими.
            3) Широкополосный случайный шум
            с относительно медленным изменением спектра.
         */
        BLACKMAN_HARRIS_WINDOW = 1, /**< Окно Блэкман-Харриса.
            Лучшее разрешение по амплитуде, худшее разрешение по частоте.
            Для чего используется:
            1) Для наблюдения высших гармоник сигнал одной частоты.
        */
        HAMMING_WINDOW = 2, /**< Окно Хемминга.
            Лучшее разрешение по частоте, разрешение по амплитуде хуже,
            чем у прямоугольного.
            Чуть лучше разрешение по частоте, чем с окном Хеннинга.
            Для чего используется:
            1) Синусоидальные, периодические сигналы
            и узкополосный случайный шум.
            2) Переходные процессы или всплески,
            где уровень сигнала до и после события, существенно различаются.
         */
        HANN_WINDOW = 3, /**< Окно Ханна
            Лучшее разрешение по частоте.
            Разрешение по амплитуде хуже, чем у прямоугольного окна.
            Для чего используется:
            1) Синусоидальные, периодические сигналы
            и узкополосный случайный шум.
            2) Переходные процессы или всплески,
            где уровень сигнала до и после события, существенно различаются.
        */
    };

    /** \brief ДФТ для действительных образцов.
     */
    template<class T>
    class DftReal {
    private:
        std::vector<T> sine_table;
        std::vector<T> cosine_table;
        std::vector<T> window_table;
        size_t table_period = 0;
        size_t window_type = RECTANGULAR_WINDOW;

        void generate_table(const size_t period) {
            if(period == table_period) return;
            cosine_table.resize(period);
            sine_table.resize(period);
            const T MATH_PI = 3.14159265358979323846264338327950288;
            const T MATH_PI_X2 = 2.0 * MATH_PI;
            for(size_t j = 0; j < period; j++) {
                T temp = MATH_PI_X2 * (T)j / (T)period;
                cosine_table[j] = std::cos(temp);
                sine_table[j] = -std::sin(temp);
            }
            table_period = period;
        }

        void generate_blackman_harris_window() {
            const T a0 = 0.35875;
            const T a1 = 0.48829;
            const T a2 = 0.14128;
            const T a3 = 0.01168;
            const T MATH_PI = 3.14159265358979323846264338327950288;
            const T coeff1 = 2.0 * MATH_PI /
                (T)(table_period - 1);
            const T coeff3 = 6.0 * MATH_PI /
                (T)(table_period - 1);
                const T coeff2 = 4.0 * MATH_PI /
                (T)(table_period - 1);
            window_table.resize(table_period);
            for(size_t i = 0; i < table_period; ++i) {
                window_table[i] = a0
                    - a1 * std::cos(coeff1 * (T)i)
                    + a2 * std::cos(coeff2 * (T)i)
                    - a3 * std::cos(coeff3 * (T)i);
            }
        }

        void generate_hamming_window() {
            const T MATH_PI = 3.14159265358979323846264338327950288;
            const T coeff = 2.0 * MATH_PI /
                (T)(table_period - 1);
            window_table.resize(table_period);
            for(size_t i = 0; i < table_period; ++i) {
                window_table[i] = 0.54 - 0.46 * std::cos(coeff * (T)i);
            }
        }

        void generate_hanning_window() {
            const T MATH_PI = 3.14159265358979323846264338327950288;
            const T coeff = 2.0 * MATH_PI /
                (T)(table_period - 1);
            window_table.resize(table_period);
            for(size_t i = 0; i < table_period; ++i) {
                window_table[i] = 0.5 - 0.5 * std::cos(coeff * (T)i);
            }
        }

        void calc_window(const size_t use_window_type) {
            if(window_table.size() == table_period &&
                window_type == use_window_type) return;
            switch(use_window_type) {
                case RECTANGULAR_WINDOW:
                    break;
                case BLACKMAN_HARRIS_WINDOW:
                    generate_blackman_harris_window();
                    break;
                case HAMMING_WINDOW:
                    generate_hamming_window();
                    break;
                case HANN_WINDOW:
                    generate_hanning_window();
                    break;
            };
            window_type = use_window_type;
        }
    public:
        DftReal() {};

        DftReal(const size_t period, const size_t use_window_type) {
            generate_table(period);
            calc_window(use_window_type);
        }

        template<class FLOAT_TYPE>
        int calc_dft(
                const std::vector<FLOAT_TYPE> &input_real,
                std::vector<FLOAT_TYPE> &output_real,
                std::vector<FLOAT_TYPE> &output_imag) {
            if(input_real.size() != table_period) {
                generate_table(input_real.size());
                calc_window(window_type);
            }

            if(table_period % 2 != 0 || table_period < 4)
                return xtechnical_common::INVALID_PARAMETER;

            const size_t period_div2 = table_period/2;
            //const size_t period_div2_inc = period_div2 + 1;

            if(output_real.size() != table_period) {
                output_real.resize(table_period);
                output_imag.resize(table_period);
            }

            if(window_type == RECTANGULAR_WINDOW) {
                for(size_t j = 0; j <= period_div2; ++j) {
                    output_real[j] = 0.0;
                    output_imag[j] = 0.0;
                    for(size_t k = 0; k < table_period; ++k) {
                        output_real[j] +=
                            input_real[k] *
                            cosine_table[(j * k) % table_period];
                        output_imag[j] +=
                            input_real[k] *
                            sine_table[(j * k) % table_period];
                    }
                    output_real[j] /= (FLOAT_TYPE)table_period;
                    output_imag[j] /= (FLOAT_TYPE)table_period;
                }
            } else {
                std::vector<FLOAT_TYPE> temp_input_real = input_real;
                for(size_t k = 0; k < table_period; ++k) {
                    temp_input_real[k] *= window_table[k];
                }
                for(size_t j = 0; j <= period_div2; ++j) {
                    output_real[j] = 0.0;
                    output_imag[j] = 0.0;
                    for(size_t k = 0; k < table_period; ++k) {
                        output_real[j] +=
                            temp_input_real[k] *
                            cosine_table[(j * k) % table_period];
                        output_imag[j] +=
                            temp_input_real[k] *
                            sine_table[(j * k) % table_period];
                    }
                    output_real[j] /= (FLOAT_TYPE)table_period;
                    output_imag[j] /= (FLOAT_TYPE)table_period;
                }
            }

            for(size_t j = 1; j < period_div2; ++j) {
                output_real[table_period - j] = output_real[j];
                output_imag[table_period - j] = -output_imag[j];
            }
            return xtechnical_common::OK;
        }

        template<class FLOAT_TYPE>
        int update(
                const std::vector<FLOAT_TYPE> &input_real,
                std::vector<FLOAT_TYPE> &amplitude,
                std::vector<FLOAT_TYPE> &frequencies,
                const FLOAT_TYPE sample_rate = 0) {
            std::vector<FLOAT_TYPE> output_real;
            std::vector<FLOAT_TYPE> output_imag;
            int err = calc_dft(
                input_real,
                output_real,
                output_imag);
            if(err != xtechnical_common::OK) return err;
            const size_t period_div2 = table_period / 2;

            amplitude.resize(period_div2 + 1);
            frequencies.resize(period_div2 + 1);
            for(size_t i = 0; i < period_div2 + 1; ++i) {
                amplitude[i] = 2* std::sqrt(
                    output_real[i] * output_real[i] +
                    output_imag[i] * output_imag[i]);
                if(sample_rate != 0) {
                    frequencies[i] =
                        (FLOAT_TYPE)i*((FLOAT_TYPE)sample_rate/
                        (FLOAT_TYPE)table_period);
                } else {
                    frequencies[i] = i;
                }
            }
            return xtechnical_common::OK;
        }
    };
}
#endif // XTECHNICAL_DFT_HPP_INCLUDED
