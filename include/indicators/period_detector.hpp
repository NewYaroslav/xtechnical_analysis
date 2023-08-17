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
#ifndef XTECHNICAL_PERIOD_DETECTOR_HPP_INCLUDED
#define XTECHNICAL_PERIOD_DETECTOR_HPP_INCLUDED

#include "delay_line.hpp"
#include "circular_buffer.hpp"

namespace xtechnical {

    /** \brief Period Detector
     * https://scask.ru/c_book_r_cos.php?id=233
     */
    template <class T = double, template <class...> class LF_TYPE = xtechnical::LowPassFilter>
    class PeriodDetector final : public BaseIndicator<T> {
    private:
        CircularBuffer<T>   m_buffer;
        LF_TYPE<T>          m_filter;
        DelayLine<T>        m_delay_line;
        MinMax<T>           m_min_max;
        T                   m_prev_signal = 0;
        bool                m_use_diff = false;

        enum PulseType {
            M1 = 0,
            M2,
            M3,
            M4,
            M5,
            M6,
        };

        std::array<T, 6>    m_pulse;

        template <class T = double>
        class Detector {
        private:
            size_t  m_period_min = 0;
            size_t  m_period_max = 0;
            size_t  m_decay_time = 0;
            size_t  m_time = 0;
            size_t  m_period = 0;
            T       m_beta = 0;
            T       m_exp_time = 0;
            bool    is_lock = false;
            bool    is_init = false;

        public:

            Detector() {};

            Detector(const size_t period_min, const size_t period_max) :
                m_period_min(period_min), m_period_min(m_period_max) {
            }

            bool update(const T value) noexcept {
                ++m_time;
                if (is_lock) {
                    if (m_decay_time) --m_decay_time;
                    is_lock = (m_decay_time > 0);
                    return false;
                }
                //экспоненциальный разряд
                const T thresholt_value = peak_value * std::exp(-m_exp_time/m_beta);
                m_exp_time += 1.0;
                if (value > thresholt_value) {
                    // Запираем детектор
                    is_lock = true;
                    // Находим период
                    if (m_period > 0) {
                        m_period = (m_time + m_period) / 2;
                        is_init = true;
                    } else {
                        m_period = m_time;
                    }
                    if (m_period > m_period_max) m_period = m_period_max;
                    else if (m_period < m_period_min) m_period = m_period_min;
                    // Находим время запирания
                    m_decay_time = (size_t)((0.4 * (T)m_period) + 0.5);
                    // Находим постоянную разряда
                    m_beta = (T)m_period / 0.695;

                    peak_value = value;
                    m_exp_time = 0;
                    m_time = 0;
                    return true;
                }
                return false;
            }

            inline bool is_ready() noexcept {
                return is_init;
            }

            void reset() noexcept {
                m_decay_time = 0;
                m_time = 0;
                m_period = 0;
                m_beta = 0;
                m_exp_time = 0;
                is_lock = false;
                is_init = false;
            }

            inline size_t get() noexcept {
                return m_period;
            }
        };

        std::array<std::array<size_t, 6>, 6> m_matrix;
        std::array<Detector, 6> m_detector;
        std::array<size_t, 4>   m_period_range_min;
        std::array<size_t, 4>   m_period_range_max;
        size_t m_period = 0;

    public:

        PeriodDetector() : BaseIndicator<T>(3) {};

        /** \brief Initialize the Period Detector indicator
         * \param period_filter     Period Filter (LowPass)
         * \param use_diff_trans    Use Difference Transformation
         */
        PeriodDetector(const size_t period_min, const size_t period_max, const size_t period_filter, const bool use_diff_trans) :
                BaseIndicator<T>(3),
                m_filter(period_filter),
                m_use_diff(use_diff_trans), m_delay_line(1) {
            std::fill(m_pulse.begin(), m_pulse.end(), 0.0);
            std::fill(m_matrix.begin(), m_matrix.end(), std::array<size_t, 6>(0));
            std::fill(m_detector.begin(), m_detector.end(), Detector(period_min, period_max));

            const size_t part = (period_max - period_min) / 15;
            m_period_range_min[0] = period_min;
            m_period_range_min[1] = m_period_range_min[0] + part;
            m_period_range_min[2] = m_period_range_min[1] + part * 2;
            m_period_range_min[3] = m_period_range_min[2] + part * 4;
            m_period_range_max[0] = m_period_range_min[1];
            m_period_range_max[1] = m_period_range_min[2];
            m_period_range_max[2] = m_period_range_min[3];
            m_period_range_max[3] = period_max;
        }

        /** \brief Update the state of the indicator
         * \param value New value for the indicator, such as price
         * \param type  Price type (affects the indicator's mode of operation - intra-bar or end-of-bar)
         * \return Returns true in case of success
         */
        inline bool update(const T value, const PriceType type = PriceType::Close) noexcept {
            // Предобработка сигнала
            if (m_use_diff) {
                m_delay_line.update(value, type);
                if (!m_delay_line.is_ready()) return false;
                m_filter.update(value - m_delay_line.get(), type);
            } else {
                m_filter.update(value, type);
            }
            if (!m_filter.is_ready()) return false;

            /*
            m_buffer.update(m_filter.get(), type);
            if (!m_buffer.is_ready()) return false;
            */

            // Детектор пиков
            bool is_up = false;
            bool is_dn = false;
            const T &signal = m_filter.get();
            if (m_prev_signal >= 0) {
                if (signal < m_prev_signal) {
                    is_up = true;
                    if (m_prev_signal > m_pulse[M1]) {
                        // Обновляется максимум
                        m_pulse[M3] = m_prev_signal - m_pulse[M1];
                        m_pulse[M1] = m_prev_signal;
                    } else {
                        m_pulse[M3] = 0;
                    }
                    m_pulse[M2] = m_pulse[M1] + m_pulse[M4];
                }
            } else {
                if (signal > m_prev_signal) {
                    is_dn = true;
                    const T abs_prev_signal = std::abs(m_prev_signal);
                    if (abs_prev_signal > m_pulse[M4]) {
                        // Обновляется минимум
                        m_pulse[M6] = abs_prev_signal - m_pulse[M4];
                        m_pulse[M4] = abs_prev_signal;
                    } else {
                        m_pulse[M6] = 0;
                    }
                    m_pulse[M5] = m_pulse[M1] + m_pulse[M4];
                }
            }
            m_prev_signal = signal;

            // Проверяем наличие измерений
            for (size_t i = M1; i <= M6; ++i) {
                if (!m_detector[i].is_ready()) return false;
            }

            // Обрабатываем детекторами периодов
            if (is_up) {
                for (size_t i = M1; i <= M3; ++i) {
                    if (m_pulse[i] == 0) continue;
                    if (m_detector[i].update(m_pulse[i])) {
                        m_matrix[i][2] = m_matrix[i][1];
                        m_matrix[i][1] = m_matrix[i][0];
                        m_matrix[i][0] = m_detector[i].get();
                    }
                }
            } else
            if (is_dn) {
                for (size_t i = M4; i <= M6; ++i) {
                    if (m_pulse[i] == 0) continue;
                    if (m_detector[i].update(m_pulse[i])) {
                        m_matrix[i][2] = m_matrix[i][1];
                        m_matrix[i][1] = m_matrix[i][0];
                        m_matrix[i][0] = m_detector[i].get();
                    }
                }
            }
            // Проверяем, что все данные заполнены
            for (size_t i = M1; i <= M6; ++i) {
                for (size_t j = M1; j <= M6; ++j) {
                    if (m_matrix[i] == 0) return false;
                }
            }
            // Вычисляем нижнию часть строк
            for (size_t i = M1; i <= M6; ++i) {
                m_matrix[i][3] = m_matrix[i][0] + m_matrix[i][1];
                m_matrix[i][4] = m_matrix[i][1] + m_matrix[i][2];
                m_matrix[i][5] = m_matrix[i][0] + m_matrix[i][1] + m_matrix[i][2];
            }
            // Ведем подсчет совпадений
            static const size_t score_offsets[4] = {1,2,5,7};
            static const size_t window_width[4][4] = {
                {1,2,3,4},
                {2,4,6,8},
                {4,8,12,16},
                {8,16,24,32}
            };
            std::vector<int> periods_score(6,0);
            for (size_t i = M1; i <= M6; ++i) {
                const size_t tp = m_matrix[i][0];
                // находим номер диапазона значения периода
                size_t col = 0;
                for (size_t k = 0; k < 4; ++k) {
                    if (tp >= m_period_range_min[k] && tp < m_period_range_max[k]) {
                        col = k;
                        break;
                    }
                }
                std::vector<int> period_score(4,0);
                for (size_t k = 0; k < 4; ++k) {
                    for (size_t l = M1; l <= M6; ++l) {
                        for (size_t j = M1; j <= M6; ++j) {
                            if (i == l && j == 0) continue;
                            const size_t score = std::abs(tp - m_matrix[l][j]);
                            if (score > window_width[k][col]) continue;
                            ++period_score[k];
                        } // for j
                    }
                    period_score[k] -= score_offsets[k];
                } // for k
                auto it = std::max_element(period_score.begin(), period_score.end());
                periods_score[i] = it != period_score.end() ? *it : period_score[0];
            }
            auto it = std::max_element(periods_score.begin(), periods_score.end());
            m_period = it != periods_score.end() ? *it : periods_score[0];
            BaseIndicator<T>::output_value[0] = m_period;
            is_done = true;
            return true;
        }

        inline bool is_ready() const noexcept {
            return is_done;
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            buffer.reset();
            ma_slow.reset();
            ma_vol.reset();
            for (auto &item : BaseIndicator<T>::output_value) {
                item = get_empty_value<T>();
            }
        }
    };
}; // xtechnical

#endif // XTECHNICAL_PERIOD_DETECTOR_HPP_INCLUDED
