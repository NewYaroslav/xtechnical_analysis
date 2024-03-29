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
#ifndef XTECHNICAL_INDICATORS_HPP_INCLUDED
#define XTECHNICAL_INDICATORS_HPP_INCLUDED

#include "xtechnical_dft.hpp"
#include "xtechnical_correlation.hpp"
#include "xtechnical_normalization.hpp"
#include "xtechnical_moving_window.hpp"
#include "xtechnical_circular_buffer.hpp"
#include "xtechnical_common.hpp"
#include "math/xtechnical_compare.hpp"
#include "math/xtechnical_smoothing.hpp"

#include "indicators/xtechnical_delay_line.hpp"
#include "indicators/xtechnical_sma.hpp"
#include "indicators/xtechnical_rsi.hpp"
#include "indicators/xtechnical_fast_min_max.hpp"
#include "indicators/xtechnical_fisher.hpp"
#include "indicators/xtechnical_fractals.hpp"
#include "indicators/xtechnical_cluster_shaper.hpp"
#include "indicators/xtechnical_true_range.hpp"
#include "indicators/xtechnical_atr.hpp"
#include "indicators/xtechnical_cci.hpp"
#include "indicators/xtechnical_super_trend.hpp"
#include "indicators/xtechnical_body_filter.hpp"
#include "indicators/xtechnical_period_stats.hpp"
#include "indicators/ssa.hpp"

#include <vector>
#include <deque>
#include <list>
#include <algorithm>
#include <functional>
#include <numeric>
#include <cmath>

#define INDICATORSEASY_DEF_RING_BUFFER_SIZE 1024

namespace xtechnical {

    /** \brief Процент разницы между актуальной ценой и ценой в прошлом
     */
    template <typename T>
    class PercentDifference {
    private:
        DelayLine<T> delay_line;
        T output_value = std::numeric_limits<T>::quiet_NaN();
    public:

        PercentDifference() {};

        PercentDifference(const size_t p) : delay_line(p) {};

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in) noexcept {
            delay_line.update(in);
            if (std::isnan(delay_line.get())) return common::INDICATOR_NOT_READY_TO_WORK;
            if (delay_line.get() == 0) {
                if (in > 0) output_value = 100;
                else if (in < 0) output_value = -100;
                else output_value = 0;
                return common::OK;
            }
            output_value = ((in - delay_line.get()) / delay_line.get()) * 100;
            return common::OK;
        }

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &out) noexcept {
            const int err = update(in);
            out = output_value;
            return err;
        }

        /** \brief Протестировать индикатор
         *
         * Данный метод отличается от update тем, что не влияет на внутреннее состояние индикатора
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in) noexcept {
            delay_line.update(in);
            if (std::isnan(delay_line.get())) return common::INDICATOR_NOT_READY_TO_WORK;
            if (delay_line.get() == 0) {
                if (in > 0) output_value = 100;
                else if (in < 0) output_value = -100;
                else output_value = 0;
                return common::OK;
            }
            output_value = ((in - delay_line.get()) / delay_line.get()) * 100;
            return common::OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данный метод отличается от update тем, что не влияет на внутреннее состояние индикатора
         * \param in    Сигнал на входе
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &out) noexcept {
            const int err = test(in);
            out = output_value;
            return err;
        }

        inline T get() const noexcept {
            return output_value;
        }

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            delay_line.clear();
            output_value = std::numeric_limits<T>::quiet_NaN();
        }
    };

    /** \brief Ro — темп изменения цены. Другое имя — Momentum
     *
     * Отличие в том, что в RoC линией баланса является «0», а в Momentum «100»
     * Формула:
     * Pt  — текущая цена
     * Pt-n — цена  n периодов назад
     * Momentum=Pn-Pn-1
     * RoCtn=((Momentum/Pt-n)*100)-100
     */
    template<class T>
    class RoC {
    private:
        xtechnical::circular_buffer<T> buffer;
        size_t buffer_size = 0;
        T output_value = std::numeric_limits<T>::quiet_NaN();
    public:

        RoC() {};

        /** \brief Конструктор RoC
         * \param period    Период
         */
        RoC(const size_t period) :
            buffer(period), buffer_size(period) {
        }

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in) noexcept {
            if(buffer_size == 0) return common::NO_INIT;
            buffer.update(in);
            if (buffer.full()) {
                const T x0 = buffer.front();
                const T x1 = buffer.back();
                if(x0 == 0) output_value = x1 > 0 ? 100 : -100;
                else output_value = ((x1 - x0) / x0) * 100.0;
                return common::OK;
            }
            return common::INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &out) noexcept {
            const int err = update(in);
            out = output_value;
            return err;
        }

        /** \brief Протестировать индикатор
         *
         * Данный метод отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in) noexcept {
            if(buffer_size == 0) return common::NO_INIT;
            buffer.test(in);
            if (buffer.full()) {
                const T x0 = buffer.front();
                const T x1 = buffer.back();
                if(x0 == 0) output_value = x1 > 0 ? 100 : -100;
                else output_value = ((x1 - x0) / x0) * 100.0;
                return common::OK;
            }
            return common::INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Протестировать индикатор
         *
         * Данный метод отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in    Сигнал на входе
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &out) noexcept {
            const int err = test(in);
            out = output_value;
            return err;
        }

        inline T get() const noexcept {
            return output_value;
        }

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            buffer.clear();
            output_value = std::numeric_limits<T>::quiet_NaN();
        }
    };

    /** \brief Cкользящий Min Max
     */
    template <class T>
    class MinMax {
    private:
        xtechnical::circular_buffer<T> buffer;
        T output_min_value = std::numeric_limits<T>::quiet_NaN();
        T output_max_value = std::numeric_limits<T>::quiet_NaN();
        size_t period = 0;
        size_t offset = 0;
    public:
        MinMax() {};

        /** \brief Конструктор скользящего Min Max
         * \param p     Период
         * \param o     Смещение назад
         */
        MinMax(const size_t p, const size_t o = 0) :
                buffer(p + o), period(p), offset(o) {
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in) noexcept {
            if(period == 0) {
                output_min_value = std::numeric_limits<T>::quiet_NaN();
                output_max_value = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }
            buffer.update(in);
            if(buffer.full()) {
                T temp = buffer[0];
                output_max_value = output_min_value = temp;
                for(size_t i = 1; i < period; ++i) {
                    temp = buffer[i];
                    if(output_max_value < temp) output_max_value = temp;
                    else if(output_min_value > temp) output_min_value = temp;
                }
            } else {
                output_min_value = std::numeric_limits<T>::quiet_NaN();
                output_max_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            return common::OK;
        }

        /** \brief Обновить состояние индикатора
         * \param in        Сигнал на входе
         * \param min_value Минимальный сигнал на выходе за период
         * \param max_value Максимальный сигнал на выходе за период
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &min_value, T &max_value) noexcept {
            const int err = update(in);
            min_value = output_min_value;
            max_value = output_max_value;
            return err;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in) noexcept {
            if(period == 0) {
                output_min_value = std::numeric_limits<T>::quiet_NaN();
                output_max_value = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }
            buffer.test(in);
            if(buffer.full()) {
                T temp = buffer[0];
                output_max_value = output_min_value = temp;
                for(size_t i = 1; i < period; ++i) {
                    temp = buffer[i];
                    if(output_max_value < temp) output_max_value = temp;
                    else if(output_min_value > temp) output_min_value = temp;
                }
            } else {
                output_min_value = std::numeric_limits<T>::quiet_NaN();
                output_max_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            return common::OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем, что не влияет на внутреннее состояние индикатора
         * \param in        Сигнал на входе
         * \param min_value Минимальный сигнал на выходе за период
         * \param max_value Максимальный сигнал на выходе за период
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &min_value, T &max_value) noexcept {
            const int err = test(in);
            min_value = output_min_value;
            max_value = output_max_value;
            return err;
        }

        /** \brief Получить минимальное значение индикатора
         * \return Минимальное значение индикатора
         */
        inline T get_min() const noexcept {
            return output_min_value;
        }

        /** \brief Получить максимальное значение индикатора
         * \return Максимальное значение индикатора
         */
        inline T get_max() const noexcept {
            return output_max_value;
        }

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            buffer.clear();
            output_min_value = std::numeric_limits<T>::quiet_NaN();
            output_max_value = std::numeric_limits<T>::quiet_NaN();
        }
    };

#if(0)
    /** \brief Быстрый алгоритм поиска Min и Max
     * Оригинал: https://arxiv.org/abs/cs/0610046v5
     */
    template <class T>
    class FastMinMax {
    private:
        T output_max_value = std::numeric_limits<T>::quiet_NaN();
        T output_min_value = std::numeric_limits<T>::quiet_NaN();
        T last_input = 0;
        int64_t period = 0;
        int64_t index = 0;
        std::deque<std::pair<int64_t, T>> U, L;
        DelayLine<T> delay_line;
    public:
        FastMinMax() {};

        FastMinMax(const size_t p, const size_t o = 0) :
            period((int64_t)p), delay_line(o) {
        };

        int update(T input) noexcept {
            if(delay_line.update(input) != OK) {
                return INDICATOR_NOT_READY_TO_WORK;
            }
            input = delay_line.get();
            if (index == 0) {
                ++index;
                last_input = input;
                return INDICATOR_NOT_READY_TO_WORK;
            }
            if (input > last_input) {
                L.push_back(std::make_pair(index - 1, last_input));
                if (index == period + L.front().first) L.pop_front() ;
                while (U.size() > 0) {
                    if (input <= U.back().second) {
                        if (index == period + U.front().first) U.pop_front();
                        break ;
                    } // end if
                    U.pop_back() ;
                } // end while
            } else {
                U.push_back(std::make_pair(index - 1, last_input)) ;
                if (index == period + U.front().first) U.pop_front() ;
                while (L.size() > 0) {
                    if (input >= L.back().second) {
                        if (index == period + L.front().first) L.pop_front();
                        break ;
                    } // end if
                    L.pop_back();
                } // end while
            } // end if else
            ++index;
            if (index >= period) {
                output_max_value = U.size() > 0 ? U.front().second : input;
                output_min_value = L.size() > 0 ? L.front().second : input;
                last_input = input;
                return OK;
            }
            last_input = input;
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Обновить состояние индикатора
         * \param input     Сигнал на входе
         * \param min_value Минимальный сигнал на выходе за период
         * \param max_value Максимальный сигнал на выходе за период
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T input, T &min_value, T &max_value) noexcept {
            const int err = update(input);
            min_value = output_min_value;
            max_value = output_max_value;
            return err;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем, что не влияет на внутреннее состояние индикатора
         * \param input     Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(T input) noexcept {
            if(delay_line.test(input) != OK) {
                return INDICATOR_NOT_READY_TO_WORK;
            }
            input = delay_line.get();
            if (index == 0) return INDICATOR_NOT_READY_TO_WORK;
            std::deque<std::pair<int64_t, T>> tU(U), tL(L);
            if (input > last_input) {
                tL.push_back(std::make_pair(index - 1, last_input));
                if (index == period + tL.front().first) tL.pop_front() ;
                while (tU.size() > 0) {
                    if (input <= tU.back().second) {
                        if (index == period + tU.front().first) tU.pop_front();
                        break ;
                    } // end if
                    tU.pop_back() ;
                } // end while
            } else {
                tU.push_back(std::make_pair(index - 1, last_input)) ;
                if (index == period + tU.front().first) tU.pop_front() ;
                while (tL.size() > 0) {
                    if (input >= tL.back().second) {
                        if (index == period + tL.front().first) tL.pop_front();
                        break ;
                    } // end if
                    tL.pop_back();
                } // end while
            } // end if else
            if ((index + 1) >= period) {
                output_max_value = tU.size() > 0 ? tU.front().second : input;
                output_min_value = tL.size() > 0 ? tL.front().second : input;
                return OK;
            }
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем, что не влияет на внутреннее состояние индикатора
         * \param input     Сигнал на входе
         * \param min_value Минимальный сигнал на выходе за период
         * \param max_value Максимальный сигнал на выходе за период
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T input, T &min_value, T &max_value) {
            const int err = test(input);
            min_value = output_min_value;
            max_value = output_max_value;
            return err;
        }

        /** \brief Получить минимальное значение индикатора
         * \return Минимальное значение индикатора
         */
        inline T get_min() const noexcept {
            return output_min_value;
        }

        /** \brief Получить максимальное значение индикатора
         * \return Максимальное значение индикатора
         */
        inline T get_max() const noexcept {
            return output_max_value;
        }

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            output_min_value = std::numeric_limits<T>::quiet_NaN();
            output_max_value = std::numeric_limits<T>::quiet_NaN();
            last_input = 0;
            index = 0;
            U.clear();
            L.clear();
            delay_line.clear();
        }
    };
#endif

    /** \brief Индикатор Stochastics
     */
	template <typename T, class MA_TYPE>
	class Stochastics {
	private:
		//xtechnical::circular_buffer<T> buffer;
		FastMinMax<T> min_max;
		MA_TYPE k_ma;
		MA_TYPE d_ma;

		T output_value = std::numeric_limits<T>::quiet_NaN();
        T output_k_value = std::numeric_limits<T>::quiet_NaN();
        T output_d_value = std::numeric_limits<T>::quiet_NaN();

        size_t period = 0;
	public:
		Stochastics() {};

        /** \brief Конструктор Stochastics
         * \param p     Период
         * \param pk    Период %K
         * \param pd    Период %D
         * \param o     Смещение назад
         */
        Stochastics(
				const size_t p,
				const size_t pk,
				const size_t pd,
				const size_t o = 0) :
            min_max(p, o),
			k_ma(pk),
			d_ma(pd),
			period(p)  {
        }

		/** \brief Обновить состояние индикатора
         * \param in 		Котировка на входе индикатора
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in) noexcept {
            if(period == 0) {
                output_k_value = std::numeric_limits<T>::quiet_NaN();
                output_d_value = std::numeric_limits<T>::quiet_NaN();
				output_value = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }
            if (min_max.update(in) == common::OK) {
                const T diff = min_max.get_max() - min_max.get_min();
				output_value = diff == 0 ? (T)50 : (T)100 * ((in - min_max.get_min()) / diff);
				k_ma.update(output_value);
				output_k_value = k_ma.get();
				if (!std::isnan(output_k_value)) {
					d_ma.update(output_k_value);
					output_d_value = d_ma.get();
				}
            } else {
				output_value = std::numeric_limits<T>::quiet_NaN();
				output_k_value = std::numeric_limits<T>::quiet_NaN();
				output_d_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            return common::OK;
        }

        /** \brief Обновить состояние индикатора
         * \param in 		Котировка на входе индикатора
         * \param k_value 	Линия %K
         * \param d_value 	Линия %D
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &k_value, T &d_value) noexcept {
            const int err = update(in);
            k_value = output_k_value;
            d_value = output_d_value;
            return err;
        }

		/** \brief Протестировать индикатор
         * \param in 		Котировка на входе индикатора
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in) noexcept {
            if(period == 0) {
                output_k_value = std::numeric_limits<T>::quiet_NaN();
                output_d_value = std::numeric_limits<T>::quiet_NaN();
				output_value = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }
            if (min_max.test(in) == common::OK) {
                const T diff = min_max.get_max() - min_max.get_min();
				output_value = diff == 0 ? (T)50 : (T)100 * ((in - min_max.get_min()) / diff);
				k_ma.test(output_value);
				output_k_value = k_ma.get();
				if (!std::isnan(output_k_value)) {
					d_ma.test(output_k_value);
					output_d_value = d_ma.get();
				}
            } else {
				output_value = std::numeric_limits<T>::quiet_NaN();
				output_k_value = std::numeric_limits<T>::quiet_NaN();
				output_d_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            return common::OK;
        }

        /** \brief Протестировать индикатор
         * \param in 		Котировка на входе индикатора
         * \param k_value 	Линия %K
         * \param d_value 	Линия %D
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &k_value, T &d_value) noexcept {
            const int err = test(in);
            k_value = output_k_value;
            d_value = output_d_value;
            return err;
        }

		/** \brief Получить значение %K до сглаживания
         * \return Значение %K до сглаживания или NaN, если значение отсутствует
         */
        inline T get() const noexcept {
            return output_value;
        }

        /** \brief Получить сглаженное значение %K
         * \return Значение %K после сглаживания или NaN, если значение отсутствует
         */
        inline T get_k() const noexcept {
            return output_k_value;
        }

		/** \brief Получить сглаженное значение %D
         * \return Значение %D после сглаживания или NaN, если значение отсутствует
         */
        inline T get_d() const noexcept {
            return output_d_value;
        }

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            min_max.clear();
			k_ma.clear();
			d_ma.clear();
			output_value = std::numeric_limits<T>::quiet_NaN();
            output_k_value = std::numeric_limits<T>::quiet_NaN();
            output_d_value = std::numeric_limits<T>::quiet_NaN();
        }
	};

    /** \brief Cкользящий Z-score
     */
    template <typename T>
    class Zscore {
    private:
        xtechnical::circular_buffer<T> buffer;
        T last_data = 0;
        T output_value = std::numeric_limits<T>::quiet_NaN();
        size_t period = 0;
    public:
        Zscore() {};

        /** \brief Инициализировать простую скользящую среднюю
         * \param p     Период
         */
        Zscore(const size_t p) :
                buffer(p + 1), period(p) {
        }

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in) noexcept {
            if(period == 0) {
                output_value = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }
            buffer.update(in);
            if(buffer.full()) {
                last_data = last_data + (in - buffer.front());
                T mean = last_data/(T)period;
                T diff = 0, sum = 0;
                for(size_t i = 1; i <= period; ++i) {
                    diff = (buffer[i] - mean);
                    sum += diff * diff;
                }
                sum /= (T)(period - 1);
                T std_dev = sum > 0 ? std::sqrt(sum) : 0;
                output_value = std_dev > 0 ? ((in - mean) / std_dev) : 0;
            } else {
                last_data += in;
                output_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            return common::OK;
        }

        /** \brief Обновить состояние индикатора
         * \param in    Cигнал на входе
         * \param out   Cигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &out) noexcept {
            const int err = update(in);
            out = output_value;
            return err;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in) {
            if(period == 0) {
                output_value = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }
            buffer.test(in);
            if(buffer.full()) {
                T mean = (last_data + (in - buffer.front()))/(T)period;
                T diff = 0, sum = 0;
                for(size_t i = 1; i <= period; ++i) {
                    diff = (buffer[i] - mean);
                    sum += diff * diff;
                }
                sum /= (T)(period - 1);
                T std_dev = sum > 0 ? std::sqrt(sum) : 0;
                output_value = std_dev > 0 ? ((in - mean) / std_dev) : 0;
            } else {
                output_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            return common::OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in    Сигнал на входе
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &out) noexcept {
            const int err = test(in);
            out = output_value;
            return err;
        }

        /** \brief Получить значение индикатора
         * \return Значение индикатора
         */
        inline T get() const noexcept {
            return output_value;
        }

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            buffer.clear();
            output_value = std::numeric_limits<T>::quiet_NaN();
            last_data = 0;
        }
    };

    /** \brief Стандартное отклонение
     */
    template <typename T>
    class StdDev {
    private:
        xtechnical::circular_buffer<T> buffer;
        T last_data = 0;
        T output_value = std::numeric_limits<T>::quiet_NaN();
        size_t period = 0;
    public:
        StdDev() {};

        /** \brief Инициализировать простую скользящую среднюю
         * \param p     Период
         */
        StdDev(const size_t p) :
                buffer(p + 1), period(p) {
        }

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in) noexcept {
            if(period == 0) {
                output_value = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }
            buffer.update(in);
            if(buffer.full()) {
                last_data = last_data + (in - buffer.front());
                T mean = last_data/(T)period;
                T diff = 0, sum = 0;
                for(size_t i = 1; i <= period; ++i) {
                    diff = (buffer[i] - mean);
                    sum += diff * diff;
                }
                sum /= (T)(period - 1);
                output_value = sum > 0 ? std::sqrt(sum) : 0;
            } else {
                last_data += in;
                output_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            return common::OK;
        }

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &out) {
            const int err = update(in);
            out = output_value;
            return common::OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in) noexcept {
            if(period == 0) {
                output_value = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }
            buffer.test(in);
            if(buffer.full()) {
                T mean = (last_data + (in - buffer.front()))/(T)period;
                T diff = 0, sum = 0;
                for(size_t i = 1; i <= period; ++i) {
                    diff = (buffer[i] - mean);
                    sum += diff * diff;
                }
                sum /= (T)(period - 1);
                output_value = sum > 0 ? std::sqrt(sum) : 0;
            } else {
                output_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            return common::OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in    Сигнал на входе
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &out) {
            const int err = test(in);
            out = output_value;
            return err;
        }

        /** \brief Получить значение индикатора
         * \return Значение индикатора
         */
        inline T get() const noexcept {
            return output_value;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() noexcept {
            buffer.clear();
            output_value = std::numeric_limits<T>::quiet_NaN();
            last_data = 0;
        }
    };

    /** \brief Линия задержки события
     */
    template <class T>
    class DelayEvent {
    private:
        std::list<std::pair<uint32_t, T>> data;
    public:

        DelayEvent() {};

        /** \brief Добавить событие
         * \param event Событие
         * \param delay Задержка
         */
        void add(T &event, const uint32_t delay) {
            data.push_back(std::make_pair(delay, event));
        }

        /** \brief Обновить состояние индикатора
         * \return вернет true, если появилось событие
         */
        bool update() {
            if(data.size() == 0) return false;
            bool is_event = false;
            for(auto &it : data) {
                if(it.first > 0) --it.first;
                if(it.first == 0) is_event = true;
            }
            return is_event;
        }

        /** \brief Получить массив состоявшихся событий
         * \return Массив событий
         */
        std::vector<T> get() {
            std::vector<T> temp;
            if(data.size() == 0) return temp;

            auto it = data.begin();
            while(it != data.end()) {
                if(it->first == 0) {
                    temp.push_back(it->second);
                    it = data.erase(it);
                    continue;
                }
                ++it;
            }
            return temp;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            data.clear();
        }
    };

    /** \brief Cкользящий Min Max Difference
     */
    template <typename T>
    class MinMaxDiff {
    private:
        xtechnical::circular_buffer<T> buffer;
        DelayLine<T> delay_line;
        T output_value = std::numeric_limits<T>::quiet_NaN();
        T output_min_value = std::numeric_limits<T>::quiet_NaN();
        T output_max_value = std::numeric_limits<T>::quiet_NaN();
        size_t period = 0;
        size_t offset = 0;
    public:
        MinMaxDiff() {};

        /** \brief Конструктор скользящего Min Max Difference
         * \param p     Период
         * \param o     Смещение назад
         */
        MinMaxDiff(const size_t p, const size_t o = 0) :
                buffer(p + o), delay_line(1),
                period(p), offset(o) {
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in) noexcept {
            if(period == 0) {
                output_value = std::numeric_limits<T>::quiet_NaN();
                output_min_value = std::numeric_limits<T>::quiet_NaN();
                output_max_value = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }

            if(delay_line.update(in) != common::OK) {
                output_value = std::numeric_limits<T>::quiet_NaN();
                output_min_value = std::numeric_limits<T>::quiet_NaN();
                output_max_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            output_value = std::abs(in - delay_line.get());
            buffer.update(output_value);
            if(buffer.full()) {
                T temp = buffer[0];
                output_max_value = output_min_value = temp;
                for(size_t i = 1; i < period; ++i) {
                    temp = buffer[i];
                    if(output_max_value < temp) output_max_value = temp;
                    else if(output_min_value > temp) output_min_value = temp;
                }
            } else {
                output_value = std::numeric_limits<T>::quiet_NaN();
                output_min_value = std::numeric_limits<T>::quiet_NaN();
                output_max_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            return common::OK;
        }

        /** \brief Обновить состояние индикатора
         * \param in        Сигнал на входе
         * \param out       Сигнал на выходе
         * \param min_value Минимальный сигнал на выходе
         * \param max_value Максимальный сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &out, T &min_value, T &max_value) noexcept {
            const int err = update(in);
            out = output_value;
            min_value = output_min_value;
            max_value = output_max_value;
            return err;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in) noexcept {
            if(period == 0) {
                output_value = std::numeric_limits<T>::quiet_NaN();
                output_min_value = std::numeric_limits<T>::quiet_NaN();
                output_max_value = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }
            if(delay_line.test(in) != common::OK) {
                output_value = std::numeric_limits<T>::quiet_NaN();
                output_min_value = std::numeric_limits<T>::quiet_NaN();
                output_max_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            output_value = std::abs(in - delay_line.get());
            buffer.test(output_value);
            if(buffer.full()) {
                T temp = buffer[0];
                output_max_value = output_min_value = temp;
                for(size_t i = 1; i < period; ++i) {
                    temp = buffer[i];
                    if(output_max_value < temp) output_max_value = temp;
                    else if(output_min_value > temp) output_min_value = temp;
                }
            } else {
                output_value = std::numeric_limits<T>::quiet_NaN();
                output_min_value = std::numeric_limits<T>::quiet_NaN();
                output_max_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            return common::OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем, что не влияет на внутреннее состояние индикатора
         * \param in Сигнал на входе
         * \param out Сигнал на выходе
         * \param min_value Минимальный сигнал на выходе
         * \param max_value Максимальный сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &out, T &min_value, T &max_value) noexcept {
            const int err = test(in);
            out = output_value;
            min_value = output_min_value;
            max_value = output_max_value;
            return common::OK;
        }

        /** \brief Получить значение индикатора
         * \return Значение индикатора
         */
        inline T get() const noexcept {
            return output_value;
        }

        /** \brief Получить минимальное значение индикатора
         * \return Минимальное значение индикатора
         */
        inline T get_min() const noexcept {
            return output_min_value;
        }

        /** \brief Получить максимальное значение индикатора
         * \return Максимальное значение индикатора
         */
        inline T get_max() const noexcept {
            return output_max_value;
        }

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            buffer.clear();
            output_value = std::numeric_limits<T>::quiet_NaN();
            output_min_value = std::numeric_limits<T>::quiet_NaN();
            output_max_value = std::numeric_limits<T>::quiet_NaN();
        }
    };

    /** \brief Кумулятивное скользящее среднее
     */
    template <typename T>
    class CMA {
    private:
        uint64_t n = 0;
        uint64_t tn = 0;
        T sum = 0;
        bool is_test = false;
    public:
        CMA() {};

        double update(const T in) noexcept {
            sum += in;
            ++n;
            is_test = false;
            return sum / (T)n;
        }

        void update(const T in, T &out) noexcept {
            sum += in;
            ++n;
            out = sum / (T)n;
            is_test = false;
        }

        double test(const T in) {
            T _sum = sum;
            tn = n;
            _sum += in;
            ++tn;
            is_test = true;
            return _sum / (T)tn;
        }

        void test(const T in, T &out) {
            T _sum = sum;
            tn = n;
            _sum += in;
            ++tn;
            out = _sum / (T)tn;
            is_test = true;
        }

        int get_period() {return is_test ? tn : n;};

        void clear() {
            n = 0;
            sum = 0;
        }
    };

    /** \brief Кумулятивное скользящее среднее с объемом
     */
    template <typename T>
    class VCMA {
    private:
        uint64_t n = 0;
        uint64_t tn = 0;
        T sum = 0;
        T sum_weight = 0;
        bool is_test = false;
    public:

        VCMA() {};

        double update(const T input, const T weight) {
            sum += input * weight;
            sum_weight += weight;
            ++n;
            is_test = false;
            if(sum_weight == 0) return 0.0;
            return sum / sum_weight;
        }

        void update(const T input, const T weight, T &out) {
            sum += input * weight;
            sum_weight += weight;
            ++n;
            if(sum_weight == 0) out = 0.0;
            else out = sum / sum_weight;
            is_test = false;
        }

        double test(const T input, const T weight) {
            tn = n;
            T _sum = sum;
            T _sum_weight = sum_weight;

            _sum += input * weight;
            _sum_weight += weight;
            ++tn;
            is_test = true;
            if(_sum_weight == 0) return 0.0;
            return _sum / _sum_weight;
        }

        void test(const T input, const T weight, T &out) {
            tn = n;
            T _sum = sum;
            T _sum_weight = sum_weight;

            _sum += input * weight;
            _sum_weight += weight;
            ++tn;
            if(_sum_weight == 0) out = 0.0;
            else out = _sum / _sum_weight;
            is_test = true;
        }

        int get_period() {return is_test ? tn : n;};

        void clear() {
            n = 0;
            tn = 0;
            sum = 0;
            sum_weight = 0;
            is_test = false;
        }
    };

    /** \brief Скользящая сумма
     */
    template <typename T>
    class SUM {
    private:
        xtechnical::circular_buffer<T> buffer;
        T last_data = 0;
        T output_value = std::numeric_limits<T>::quiet_NaN();
        size_t period = 0;
    public:
        SUM() {};

        /** \brief Конструктор cкользящей суммы
         * \param user_period период
         */
        SUM(const size_t user_period) :
                buffer(user_period + 1), period(user_period) {
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &out) {
            if(period == 0) {
                out = output_value = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }
            buffer.update(in);
            if(buffer.full()) {
                last_data = last_data + (in - buffer.front());
                out = output_value = last_data;
            } else {
                last_data += in;
                out = output_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            return common::OK;
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in) {
            if(period == 0) {
                output_value = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }
            buffer.update(in);
            if(buffer.full()) {
                last_data = last_data + (in - buffer.front());
                output_value = last_data;
            } else {
                last_data += in;
                output_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            return common::OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &out) {
            if(period == 0) {
                out = output_value = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }
            buffer.test(in);
            if(buffer.full()) {
                out = output_value = (last_data + (in - buffer.front()));
            } else {
                out = output_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            return common::OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in) {
            if(period == 0) {
                output_value = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }
            buffer.test(in);
            if(buffer.full()) {
                output_value = (last_data + (in - buffer.front()));
            } else {
                output_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            return common::OK;
        }

        /** \brief Получить значение индикатора
         * \return Значение индикатора
         */
        inline T get() const {
            return output_value;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            buffer.clear();
            output_value = std::numeric_limits<T>::quiet_NaN();
            last_data = 0;
        }
    };

    /** \brief Взвешенное скользящее среднее
     */
    template <typename T>
    class WMA {
    private:
        std::vector<T> data_;
        T output_value = std::numeric_limits<T>::quiet_NaN();
        size_t period = 0;
    public:
        WMA() {};

        /** \brief Инициализировать взвешенное скользящее среднее
         * \param p Период
         */
        WMA(const size_t p) : period(p) {
            data_.reserve(p);
        }

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in) noexcept {
            if(period == 0) {
                output_value = in;
                return common::NO_INIT;
            }
            if(data_.size() < (size_t)period) {
                data_.push_back(in);
                if(data_.size() == (size_t)period) {
                    T sum = 0;
                    for(size_t i = data_.size(); i > 0; i--) {
                        sum += data_[i - 1] * (T)i;
                    }
                    output_value = (sum * 2.0d) / ((T)period * ((T)period + 1.0d));
                    return common::OK;
                }
            } else {
                data_.push_back(in);
                data_.erase(data_.begin());
                T sum = 0;
                for(size_t i = data_.size(); i > 0; i--) {
                    sum += data_[i - 1] * (T)i;
                }
                output_value = (sum * 2.0d) / ((T)period * ((T)period + 1.0d));
                return common::OK;
            }
            return common::INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &out) noexcept {
            const int err = update(in);
            out = output_value;
            return err;
        }

        /** \brief Протестировать индикатор
         *
         * Данный метод отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in) noexcept {
            if(period == 0) {
                output_value = in;
                return common::NO_INIT;
            }
            std::vector<T> _data = data_;
            if(_data.size() < (size_t)period) {
                _data.push_back(in);
                if(_data.size() == (size_t)period) {
                    T sum = 0;
                    for(size_t i = _data.size(); i > 0; i--) {
                        sum += _data[i - 1] * (T)i;
                    }
                    output_value = (sum * 2.0d) / ((T)period * ((T)period + 1.0d));
                    return common::OK;
                }
            } else {
                _data.push_back(in);
                _data.erase(_data.begin());
                T sum = 0;
                for(size_t i = data_.size(); i > 0; i--) {
                    sum += data_[i - 1] * (T)i;
                }
                output_value = (sum * 2.0d) / ((T)period * ((T)period + 1.0d));
                return common::OK;
            }
            return common::INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Протестировать индикатор
         *
         * Данный метод отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in    Сигнал на входе
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &out) noexcept {
            const int err = test(in);
            out = output_value;
            return err;
        }

        /** \brief Получить значение индикатора
         * \return Значение индикатора
         */
        inline T get() const noexcept {
            return output_value;
        }

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            data_.clear();
            output_value = std::numeric_limits<T>::quiet_NaN();
        }
    };

    /** \brief Экспоненциально взвешенное скользящее среднее
     */
    template <class T>
    class EMA {
    protected:
        std::vector<T> data_;
        T last_data_;
        T a;
        size_t period_ = 0;
        T output_value = std::numeric_limits<T>::quiet_NaN();
    public:
        EMA() {};

        /** \brief Инициализировать экспоненциально взвешенное
         * скользящее среднее
         * \param period период
         */
        EMA(const size_t period) : period_(period) {
            data_.reserve(period_);
            a = 2.0/(T)(period_ + 1.0d);
        }

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        virtual int update(const T in) noexcept {
            if(period_ == 0) {
                output_value = in;
                return common::NO_INIT;
            }
            if(data_.size() < (size_t)period_) {
                data_.push_back(in);
                if(data_.size() == (size_t)period_) {
                    T sum = std::accumulate(data_.begin(), data_.end(), T(0));
                    last_data_ = sum / (T)period_;
                }
            } else {
                last_data_ = a * in + (1.0 - a) * last_data_;
                output_value = last_data_;
                return common::OK;
            }
            return common::INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        virtual int update(const T in, T &out) noexcept {
            const int err = update(in);
            out = output_value;
            return err;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем, что не влияет на внутреннее
         * состояние индикатора
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in) noexcept {
            if(period_ == 0) {
                output_value = in;
                return common::NO_INIT;
            }
            if(data_.size() == period_) {
                output_value = a * in + (1.0 - a) * last_data_;
                return common::OK;
            }
            return common::INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем, что не влияет на внутреннее
         * состояние индикатора
         * \param in    Сигнал на входе
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in, T &out) noexcept {
            const int err = test(in);
            out = output_value;
            return err;
        }

        /** \brief Получить значение индикатора
         * \return Значение индикатора
         */
        virtual inline T get() const noexcept {
            return output_value;
        }

        /** \brief Очистить данные индикатора
         */
        virtual inline void clear() noexcept {
            data_.clear();
            output_value = std::numeric_limits<T>::quiet_NaN();
        }
    };

    /** \brief Модифицированное скользящее среднее
     */
    template <class T>
    class MMA : public EMA<T> {
    public:
        MMA() {};

        /** \brief Инициализировать модифицированное скользящее среднее
         * \param period период
         */
        MMA(const size_t period) {
            EMA<T>::period_ = period;
            EMA<T>::data_.reserve(period);
            EMA<T>::a = 1.0/(T)period;
        }
    };

    /** \brief Индикатор Volume Weighted MA - модифицированная скользящая средняя, с реализацией взвешенности по объему.
     */
    template <typename T>
    class VWMA {
    private:
        xtechnical::circular_buffer<T> price_buffer;
        xtechnical::circular_buffer<T> weight_buffer;
        size_t period = 0;
        T output_value = std::numeric_limits<T>::quiet_NaN();
    public:
        VWMA() {};

        /** \brief Инициализировать скользящее среднее
         * \param period    Период
         */
        VWMA(const size_t p) : price_buffer(p), weight_buffer(p), period(p) {

        }

        /** \brief Обновить состояние индикатора
         * \param input     Сигнал на входе
         * \param weight    Вес сигнала на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T input, const T weight) noexcept {
            if(period == 0) return common::NO_INIT;
            price_buffer.update(input);
            weight_buffer.update(weight);
            if (!weight_buffer.full()) return common::INDICATOR_NOT_READY_TO_WORK;
            std::vector<T> price_data(price_buffer.to_vector());
            std::vector<T> weight_data(weight_buffer.to_vector());
            T sum = 0;
            T sum_weight = 0;
            for(size_t i = 0; i < price_data.size(); ++i) {
                sum += price_data[i] * weight_data[i];
                sum_weight += weight_data[i];
            }
            if(sum_weight == 0) {
                sum = 0;
                for(size_t i = 0; i < price_data.size(); ++i) {
                    sum += price_data[i];
                }
                output_value = sum / (T)period;
            } else {
                output_value = sum / (sum_weight);
            }
            return common::OK;
        }

        /** \brief Обновить состояние индикатора
         * \param input     Сигнал на входе
         * \param weight    Вес сигнала на входе
         * \param output    Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T input, const T weight, T &output) noexcept {
            const int err = update(input, weight);
            output = output_value;
            return err;
        }

        /** \brief Протестировать индикатор
         *
         * Данный метод отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param input     Сигнал на входе
         * \param weight    Вес сигнала на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
         int test(const T input, const T weight) noexcept {
            if(period == 0) return common::NO_INIT;
            price_buffer.test(input);
            weight_buffer.test(weight);
            if (!weight_buffer.full()) return common::INDICATOR_NOT_READY_TO_WORK;
            std::vector<T> price_data(price_buffer.to_vector());
            std::vector<T> weight_data(weight_buffer.to_vector());
            T sum = 0;
            T sum_weight = 0;
            for(size_t i = 0; i < price_data.size(); ++i) {
                sum += price_data[i] * weight_data[i];
                sum_weight += weight_data[i];
            }
            if(sum_weight == 0) {
                sum = 0;
                for(size_t i = 0; i < price_data.size(); ++i) {
                    sum += price_data[i];
                }
                output_value = sum / (T)period;
            } else {
                output_value = sum / (sum_weight);
            }
            return common::OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данный метод отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param input     Сигнал на входе
         * \param weight    Вес сигнала на входе
         * \param output    Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T input, const T weight, T &output) noexcept {
            const int err = test(input, weight);
            output = output_value;
            return err;
        }

        /** \brief Получить значение индикатора
         * \return Значение индикатора
         */
        inline T get() const noexcept {
            return output_value;
        }

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            price_buffer.clear();
            weight_buffer.clear();
            output_value = std::numeric_limits<T>::quiet_NaN();
        }
    };

    /** \brief Класс фильтра низкой частоты
     */
    template <typename T>
    class LowPassFilter {
    private:
        T alfa_ = 0;
        T beta_ = 0;
        T prev_ = 0;
        T tranTime = 0;
        bool is_update_ = false;
        bool is_init_ = false;
    public:

        LowPassFilter() {};

        /** \brief Инициализация фильтра низкой частоты
         * \param dt время переходного процесса
         * \param period период дискретизации
         * \param error_signal ошибка сигнала
         */
        LowPassFilter(
                const T dt,
                const T period = 1.0,
                const T error_signal = 0.03) {
            T N = dt / period;
            T Ntay = std::log(1.0 / error_signal);
            alfa_ = std::exp(-Ntay / N);
            beta_ = 1.0 - alfa_;
            is_init_ = true;
        }

        /** \brief Получить новые данные индикатора
         * \param in    Сигнал на входе
         * \param out   Массив на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in, T &out) {
            if(!is_init_) return common::NO_INIT;
            if (!is_update_) {
                prev_ = in;
                is_update_ = true;
                out = in;
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            out = alfa_ * prev_ + beta_ * in;
            prev_ = out;
            return common::OK;
        }

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in) {
            if(!is_init_) return common::NO_INIT;
            if (!is_update_) {
                prev_ = in;
                is_update_ = true;
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            prev_ = alfa_ * prev_ + beta_ * in;
            return common::OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &out) {
            if(!is_init_) {
                return common::NO_INIT;
            }
            if (!is_init_) {
                out = in;
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            out = alfa_ * prev_ + beta_ * in;
            return common::OK;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            is_update_ = false;
        }
    };

    /** \brief Относительная Величина Изменения Цены
     * Значение Percent Rank – это сумма значений за выбранный период,
     * которые меньше текущего значения,
     * деленное на общее количество значений за данный период.
     */
    template <typename T>
    class PercentRank {
    private:
        PercentDifference<T> percent_diff;
        xtechnical::circular_buffer<T> buffer;
        T output_value = std::numeric_limits<T>::quiet_NaN();
        bool mode_offset = false;
    public:
        PercentRank() {};

        PercentRank(const size_t p, const bool use_offset) :
            percent_diff(1), buffer(p), mode_offset(use_offset) {
        };

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in) noexcept {
            percent_diff.update(in);
            if (std::isnan(percent_diff.get())) return common::INDICATOR_NOT_READY_TO_WORK;
            const T diff = percent_diff.get();
            if (!mode_offset) buffer.update(diff);
            if (buffer.full()) {
                std::vector<T> temp(buffer.to_vector());
                T counter = 0;
                for (size_t i = 0; i < temp.size(); ++i) {
                    if (temp[i] <= diff) counter += 1;
                }
                if (mode_offset) buffer.update(diff);
                if (temp.size() == 0) output_value = 0;
                else output_value = (counter / (T)temp.size()) * 100;
                return common::OK;
            }
            if (mode_offset) buffer.update(diff);
            return common::INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \param out   Массив на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in, T &out) noexcept {
            const int err = update(in);
            out = output_value;
            return err;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем, что не влияет на внутреннее состояние индикатора
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in) noexcept {
            percent_diff.test(in);
            if (std::isnan(percent_diff.get())) return common::INDICATOR_NOT_READY_TO_WORK;
            const T diff = percent_diff.get();
            if (!mode_offset) buffer.test(diff);
            if (buffer.full()) {
                std::vector<T> temp(buffer.to_vector());
                T counter = 0;
                for (size_t i = 0; i < temp.size(); ++i) {
                    if (temp[i] <= diff) counter += 1;
                }
                if (mode_offset) buffer.test(diff);
                if (temp.size() == 0) output_value = 0;
                else output_value = (counter / (T)temp.size()) * 100;
                return common::OK;
            }
            if (mode_offset) buffer.test(diff);
            return common::INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем, что не влияет на внутреннее состояние индикатора
         * \param in    Сигнал на входе
         * \param out   Массив на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in, T &out) noexcept {
            const int err = test(in);
            out = output_value;
            return err;
        }

        /** \brief Получить значение индикатора
         * \return Значение индикатора
         */
        inline T get() const noexcept {
            return output_value;
        }

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            percent_diff.clear();
            buffer.clear();
            output_value = std::numeric_limits<T>::quiet_NaN();
        }
    };

    /** \brief ConnorsRSI
     * Данный индикатор был разработан Ларри Коннорсом из Connors Research
     * его доклад вы легко сможете найти в интернете по запросу
     * "Connors Research Trading Strategy Series An Introduction to ConnorsRSI"
     */
    template <typename T, class MA_TYPE>
    class CRSI {
    private:
        RSI<T, MA_TYPE> rsi_0;
        RSI<T, MA_TYPE> rsi_1;
        PercentRank<T> percent_rank;
        DelayLine<T> delay_line;
        T streak = 0;
        T weight_a = 1, weight_b = 1, weight_c = 1;
        T sum_weight = 3;
        T output_value = std::numeric_limits<T>::quiet_NaN();
    public:
        CRSI() {};

        /** \brief Конструктор ConnorsRSI
         * \param rsi_period
         * \param streak_period
         * \param percent_rank_period
         * \param percent_rank_mode             Режим работы индикатора Percent Rank (новое значение входит в буфер, или нет)
         * \param weight_rsi_period             Вес
         * \param weight_streak_period          Вес
         * \param weight_percent_rank_period    Вес
         */
        CRSI(
                const size_t rsi_period,
                const size_t streak_period,
                const size_t percent_rank_period,
                const bool percent_rank_mode = false,
                const T weight_rsi_period = 1.0,
                const T weight_streak_period = 1.0,
                const T weight_percent_rank_period = 1.0) :
            rsi_0(rsi_period), rsi_1(streak_period),
            percent_rank(percent_rank_period, percent_rank_mode),
            delay_line(1),
            weight_a(weight_rsi_period), weight_b(weight_streak_period),
            weight_c(weight_percent_rank_period) {
            sum_weight = weight_a + weight_b + weight_c;
        }

        /** \brief Обновить состояние индикатора
         * \param price Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T price) {
            delay_line.update(price);
            rsi_0.update(price);
            percent_rank.update(price);
            if (std::isnan(delay_line.get())) return common::INDICATOR_NOT_READY_TO_WORK;
            const T diff = price - delay_line.get();
            if (combined_tolerance_compare(price, delay_line.get())) {
                streak = 0;
            } else {
                const T diff = price - delay_line.get();
                streak = diff > 0 ? (streak > 0 ? (streak + 1) : 0) : (streak < 0 ? (streak - 1) : 0);
            }
            rsi_1.update(streak);
            if (std::isnan(percent_rank.get()) ||
                std::isnan(rsi_0.get()) ||
                std::isnan(rsi_1.get())) return common::INDICATOR_NOT_READY_TO_WORK;
            if (sum_weight == 0) return common::NO_INIT;
            output_value = (weight_a * rsi_0.get() + weight_b * rsi_1.get() +
                weight_c * percent_rank.get()) / sum_weight;
            return common::OK;
        }

        /** \brief Обновить состояние индикатора
         * \param price Сигнал на входе
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T price, T &out) {
            const int err = update(price);
            out = output_value;
            return err;
        }

        /** \brief Протестировать индикатор
         * \param price Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T price) {
            delay_line.test(price);
            rsi_0.test(price);
            percent_rank.test(price);
            if (std::isnan(delay_line.get())) return common::INDICATOR_NOT_READY_TO_WORK;
            const T diff = price - delay_line.get();
            T save_streak = streak;
            if (combined_tolerance_compare(price, delay_line.get())) {
                save_streak = 0;
            } else {
                const T diff = price - delay_line.get();
                save_streak = diff > 0 ? (save_streak > 0 ? (save_streak + 1) : 0) : (save_streak < 0 ? (save_streak - 1) : 0);
            }
            rsi_1.test(save_streak);
            if (std::isnan(percent_rank.get()) ||
                std::isnan(rsi_0.get()) ||
                std::isnan(rsi_1.get())) return common::INDICATOR_NOT_READY_TO_WORK;
            if (sum_weight == 0) return common::NO_INIT;
            output_value = (weight_a * rsi_0.get() + weight_b * rsi_1.get() +
                weight_c * percent_rank.get()) / sum_weight;
            return common::OK;
        }

        /** \brief Протестировать индикатор
         * \param price Сигнал на входе
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T price, T &out) {
            const int err = test(price);
            out = output_value;
            return err;
        }

        /** \brief Получить значение индикатора
         * \return Значение индикатора
         */
        inline T get() const noexcept {
            return output_value;
        }

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            rsi_0.clear();
            rsi_1.clear();
            percent_rank.clear();
            delay_line.clear();
            output_value = std::numeric_limits<T>::quiet_NaN();
        }
    };

    /** \brief Среднее абсолютное отклонение
     */
    template <typename T, class MA_TYPE>
    class MAD {
    private:
        MA_TYPE ma;
        xtechnical::circular_buffer<T> buffer;
        T output_value = std::numeric_limits<T>::quiet_NaN();
    public:

        MAD() {};

        MAD(const size_t p) :
            ma(p), buffer(p) {
        };

        int update(const T in) {
            buffer.update(in);
            ma.update(in);
            if (!buffer.full()) return common::INDICATOR_NOT_READY_TO_WORK;
            if (std::isnan(ma.get())) return common::INDICATOR_NOT_READY_TO_WORK;
            std::vector<T> temp(buffer.to_vector());
            T sum = 0;
            for (size_t  i = 0; i < temp.size(); ++i) {
                sum += std::abs(temp[i] - ma.get());
            }
            output_value = sum / (T)temp.size();
            return common::OK;
        }

        int update(const T in, T &out) {
            const int err = update(in);
            out = output_value;
            return err;
        }

        int test(const T in) {
            buffer.test(in);
            ma.test(in);
            if (!buffer.full()) return common::INDICATOR_NOT_READY_TO_WORK;
            if (std::isnan(ma.get())) return common::INDICATOR_NOT_READY_TO_WORK;
            const std::vector<T> temp(buffer.to_vector());
            T sum = 0;
            for (size_t  i = 0; i < temp.size(); ++i) {
                sum += std::abs(temp[i] - ma.get());
            }
            output_value = sum / (T)temp.size();
            return common::OK;
        }

        int test(const T in, T &out) {
            const int err = test(in);
            out = output_value;
            return err;
        }

        /** \brief Получить значение индикатора
         * \return Значение индикатора
         */
        inline T get() const noexcept {
            return output_value;
        }

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            ma.clear();
            buffer.clear();
            output_value = std::numeric_limits<T>::quiet_NaN();
        }
    };

#if(0)
    /** \brief Индекс товарного канала
     */
    template <typename T, class MA_TYPE>
    class CCI {
    private:
        MA_TYPE ma;
        xtechnical::circular_buffer<T> buffer;
        T output_value = std::numeric_limits<T>::quiet_NaN();
    public:

        CCI() {};

        CCI(const size_t p) :
            ma(p), buffer(p) {
        };

        int update(const T in) {
            buffer.update(in);
            ma.update(in);
            if (!buffer.full()) return common::INDICATOR_NOT_READY_TO_WORK;
            if (std::isnan(ma.get())) return common::INDICATOR_NOT_READY_TO_WORK;
            std::vector<T> temp(buffer.to_vector());
            T sum = 0;
            for (size_t  i = 0; i < temp.size(); ++i) {
                sum += std::abs(temp[i] - ma.get());
            }
            const T mad = sum / (T)temp.size();
            output_value = (in - ma.get()) / (0.015 * mad);
            return common::OK;
        }

        int update(const T high, const T low, const T close) {
            const T in = (high + low + close) / 3.0d;
            return update(in);
        }

        int update(const T in, T &out) {
            const int err = update(in);
            out = output_value;
            return err;
        }

        int update(const T high, const T low, const T close, T &out) {
            const T in = (high + low + close) / 3.0d;
            const int err = update(in);
            out = output_value;
            return err;
        }

        int test(const T in) {
            buffer.test(in);
            ma.test(in);
            if (!buffer.full()) return common::INDICATOR_NOT_READY_TO_WORK;
            if (std::isnan(ma.get())) return common::INDICATOR_NOT_READY_TO_WORK;
            const std::vector<T> temp(buffer.to_vector());
            T sum = 0;
            for (size_t  i = 0; i < temp.size(); ++i) {
                sum += std::abs(temp[i] - ma.get());
            }
            const T mad = sum / (T)temp.size();
            output_value = (in - ma.get()) / (0.015 * mad);
            return common::OK;
        }

        int test(const T high, const T low, const T close) {
            const T in = (high + low + close) / 3.0d;
            return test(in);
        }

        int test(const T in, T &out) {
            const int err = test(in);
            out = output_value;
            return err;
        }

        int test(const T high, const T low, const T close, T &out) {
            const T in = (high + low + close) / 3.0d;
            const int err = test(in);
            out = output_value;
            return err;
        }

        /** \brief Получить значение индикатора
         * \return Значение индикатора
         */
        inline T get() const noexcept {
            return output_value;
        }

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            ma.clear();
            buffer.clear();
            output_value = std::numeric_limits<T>::quiet_NaN();
        }
    };
#endif

    /** \brief Линии Боллинджера
     */
    template <typename T, class MA_TYPE = SMA<T>>
    class BollingerBands {
    private:
        xtechnical::circular_buffer<T> buffer;
        MA_TYPE ma;
        DelayLine<T> delay_line;
        size_t period = 0;
        double deviations = 0;
        T output_tl = std::numeric_limits<T>::quiet_NaN();
        T output_ml = std::numeric_limits<T>::quiet_NaN();
        T output_bl = std::numeric_limits<T>::quiet_NaN();
        T output_std_dev = std::numeric_limits<T>::quiet_NaN();
    public:

        BollingerBands() {};

        /** \brief Инициализация линий Боллинджера
         * \param p Период
         * \param d Множитель стандартного отклонения
         * \param o Смещение назад
         */
        BollingerBands(const size_t p, const size_t d, const size_t o = 0) :
                buffer(p), ma(p), delay_line(o),
                period(p), deviations(d) {
        }

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in) noexcept {
            if(period == 0) {
                output_tl = std::numeric_limits<T>::quiet_NaN();
                output_ml = std::numeric_limits<T>::quiet_NaN();
                output_bl = std::numeric_limits<T>::quiet_NaN();
                output_std_dev = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }
            if(delay_line.update(in) != common::OK) {
                output_tl = std::numeric_limits<T>::quiet_NaN();
                output_ml = std::numeric_limits<T>::quiet_NaN();
                output_bl = std::numeric_limits<T>::quiet_NaN();
                output_std_dev = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            buffer.update(delay_line.get());
            ma.update(delay_line.get());
            if(buffer.full() && !std::isnan(ma.get())) {
                std::vector<T> data = buffer.to_vector();
                output_ml = ma.get();
                T sum = 0;
                for (size_t i = 0; i < period; ++i) {
                    const T diff = (data[i] - output_ml);
                    sum += diff * diff;
                }
                output_std_dev = std::sqrt(sum / (T)(period - 1));
                const T std_dev_offset = output_std_dev * deviations;
                output_tl = std_dev_offset + output_ml;
                output_bl = output_ml - std_dev_offset;
            } else {
                output_tl = std::numeric_limits<T>::quiet_NaN();
                output_ml = std::numeric_limits<T>::quiet_NaN();
                output_bl = std::numeric_limits<T>::quiet_NaN();
                output_std_dev = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            return common::OK;
        }

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \param tl    Верхняя полоса боллинджера
         * \param ml    Среняя полоса боллинджера
         * \param bl    Нижняя полоса боллинджера
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &tl, T &ml, T &bl) noexcept {
            const int err = update(in);
            tl = output_tl;
            ml = output_ml;
            bl = output_bl;
            return err;
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param ml среняя полоса боллинджера
         * \param std_dev стандартное отклонение
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &ml, T &std_dev) noexcept {
            const int err = update(in);
            ml = output_ml;
            std_dev = output_std_dev;
            return err;
        }

        /** \brief Протестировать индикатор
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in) noexcept {
            if(period == 0) {
                output_tl = std::numeric_limits<T>::quiet_NaN();
                output_ml = std::numeric_limits<T>::quiet_NaN();
                output_bl = std::numeric_limits<T>::quiet_NaN();
                output_std_dev = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }
            if(delay_line.test(in) != common::OK) {
                output_tl = std::numeric_limits<T>::quiet_NaN();
                output_ml = std::numeric_limits<T>::quiet_NaN();
                output_bl = std::numeric_limits<T>::quiet_NaN();
                output_std_dev = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            buffer.test(delay_line.get());
            ma.test(delay_line.get());
            if(buffer.full() && !std::isnan(ma.get())) {
                std::vector<T> data = buffer.to_vector();
                output_ml = ma.get();
                T sum = 0;
                for (size_t i = 0; i < period; ++i) {
                    const T diff = (data[i] - output_ml);
                    sum += diff * diff;
                }
                output_std_dev = std::sqrt(sum / (T)(period - 1));
                const T std_dev_offset = output_std_dev * deviations;
                output_tl = std_dev_offset + output_ml;
                output_bl = output_ml - std_dev_offset;
            } else {
                output_tl = std::numeric_limits<T>::quiet_NaN();
                output_ml = std::numeric_limits<T>::quiet_NaN();
                output_bl = std::numeric_limits<T>::quiet_NaN();
                output_std_dev = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            return common::OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in    Сигнал на входе
         * \param tl    Верхняя полоса боллинджера
         * \param ml    Среняя полоса боллинджера
         * \param bl    Нижняя полоса боллинджера
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in, T &tl, T &ml, T &bl) noexcept {
            const int err = test(in);
            tl = output_tl;
            ml = output_ml;
            bl = output_bl;
            return err;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in        Сигнал на входе
         * \param ml        Среняя полоса боллинджера
         * \param std_dev   Стандартное отклонение
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in, T &ml, T &std_dev) noexcept {
            const int err = test(in);
            ml = output_ml;
            std_dev = output_std_dev;
            return err;
        }

        inline T get_tl() const noexcept {return output_tl;};
        inline T get_ml() const noexcept {return output_ml;};
        inline T get_bl() const noexcept {return output_bl;};
        inline T get_std_dev() const noexcept {return output_std_dev;};

        /** \brief Очистить данные индикатора
         */
        void clear() noexcept {
            buffer.clear();
            ma.clear();
            delay_line.clear();
            output_tl = std::numeric_limits<T>::quiet_NaN();
            output_ml = std::numeric_limits<T>::quiet_NaN();
            output_bl = std::numeric_limits<T>::quiet_NaN();
            output_std_dev = std::numeric_limits<T>::quiet_NaN();
        }
    };

#if(0)
    /** \brief Линии Боллинджера
     */
    template <typename T>
    class BollingerBands {
    private:
        std::vector<T> data_;
        size_t period_ = 0;
        T d_;
    public:
        BollingerBands() {};

        /** \brief Инициализация линий Боллинджера
         * \param period период  индикатора
         * \param factor множитель стандартного отклонения
         */
        BollingerBands(const size_t &period, const T &factor) {
            period_ = period;
            d_ = factor;
        }

        /** \brief Инициализация линий Боллинджера
         * \param period период  индикатора
         * \param factor множитель стандартного отклонения
         */
        void init(const size_t &period, const T &factor) {
            period_ = period;
            d_ = factor;
            data_.clear();
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param tl верхняя полоса боллинджера
         * \param ml среняя полоса боллинджера
         * \param bl нижняя полоса боллинджера
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in, T &tl, T &ml, T &bl) {
            if(period_ == 0) {
                tl = in;
                ml = in;
                bl = in;
                return NO_INIT;
            }
            if(data_.size() < (size_t)period_) {
                data_.push_back(in);
                if(data_.size() != (size_t)period_) {
                    tl = in;
                    ml = in;
                    bl = in;
                    return INDICATOR_NOT_READY_TO_WORK;
                }
            } else {
                data_.push_back(in);
                data_.erase(data_.begin());
            }
            ml = std::accumulate(data_.begin(), data_.end(), T(0));
            ml /= (T)period_;
            T sum = 0;
            for (size_t i = 0; i < period_; ++i) {
                T diff = (data_[i] - ml);
                sum +=  diff * diff;
            }
            T std_dev = std::sqrt(sum / (T)(period_ - 1));
            tl = std_dev * d_ + ml;
            bl = ml - std_dev * d_;
            return OK;
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param ml среняя полоса боллинджера
         * \param std_dev стандартное отклонение
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in, T &ml, T &std_dev) {
            if(period_ == 0) {
                ml = 0;
                std_dev = 0;
                return NO_INIT;
            }
            if(data_.size() < period_) {
                data_.push_back(in);
                if(data_.size() != period_) {
                    ml = 0;
                    std_dev = 0;
                    return INDICATOR_NOT_READY_TO_WORK;
                }
            } else {
                data_.push_back(in);
                data_.erase(data_.begin());
            }
            ml = std::accumulate(data_.begin(), data_.end(), T(0));
            ml /= (T)period_;
            T sum = 0;
            for (size_t i = 0; i < period_; ++i) {
                T diff = (data_[i] - ml);
                sum +=  diff * diff;
            }
            std_dev = std::sqrt(sum / (T)(period_ - 1));
            return OK;
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in) {
            if(period_ == 0) {
                return NO_INIT;
            }
            if(data_.size() < period_) {
                data_.push_back(in);
                if(data_.size() != period_) {
                    return INDICATOR_NOT_READY_TO_WORK;
                }
            } else {
                data_.push_back(in);
                data_.erase(data_.begin());
            }
            return OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \param tl верхняя полоса боллинджера
         * \param ml среняя полоса боллинджера
         * \param bl нижняя полоса боллинджера
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in, T &tl, T &ml, T &bl) {
            if(period_ == 0) {
                tl = in;
                ml = in;
                bl = in;
                return NO_INIT;
            }
            std::vector<T> data_test = data_;
            if(data_test.size() < period_) {
                data_test.push_back(in);
                if(data_test.size() != period_) {
                    tl = in;
                    ml = in;
                    bl = in;
                    return INDICATOR_NOT_READY_TO_WORK;
                }
            } else {
                data_test.push_back(in);
                data_test.erase(data_test.begin());
            }
            ml = std::accumulate(data_test.begin(), data_test.end(), T(0));
            ml /= (T)period_;
            T sum = 0;
            for (size_t i = 0; i < period_; ++i) {
                T diff = (data_test[i] - ml);
                sum +=  diff * diff;
            }
            T std_dev = std::sqrt(sum / (T)(period_ - 1));
            tl = std_dev * d_ + ml;
            bl = ml - std_dev * d_;
            return OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \param ml среняя полоса боллинджера
         * \param std_dev стандартное отклонение
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in, T &ml, T &std_dev) {
            if(period_ == 0) {
                ml = in;
                std_dev = 0;
                return NO_INIT;
            }
            std::vector<T> data_test = data_;
            if(data_test.size() < period_) {
                data_test.push_back(in);
                if(data_test.size() != period_) {
                    ml = 0;
                    std_dev = 0;
                    return INDICATOR_NOT_READY_TO_WORK;
                }
            } else {
                data_test.push_back(in);
                data_test.erase(data_test.begin());
            }
            ml = std::accumulate(data_test.begin(), data_test.end(), T(0));
            ml /= (T)period_;
            T sum = 0;
            for (size_t i = 0; i < period_; ++i) {
                T diff = (data_test[i] - ml);
                sum +=  diff * diff;
            }
            std_dev = std::sqrt(sum / (T)(period_ - 1));
            return OK;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            data_.clear();
        }
    };
#endif

    /** \brief Обработать массив данных боллинджером по кругу
     */
    template <class T1, class T2>
    int calc_ring_bollinger(
            T1 &in,
            T2 &tl,
            T2 &ml,
            T2 &bl,
            const size_t &period,
            const double &std_dev_factor) {
        size_t input_size = in.size();
        size_t tl_size = tl.size();
        if( input_size == 0 || input_size < period ||
            tl_size != bl.size() || tl_size != input_size ||
            ml.size() != input_size) return common::INVALID_PARAMETER;
        using NumType = typename T1::value_type;
        BollingerBands<NumType> iBB(period, std_dev_factor);
        for(size_t i = input_size - period; i < input_size; ++i) {
            iBB.update(in[i]);
        }
        for(size_t i = 0; i < input_size; ++i) {
            iBB.update(in[i], tl[i], ml[i], bl[i]);
        }
        return common::OK;
    }

    /** \brief Средняя скорость
     */
    template <typename T>
    class AverageSpeed {
    private:
        MW<T> iMW;
        bool is_init_ = false;
    public:
        AverageSpeed() {};

        /** \brief Инициализировать класс индикатора
         * \param period период индикатора
         */
        AverageSpeed(const size_t &period) : iMW(period + 1) {
            is_init_ = true;
        }

        /** \brief Обновить состояние индикатора
         * \param in Цена
         * \param out Значение индикатора
         * \return вернет 0 в случае успеха
         */
        int update(const T &in, T &out) {
            if(!is_init_) return common::NO_INIT;
            std::vector<T> mw_out;
            int err = iMW.update(in, mw_out);
            if(err == common::OK) {
                std::vector<T> mw_diff(mw_out.size());
                normalization::calculate_difference(mw_out, mw_diff);
                T sum = std::accumulate(mw_diff.begin(), mw_diff.end(), T(0));
                out = sum /(T)mw_diff.size();
                return common::OK;
            }
            return err;
        }

        /** \brief Протестировать индикатор
         * \param in Цена
         * \param out Значение индикатора
         * \return вернет 0 в случае успеха
         */
        int test(const T &in, T &out) {
            if(!is_init_) return common::NO_INIT;
            std::vector<T> mw_out;
            int err = iMW.test(in, mw_out);
            if(err == common::OK) {
                std::vector<T> mw_diff(mw_out.size());
                normalization::calculate_difference(mw_out, mw_diff);
                T sum = std::accumulate(mw_diff.begin(), mw_diff.end(), T(0));
                out = sum /(T)mw_diff.size();
                return common::OK;
            }
            return err;
        }

        /** \brief Очистить состояние индикатора
         */
        void clear() {
            iMW.clear();
        }
    };

    /** \brief Экспериментальный индикатор, не применять!
     */
    template <typename T>
    class DetectorWaveform {
    private:
        MW<T> iMW;
        const size_t MIN_WAVEFORM_LEN = 3;
        T coeff_exp = 3.141592;
        std::vector<std::vector<T>> exp_data_up_;
        std::vector<std::vector<T>> exp_data_dn_;

        void init_exp_data_up(std::vector<T> &data) {
            T dt = 1.0/(T)data.size();
            for(size_t i = 0; i < data.size(); ++i) {
                data[i] = exp(coeff_exp*(T)i*dt);
            }
            normalization::calculate_min_max(
                data,
                data,
                common::MINMAX_UNSIGNED);
        }

        void init_exp_data_dn(std::vector<T> &data) {
            T dt = 1.0/(T)data.size();
            for(size_t i = 0; i < data.size(); ++i) {
                data[i] = -exp(coeff_exp*(T)i*dt);
            }
            normalization::calculate_min_max(
                data,
                data,
                common::MINMAX_UNSIGNED);
        }
    public:
        /** \brief Инициализировать класс
         * \param max_len максимальная длина файла
         */
        DetectorWaveform(const int max_len) : iMW(max_len) {
            if(max_len < MIN_WAVEFORM_LEN) return;
            size_t max_num_exp_data = max_len - MIN_WAVEFORM_LEN + 1;
            exp_data_up_.resize(max_num_exp_data);
            exp_data_dn_.resize(max_num_exp_data);
            for(size_t l = MIN_WAVEFORM_LEN; l <= max_len; ++l) {
                exp_data_up_[l-MIN_WAVEFORM_LEN].resize(l);
                exp_data_dn_[l-MIN_WAVEFORM_LEN].resize(l);
                init_exp_data_up(exp_data_up_[l-MIN_WAVEFORM_LEN]);
                init_exp_data_dn(exp_data_dn_[l-MIN_WAVEFORM_LEN]);
            }
        }

        int update(T in, T &out, const int len_waveform) {
            std::vector<T> mw_out;
            int err = iMW.update(in, mw_out);
            if(err == common::OK) {
                if(mw_out.size() >= MIN_WAVEFORM_LEN &&
                    len_waveform <= mw_out.size()) {
                    std::vector<T> fragment_data;
                    fragment_data.insert(
                        fragment_data.begin(),
                        mw_out.begin() + mw_out.size() - len_waveform,
                        mw_out.end());
                    int err_n = normalization::calculate_min_max(
                        fragment_data,
                        fragment_data,
                        common::MINMAX_UNSIGNED);
                    if(err_n != common::OK) return err_n;
                    T coeff_up = 0, coeff_dn = 0;
                    int err_up = correlation::calculate_spearman_rank_correlation_coefficient(
                        fragment_data,
                        exp_data_up_[len_waveform-MIN_WAVEFORM_LEN],
                        coeff_up);

                    int err_dn = correlation::calculate_spearman_rank_correlation_coefficient(
                        fragment_data,
                        exp_data_dn_[len_waveform-MIN_WAVEFORM_LEN],
                        coeff_dn);
                    if(err_up != common::OK) return err_up;
                    if(err_dn != common::OK) return err_dn;
                    if(abs(coeff_up) > abs(coeff_dn)) {
                        out = coeff_up;
                    } else {
                        out = coeff_dn;
                    }
                    return common::OK;
                }
            }
            return err;
        }

        void clear() {
            iMW.clear();
        }
    };

    /** \brief Класс для подсчета коррлеяции между валютными парами
     */
    template <typename T>
    class CurrencyCorrelation {
    private:
        std::vector<std::vector<T>> data_;
        std::vector<std::vector<T>> data_test_;
        size_t period_ = 0;
        bool is_test_ = false;
    public:
        enum CorrelationType {
            SPEARMAN_RANK = 0,
            PEARSON = 1,
        };
        /** \brief Инициализировать индикатор
         * \param period период индикатора
         * \param num_symbols колючество валютных пар
         */
        CurrencyCorrelation(const size_t &period, const size_t &num_symbols) {
            data_.resize(num_symbols);
            data_test_.resize(num_symbols);
            period_ = period;
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param num_symbol номер валютной пары
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in, const size_t &num_symbol) {
            is_test_ = false;
            if(period_ == 0) {
                return common::NO_INIT;
            }
            if(data_[num_symbol].size() < period_) {
                data_[num_symbol].push_back(in);
                if(data_[num_symbol].size() == period_) {
                    return common::OK;
                }
            } else {
                data_[num_symbol].push_back(in);
                data_[num_symbol].erase(data_[num_symbol].begin());
                return common::OK;
            }
            return common::INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param num_symbol номер валютной пары
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in, const size_t &num_symbol) {
            is_test_ = true;
            if(period_ == 0) {
                return common::NO_INIT;
            }
            data_test_ = data_;
            if(data_test_[num_symbol].size() < period_) {
                data_test_[num_symbol].push_back(in);
                if(data_test_[num_symbol].size() == period_) {
                    return common::OK;
                }
            } else {
                data_test_[num_symbol].push_back(in);
                data_test_[num_symbol].erase(data_test_[num_symbol].begin());
                return common::OK;
            }
            return common::INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Посчитать корреляцию между двумя валютными парами
         * \param out значение корреляции
         * \param num_symbol_1 номер первой валютной пары
         * \param num_symbol_2 номер второй валютной пары
         * \param correlation_type тип корреляции (SPEARMAN_RANK, PEARSON)
         * \return состояние ошибки, 0 в случае успеха
         */
        int calculate_correlation(
                T &out,
                const size_t &num_symbol_1,
                const size_t &num_symbol_2,
                const size_t &correlation_type = SPEARMAN_RANK) {
            std::vector<T> norm_vec_1(period_), norm_vec_2(period_);
            if(is_test_) {
                if(data_test_[num_symbol_1].size() == (size_t)period_ &&
                    data_test_[num_symbol_2].size() == (size_t)period_) {
                    if(correlation_type == SPEARMAN_RANK) {
                        normalization::calculate_min_max(
                            data_test_[num_symbol_1],
                            norm_vec_1,
                            common::MINMAX_SIGNED);
                        normalization::calculate_min_max(
                            data_test_[num_symbol_2],
                            norm_vec_2,
                            common::MINMAX_SIGNED);
                        return correlation::calculate_spearman_rank_correlation_coefficient(
                            norm_vec_1,
                            norm_vec_2,
                            out);
                    } else
                    if(correlation_type == PEARSON) {
                        normalization::calculate_min_max(
                            data_test_[num_symbol_1],
                            norm_vec_1,
                            common::MINMAX_SIGNED);
                        normalization::calculate_min_max(
                            data_test_[num_symbol_2],
                            norm_vec_2,
                            common::MINMAX_SIGNED);
                        return correlation::calculate_pearson_correlation_coefficient(
                            norm_vec_1,
                            norm_vec_2,
                            out);
                    } else {
                        return common::INVALID_PARAMETER;
                    }
                }
            } else {
                if(data_[num_symbol_1].size() == (size_t)period_ &&
                    data_[num_symbol_2].size() == (size_t)period_) {
                    if(correlation_type == SPEARMAN_RANK) {
                        normalization::calculate_min_max(
                            data_[num_symbol_1],
                            norm_vec_1,
                            common::MINMAX_SIGNED);
                        normalization::calculate_min_max(
                            data_[num_symbol_2],
                            norm_vec_2,
                            common::MINMAX_SIGNED);
                        return correlation::calculate_spearman_rank_correlation_coefficient(
                            norm_vec_1,
                            norm_vec_2,
                            out);
                    } else
                    if(correlation_type == PEARSON) {
                        normalization::calculate_min_max(
                            data_[num_symbol_1],
                            norm_vec_1,
                            common::MINMAX_SIGNED);
                        normalization::calculate_min_max(
                            data_[num_symbol_2],
                            norm_vec_2,
                            common::MINMAX_SIGNED);
                        return correlation::calculate_pearson_correlation_coefficient(
                            norm_vec_1,
                            norm_vec_2,
                            out);
                    } else {
                        return common::INVALID_PARAMETER;
                    }
                }
            }
            return common::INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Найти коррелирующие валютные пары
         * \param symbol_1 список первой валютной пары в коррелирующей паре
         * \param symbol_2 список второй валютной пары в коррелирующей паре
         * \param threshold_coefficient порог срабатывания
         * для коэффициента корреляции
         * \param correlation_type тип корреляции (SPEARMAN_RANK, PEARSON)
         */
        void find_correlated_pairs(
                std::vector<size_t> &symbol_1,
                std::vector<size_t> &symbol_2,
                std::vector<T> &coefficient,
                T threshold_coefficient,
                const int correlation_type = SPEARMAN_RANK) {
            symbol_1.clear();
            symbol_2.clear();
            coefficient.clear();
            size_t data_test_size = data_test_.size();
            size_t data_test_size_dec = data_test_.size() - 1;
            if(is_test_) {
                for(size_t i = 0; i < data_test_size_dec; ++i) {
                    for(size_t j = i + 1; j < data_test_size; ++j) {
                        T coeff;
                        if(calculate_correlation(
                                coeff,
                                i,
                                j,
                                correlation_type) == common::OK) {
                            if(std::abs(coeff) > threshold_coefficient) {
                                symbol_1.push_back(i);
                                symbol_2.push_back(j);
                                coefficient.push_back(coeff);
                            } // if
                        } // if
                    } // for j
                } // for i
            } // if
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            data_.clear();
            data_test_.clear();
        }
    };

    /** \brief Адаптивная скользящая средняя Кауфмана
     *
     * Технический индикатор, разновидность адаптивной скользящей средней,
     * построенной на базе экспоненциально сглаженной скользящей средней
     * и оригинальной методики определения
     * и применения волатильности в качестве динамически
     * изменяющейся сглаживающей константы
     */
    template <typename T>
    class AMA {
    private:
        uint32_t n;
        uint32_t f;
        uint32_t s;
        uint32_t period_std_dev;
        T coeff;
        T prev_ama = 0;
        T filter = 0;
        bool is_square;
        std::vector<T> data;
        MW<T> iMW;
        int err_std_dev = common::NO_INIT;
    public:

        AMA(const uint32_t period = 10,
            const uint32_t fast_ma_period = 2,
            const uint32_t slow_ma_period = 30,
            const uint32_t period_filter = 10,
            const T coeff_filter = 0.1,
            const bool is_square_smooth = true) :
            n(period),
            f(fast_ma_period),
            s(slow_ma_period),
            period_std_dev(period_filter),
            coeff(coeff_filter),
            is_square(is_square_smooth),
            iMW(period_filter){}

       int update(const T in, T &out) {
            data.push_back(in);
            if(data.size() > n) {
                T direction = std::abs(data[n - 1] - data[0]);
                T volume = 0;
                for(uint32_t i = n - 1; i > 0; i--) {
                    volume += std::abs(data[i] - data[i - 1]);
                }
                T er = direction/volume;
                T fastest = 2.0/(T)(f + 1);
                T slowest = 2.0/(T)(s + 1);
                T smooth = er * (fastest - slowest) + slowest;
                T c = is_square ? smooth * smooth : smooth;
                T temp = c * in + (1.0 - c) * prev_ama;
                T di = temp - prev_ama;
                prev_ama = temp;
                out = prev_ama;
                err_std_dev = iMW.update(di);
                if(err_std_dev != common::OK)
                    return common::OK;
                T std_dev_value = 0;
                iMW.get_std_dev(std_dev_value, period_std_dev);
                filter = coeff * std_dev_value;
                return common::OK;
            } else {
                prev_ama = in;
                filter = 0;
                out = in;
                return common::NO_INIT;
            }
        }

        int test(const T in, T &out) {
            std::vector<T> test_data = data;
            test_data.push_back(in);
            if(test_data.size() > n) {
                T direction = std::abs(test_data[n - 1] - test_data[0]);
                T volume = 0;
                for(uint32_t i = n - 1; i > 0; i--) {
                    volume += std::abs(test_data[i] - test_data[i - 1]);
                }
                T er = direction/volume;
                T fastest = 2.0/(T)(f + 1);
                T slowest = 2.0/(T)(s + 1);
                T smooth = er * (fastest - slowest) + slowest;
                T c = is_square ? smooth * smooth : smooth;
                T temp = c * in + (1.0 - c) * prev_ama;
                T di = temp - prev_ama;
                out = temp;
                err_std_dev = iMW.update(di);
                if(err_std_dev != common::OK)
                    return common::OK;
                T std_dev_value = 0;
                iMW.get_std_dev(std_dev_value, period_std_dev);
                filter = coeff * std_dev_value;
                return common::OK;
            } else {
                filter = 0;
                out = in;
                return common::NO_INIT;
            }
        }

        int get_filter(T &out) {
            out = filter;
            return err_std_dev;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            data.clear();
            iMW.clear();
            prev_ama = 0;
            filter = 0;
            err_std_dev = common::NO_INIT;
        }
    };

    /** \brief Скользящая средняя NoLagMa
     */
    template <typename T>
    class NoLagMa {
        private:
        T Pi = 3.14159265358979323846264338327950288;
        std::vector<T> nlm_values;
        std::vector<T> nlm_prices;
        std::vector<T> nlm_alphas;
        uint32_t _length = 0;
        uint32_t _len = 1;
        uint32_t _weight = 2;
        uint32_t LengthMA = 10;
        size_t bars = 0;
        int err = common::NO_INIT;

        inline T calc(const T price, const int length, int r) {
            /* прошу прощения за говнокод,
             * это копипаст из метатрейдера, переписанный на С++
             */
            if(nlm_prices.size() != bars) {
                nlm_prices.resize(bars);
            }
            nlm_prices[r] = price;
            if (nlm_values[_length] != length) {
                T Cycle = 4.0;
                T Coeff = 3.0*Pi;
                int    Phase = length-1;

                nlm_values[_length] = length;
                nlm_values[_len] = length*4 + Phase;
                nlm_values[_weight] = 0;

                if (nlm_alphas.size() < nlm_values[_len]) {
                    nlm_alphas.resize((size_t)nlm_values[_len]);
                }
                for (int k = 0; k< nlm_values[_len]; k++) {
                    T t;
                    if (k <= Phase-1) {
                        t = 1.0 * k/(Phase-1);
                    } else {
                        t = 1.0 + (k - Phase + 1)*(2.0 * Cycle - 1.0)/
                            (Cycle * (T)length - 1.0);
                    }
                    T beta = cos(Pi*t);
                    T g = 1.0/(Coeff*t+1);
                    if (t <= 0.5 ) {g = 1;}

                    nlm_alphas[k] = g * beta;
                    nlm_values[_weight] += nlm_alphas[k];
                }
            }

            if (nlm_values[_weight]>0) {
                double sum = 0;
                for (int k=0; k < nlm_values[_len] && (r-k)>=0; k++)
                    sum += nlm_alphas[k]*nlm_prices[r-k];
                err = common::OK;
                return(sum / nlm_values[_weight]);
            } else return 0;
        }

        public:

        NoLagMa(const uint32_t period = 10) {
            LengthMA = period;
            nlm_values.resize(3);
        }

        int update(const T in, T &out) {
            ++bars;
            out = calc(in, LengthMA, bars - 1);
            return err;
        }

        int test(const T in, T &out) {
            std::vector<T> old_nlm_values = nlm_values;
            std::vector<T> old_nlm_prices = nlm_prices;
            std::vector<T> old_nlm_alphas = nlm_alphas;
            out = calc(in, LengthMA, bars);
            nlm_values = old_nlm_values;
            nlm_prices = old_nlm_prices;
            nlm_alphas = old_nlm_alphas;
            bars = 0;
            return err;
        }

        void clear() {
            nlm_values.resize(3);
            nlm_prices.clear();
            nlm_alphas.clear();
            err = common::NO_INIT;
        }
    };

    /** \brief Гистограмма частот
     */
    template<class T>
    class FreqHist {
    private:
        MW<T> iMW;
        dft::DftReal<T> iDftReal;
        size_t dft_period = 0;
    public:

        FreqHist() {};

        FreqHist(const size_t period, const size_t window_type) :
            iMW(period), iDftReal(period, window_type) {
            dft_period = period;
        };

        int update(
                const T &input,
                std::vector<T> &histogram,
                const T sample_rate = 0) {
            int err = iMW.update(input);
            if(err != common::OK) return err;
            std::vector<T> buffer;
            iMW.get_data(buffer);
            std::vector<T> frequencies;
            normalization::calculate_min_max(
                buffer,
                buffer,
                common::MINMAX_SIGNED);
            return iDftReal.update(buffer, histogram, frequencies, sample_rate);
        }

        int update(
                const T &input,
                std::vector<T> &amplitude,
                std::vector<T> &frequencies,
                const T sample_rate = 0) {
            int err = iMW.update(input);
            if(err != common::OK) return err;
            std::vector<T> buffer;
            iMW.get_data(buffer);
            normalization::calculate_min_max(
                buffer,
                buffer,
                common::MINMAX_SIGNED);
            return iDftReal.update(buffer, amplitude, frequencies, sample_rate);
        }

        void clear() {
            iMW.clear();
        }
    };

    /** \brief Индекс денежного потока
     */
    template <typename T, class INDICATOR_TYPE>
    class MFI {
    private:
        INDICATOR_TYPE iU;
        INDICATOR_TYPE iD;
        bool is_init_ = false;
        bool is_update_ = false;
        T prev_ = 0;
    public:
        MFI() {}

        /** \brief Инициализировать индикатор индекс денежного потока
         * \param period период индикатора
         */
        MFI(const size_t &period) : iU(period), iD(period) {
            is_init_ = true;
        }

        /** \brief Инициализировать индикатор индекса относительной силы
         * \param period период индикатора
         */
        void init(const size_t &period) {
            is_init_ = true;
            is_update_ = false;
            iU = INDICATOR_TYPE(period);
            iD = INDICATOR_TYPE(period);
        }

        /** \brief Обновить состояние индикатора
         * \param price Цена, в оригинале используется типичная цена
         * \param volume Объем торгов
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &price, const T &volume, T &out) {
            if(!is_init_) {
                return common::NO_INIT;
            }
            if(!is_update_) {
                prev_ = price;
                is_update_ = true;
                out = 50.0;
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            /* поток необработанных денег */
            T mf = price * volume;

            /* На основе денежного потока вычисляются положительный и отрицательный денежные потоки */
            T u = 0;
            T d = 0;
            if(prev_ < price) {
                u = mf;
            } else
            if(prev_ > price) {
                d = mf;
            }

            int erru, errd = 0;
            T mu = 0;
            T md = 0;
            erru = iU.update(u, mu);
            errd = iD.update(d, md);
            prev_ = price;
            if(erru != common::OK || errd != common::OK) {
                out = 50.0;
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            if(md == 0) {
                out = 100.0;
                return common::OK;
            }
            /* коэффициент денежного потока */
            T mfr = mu / md;
            out = 100.0 - (100.0 / (1.0 + mfr));
            return common::OK;
        }

        /** \brief Обновить состояние индикатора
         * \param high Наивысшая цена бара
         * \param low Наинизшая цена бара
         * \param close Цена закрытия бара
         * \param volume Объем торгов
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(
                const T &high,
                const T &low,
                const T &close,
                const T &volume,
                T &out) {
            const T price = (high + low + close) / 3.0;
            return update(price, volume, out);
        }

        /** \brief Обновить состояние индикатора
         * \param price Цена, в оригинале используется типичная цена
         * \param volume Объем торгов
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &price, const T &volume) {
            if(!is_init_) {
                return common::NO_INIT;
            }
            if(!is_update_) {
                prev_ = price;
                is_update_ = true;
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            /* поток необработанных денег */
            T mf = price * volume;

            /* На основе денежного потока вычисляются положительный и отрицательный денежные потоки */
            T u = 0;
            T d = 0;
            if(prev_ < price) {
                u = mf;
            } else
            if(prev_ > price) {
                d = mf;
            }
            int erru, errd = 0;
            erru = iU.update(u, u);
            errd = iD.update(d, d);
            prev_ = price;
            if(erru != common::OK || errd != common::OK) {
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            return common::OK;
        }

        /** \brief Обновить состояние индикатора
         * \param high Наивысшая цена бара
         * \param low Наинизшая цена бара
         * \param close Цена закрытия бара
         * \param volume Объем торгов
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(
                const T &high,
                const T &low,
                const T &close,
                const T &volume) {
            const T price = (high + low + close) / 3.0;
            return update(price, volume);
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param price Цена, в оригинале используется типичная цена
         * \param volume Объем торгов
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &price, const T &volume, T &out) {
            if(!is_init_) {
                return common::NO_INIT;
            }
            if(!is_update_) {
                out = 50.0;
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            /* поток необработанных денег */
            T mf = price * volume;

            /* На основе денежного потока вычисляются положительный и отрицательный денежные потоки */
            T u = 0;
            T d = 0;
            if(prev_ < price) {
                u = mf;
            } else
            if(prev_ > price) {
                d = mf;
            }

            T mu = 0;
            T md = 0;
            int erru, errd = 0;
            erru = iU.test(u, mu);
            errd = iD.test(d, md);
            if(erru != common::OK || errd != common::OK) {
                out = 50.0;
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            if(d == 0) {
                out = 100.0;
                return common::OK;
            }
            /* коэффициент денежного потока */
            T mfr = mu / md;
            out = 100.0 - (100.0 / (1.0 + mfr));
            return common::OK;
        }

        /** \brief Протестировать индикатор
         * \param high Наивысшая цена бара
         * \param low Наинизшая цена бара
         * \param close Цена закрытия бара
         * \param volume Объем торгов
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(
                const T &high,
                const T &low,
                const T &close,
                const T &volume,
                T &out) {
            const T price = (high + low + close) / 3.0;
            return test(price, volume, out);
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            is_update_ = false;
            iU.clear();
            iD.clear();
        }
    };

	/** \brief Индикатор осциллятор
     */
	template <class T, class INDICATOR_TYPE_1 = EMA<T>, class INDICATOR_TYPE_2 = EMA<T>, class INDICATOR_TYPE_3 = SMA<T>>
	class OsMa {
	private:
		INDICATOR_TYPE_1 iFastMA;
		INDICATOR_TYPE_2 iSlowMA;
		INDICATOR_TYPE_3 iSignalMA;
		DelayLine<T> iDelayLine;
	public:

		OsMa() {};

        /** \brief Конструктор индикатора осциллятора
         * \param period_fast Период быстрой скользящей средней
		 * \param period_slow Период медленной скользящей средней
		 * \param period_signal Период усреднения сигнальной линии
		 * \param shift Смещение назад
         */
        OsMa(	const size_t period_fast = 5,
				const size_t period_slow = 9,
				const size_t period_signal = 3,
				const size_t shift = 0) :
				iFastMA(period_fast),
				iSlowMA(period_slow),
				iSignalMA(period_signal),
				iDelayLine(shift) {}

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &out) {
			out = 0;
			T fast_ma = in, slow_ma = in, signal_ma = in;
			int err_fast = iFastMA.update(in, fast_ma);
			int err_slow = iSlowMA.update(in, slow_ma);
			if(err_fast != common::OK || err_slow != common::OK) return common::NO_INIT;
			int err_signal = iSignalMA.update((fast_ma - slow_ma), signal_ma);
			if(err_signal != common::OK) return err_signal;
			return iDelayLine.update(signal_ma, out);
        }

		/** \brief Протестировать состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &out) {
			out = 0;
			T fast_ma = in, slow_ma = in, signal_ma = in;
			int err_fast = iFastMA.test(in, fast_ma);
			int err_slow = iSlowMA.test(in, slow_ma);
			if(err_fast != common::OK || err_slow != common::OK) return common::NO_INIT;
			int err_signal = iSignalMA.test((fast_ma - slow_ma), signal_ma);
			if(err_signal != common::OK) return err_signal;
			return iDelayLine.test(signal_ma, out);
        }

		inline void clear() noexcept {
			iFastMA.clear();
			iSlowMA.clear();
			iSignalMA.clear();
			iDelayLine.clear();
		}
	};

	/** \brief Percent Range Index Function
     */
	template<class T>
	class PRI {
    private:
        FastMinMax<T> min_max;
        size_t period = 0;
        size_t offset = 0;
        T output = std::numeric_limits<T>::quiet_NaN();
    public:
        PRI() {};

        /** \brief Инициализация линий Боллинджера
         * \param user_period Период
         * \param user_offset Смещение назад
         */
        PRI(const size_t user_period, const size_t user_offset = 0) :
                min_max(user_period, user_offset),
                period(user_period), offset(user_offset) {
        }

        /** \brief Обновить состояние индикатора
         * \param in Сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in) noexcept {
            if(period == 0) {
                output = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }
            if(min_max.update(in) != common::OK) {
                output = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            const T diff = min_max.get_max() - min_max.get_min();
            output = diff == 0 ? 0 : 2.0 * (((in - min_max.get_min()) / diff) - 0.5);
            return common::OK;
        }


        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \param out   Сигнал на выходе от -1 до 1
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &out) noexcept {
            const int err = update(in);
            out = output;
            return err;
        }

        /** \brief Протестировать индикатор
         * \param in Сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in) noexcept {
            if(period == 0) {
                output = std::numeric_limits<T>::quiet_NaN();
                return common::NO_INIT;
            }
            if(min_max.test(in) != common::OK) {
                output = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            const T diff = min_max.get_max() - min_max.get_min();
            output = diff == 0 ? 0 : 2.0 * (((in - min_max.get_min()) / diff) - 0.5);
            return common::OK;
        }

        /** \brief Протестировать индикатор
         * \param in    Сигнал на входе
         * \param out   Сигнал на выходе от -1 до 1
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &out) noexcept {
            const int err = test(in);
            out = output;
            return err;
        }

        inline T get() noexcept {return output;};

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            min_max.clear();
            output = std::numeric_limits<T>::quiet_NaN();
        }
    };

	template<class T>
	class MaBBandsYxf {
	private:
		OsMa<T> iOsMa;
		DelayLine<T> iDelayLineOsMa;
		SMA<T> iSmaHigh;
		SMA<T> iSmaLow;
		DelayLine<T> iDelayLineHigh;
		DelayLine<T> iDelayLineLow;
		WMA<T> iWmaHigh;
		WMA<T> iWmaLow;
		BollingerBands<T> iBbHigh;
		BollingerBands<T> iBbLow;
		double point = 0.00001;
		size_t dist_2 = 20;     /**<  */
    public:

        MaBBandsYxf(
                const size_t ma_period = 9,
                const size_t move_shift = 12,
                const size_t bb_period = 20,
                const double bb_factor = 0.4,
                const size_t os_period = 3,
                const double symbol_point = 0.00001,
                const size_t symbol_dist = 20) :
            iOsMa(5, 9, os_period), iDelayLineOsMa(1),
            iSmaHigh(ma_period), iSmaLow(ma_period),
            iDelayLineHigh(move_shift), iDelayLineLow(move_shift),
            iWmaHigh(4), iWmaLow(4),
            iBbHigh(bb_period, bb_factor), iBbLow(bb_period, bb_factor),
            point(symbol_point), dist_2(symbol_dist) {
        }

        /** \brief Обновить состояние индикатора
         * \param high Наивысшая цена бара
         * \param low Наинизшая цена бара
         * \param close Цена закрытия бара
         * \param out Прогноз, BUY == 1, SELL == 1
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(
                const T &high,
                const T &low,
                const T &close,
                T &out) {
			out = 0;
			T sma_high = high, sma_low = low, wma_high = high, wma_low = low;
			T bb_high_tl = high, bb_high_ml = high, bb_high_bl = high;
			T bb_low_tl = low, bb_low_ml = low, bb_low_bl = low;
			T os_ma_now = 0, os_ma_pre = 0;

			int err_sma_high = iSmaHigh.update(high, sma_high);
			int err_wma_high = iWmaHigh.update(high, wma_high);
			int err_bb_high = iBbHigh.update(high, bb_high_tl, bb_high_ml, bb_high_bl);

			int err_sma_low = iSmaLow.update(low, sma_low);
			int err_wma_low = iWmaLow.update(low, wma_low);
            int err_bb_low = iBbLow.update(low, bb_low_tl, bb_low_ml, bb_low_bl);

            int err_os_ma = iOsMa.update(close, os_ma_now);
            int err_os_ma_pre = common::NO_INIT;
            if(err_os_ma == common::OK) {
                err_os_ma_pre = iDelayLineOsMa.update(os_ma_now, os_ma_pre);
            }

            if(err_sma_high != common::OK || err_sma_low != common::OK ||
                err_os_ma != common::OK || err_os_ma_pre != common::OK) return common::NO_INIT;
            T maup1 = sma_high;
            T madn1 = sma_low;
            int err_maup1 = iDelayLineHigh.update(sma_high, maup1);
            int err_madn1 = iDelayLineLow.update(sma_low, madn1);
            if(err_maup1 != common::OK || err_madn1 != common::OK) return common::NO_INIT;

            T line_1 = 0, line_2 = 0;
            if(maup1 > bb_high_tl) {
                line_1 = maup1 + dist_2 * point;
                bb_high_tl = 0;
            } else if(maup1 < bb_high_tl) {
                line_1 = bb_high_tl;
                maup1 = 0;
            }

            if(madn1 > 0.0) {
                if(madn1 < bb_low_bl) {
                    line_2 = madn1 - dist_2 * point;
                    bb_low_bl = 0;
                } else if(madn1 > bb_low_bl) {
                    line_2 = bb_low_bl ;
                    madn1 = 0;
                }
            }
            if(madn1 == 0.0) {
                line_2 = bb_low_bl;
                madn1 = 0;
            }

            /* сигнал вверх */
            if((os_ma_now > 0 && os_ma_pre < 0) &&
                (wma_low < line_2) &&
                (low < line_2)) {
                out = common::BUY;
            }
            /* сигнал вниз */
            if((os_ma_now < 0 && os_ma_pre > 0) &&
                (wma_high > line_1) &&
                (high > line_1) ) {
                out = common::SELL;
            }
            return common::OK;
        }

        /** \brief Протестировать индикатор
         * \param high Наивысшая цена бара
         * \param low Наинизшая цена бара
         * \param close Цена закрытия бара
         * \param out Прогноз, BUY == 1, SELL == 1
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(
                const T &high,
                const T &low,
                const T &close,
                T &out) noexcept {
			out = 0;
			T sma_high = high, sma_low = low, wma_high = high, wma_low = low;
			T bb_high_tl = high, bb_high_ml = high, bb_high_bl = high;
			T bb_low_tl = low, bb_low_ml = low, bb_low_bl = low;
			T os_ma_now = 0, os_ma_pre = 0;

			int err_sma_high = iSmaHigh.test(high, sma_high);
			int err_wma_high = iWmaHigh.test(high, wma_high);
			int err_bb_high = iBbHigh.test(high, bb_high_tl, bb_high_ml, bb_high_bl);

			int err_sma_low = iSmaLow.test(low, sma_low);
			int err_wma_low = iWmaLow.test(low, wma_low);
            int err_bb_low = iBbLow.test(low, bb_low_tl, bb_low_ml, bb_low_bl);

            int err_os_ma = iOsMa.test(close, os_ma_now);
            int err_os_ma_pre = common::NO_INIT;
            if(err_os_ma == common::OK) {
                err_os_ma_pre = iDelayLineOsMa.test(os_ma_now, os_ma_pre);
            }

            if(err_sma_high != common::OK || err_sma_low != common::OK ||
                err_os_ma != common::OK || err_os_ma_pre != common::OK) return common::NO_INIT;
            T maup1 = sma_high;
            T madn1 = sma_low;
            int err_maup1 = iDelayLineHigh.test(sma_high, maup1);
            int err_madn1 = iDelayLineLow.test(sma_low, madn1);
            if(err_maup1 != common::OK || err_madn1 != common::OK) return common::NO_INIT;

            T line_1 = 0, line_2 = 0;
            if(maup1 > bb_high_tl) {
                line_1 = maup1 + dist_2 * point;
                bb_high_tl = 0;
            } else if(maup1 < bb_high_tl) {
                line_1 = bb_high_tl;
                maup1 = 0;
            }

            if(madn1 > 0.0) {
                if(madn1 < bb_low_bl) {
                    line_2 = madn1 - dist_2 * point;
                    bb_low_bl = 0;
                } else if(madn1 > bb_low_bl) {
                    line_2 = bb_low_bl ;
                    madn1 = 0;
                }
            }
            if(madn1 == 0.0) {
                line_2 = bb_low_bl;
                madn1 = 0;
            }

            /* сигнал вверх */
            if((os_ma_now > 0 && os_ma_pre < 0) &&
                (wma_low < line_2) &&
                (low < line_2)) {
                out = common::BUY;
            }
            /* сигнал вниз */
            if((os_ma_now < 0 && os_ma_pre > 0) &&
                (wma_high > line_1) &&
                (high > line_1) ) {
                out = common::SELL;
            }
            return common::OK;
        }

        inline void clear() noexcept {
        	iSmaHigh.clear();
			iWmaHigh.clear();
			iBbHigh.clear();
			iSmaLow.clear();
			iWmaLow.clear();
            iBbLow.clear();
            iOsMa.clear();
            iDelayLineOsMa.clear();
            iDelayLineHigh.clear();
            iDelayLineLow.clear();
        }
	};


    /** \brief Trend-Direction-Force-Index
     * https://www.tradingview.com/script/9oPTFsRu-Trend-Direction-Force-Index/
     */
    template <typename T, class INDICATOR_TYPE>
    class TrendDirectionForceIndex {
    private:
        INDICATOR_TYPE ma1;
        INDICATOR_TYPE ma2;
        xtechnical::circular_buffer<T> buffer;
        T prev_ma1 = std::numeric_limits<T>::quiet_NaN();
        T prev_ma2 = std::numeric_limits<T>::quiet_NaN();
        T output = std::numeric_limits<T>::quiet_NaN();
        T point = 1;
    public:
        TrendDirectionForceIndex() {}

        /** \brief Инициализировать индикатор индекса относительной силы
         * \param user_period период индикатора
         */
        TrendDirectionForceIndex(const size_t user_period) :
                ma1(user_period), ma2(user_period),
                buffer(3 * user_period) {
        }

        /** \brief Инициализировать индикатор индекса относительной силы
         * \param user_period_ma периоды MA индикатора
         * \param user_period_buffer период внутреннего буфера индикатора
         */
        TrendDirectionForceIndex(
            const size_t user_period_ma,
            const size_t user_period_buffer) :
                ma1(user_period_ma),
                ma2(user_period_ma),
                buffer(user_period_buffer) {
        }

        /** \brief Инициализировать индикатор индекса относительной силы
         * \param user_period период индикатора
         */
        void init(const size_t user_period) noexcept {
            ma1 = INDICATOR_TYPE(user_period);
            ma2 = INDICATOR_TYPE(user_period);
            buffer = xtechnical::circular_buffer<T>(3 * user_period);
        }

        /** \brief Инициализировать индикатор индекса относительной силы
         * \param user_period_ma периоды MA индикатора
         * \param user_period_buffer период внутреннего буфера индикатора
         */
        void init(
                const size_t user_period_ma,
                const size_t user_period_buffer) noexcept {
            ma1 = INDICATOR_TYPE(user_period_ma);
            ma2 = INDICATOR_TYPE(user_period_ma);
            buffer = xtechnical::circular_buffer<T>(user_period_buffer);
        }

        inline void set_point(const T user_point) noexcept {
            point = user_point;
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in) noexcept {
            output = std::numeric_limits<T>::quiet_NaN();
            /* сначала имеем два значения скользящих средних */
            T v1, v2 = 0;
            if(ma1.update(in, v1) != common::OK) return common::NO_INIT;
            if(ma2.update(v1, v2) != common::OK) return common::NO_INIT;

            /* запоминаем предыдущие значения в перый раз */
            if(std::isnan(prev_ma1) || std::isnan(prev_ma2)) {
                prev_ma1 = v1;
                prev_ma2 = v2;
                return common::NO_INIT;
            }

            /* проводим вычисления */
            /*
            const T a = v1 - prev_ma1;
            const T b = v2 - prev_ma2;
            const T c = std::abs(v1 - v2) / point;
            const T d = (a + b) / (2.0 * point);
            const T r = c * d *d *d;
            */
            const T ma1_diff = v1 - prev_ma1;
            const T ma2_diff = v2 - prev_ma2;
            const T ma_diff_avg = (ma1_diff + ma2_diff) / 2.0;
            const T tdf = std::abs(v1 - v2) * ma_diff_avg * ma_diff_avg * ma_diff_avg;

            /* запоминаем предыдущие значения в последующие разы */
            prev_ma1 = v1;
            prev_ma2 = v2;

            buffer.update(std::abs(tdf));
            if(buffer.full()) {
                T h = 0;
                std::vector<T> data = buffer.to_vector();
                for (size_t i = 0; i < data.size(); ++i) {
                    if (h < data[i]) h = data[i];
                }
                output = (h > 0.0) ? (tdf / h) : 0.0;
                return common::OK;
            };
            return common::NO_INIT;
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in, T &out) noexcept {
            const int err = update(in);
            out = output;
            return err;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in) noexcept {
            output = std::numeric_limits<T>::quiet_NaN();
            /* сначала имеем два значения скользящих средних */
            T v1, v2 = 0;
            if(ma1.test(in, v1) != common::OK) return common::NO_INIT;
            if(ma2.test(v1, v2) != common::OK) return common::NO_INIT;

            /* запоминаем предыдущие значения в перый раз */
            if(std::isnan(prev_ma1) || std::isnan(prev_ma2)) {
                return common::NO_INIT;
            }

            /* проводим вычисления */
            /*
            const T a = v1 - prev_ma1;
            const T b = v2 - prev_ma2;
            const T c = std::abs(v1 - v2) / point;
            const T d = (a + b) / (2.0 * point);
            const T r = c * d *d *d;
            */
            const T ma1_diff = v1 - prev_ma1;
            const T ma2_diff = v2 - prev_ma2;
            const T ma_diff_avg = (ma1_diff + ma2_diff) / 2.0;
            const T tdf = std::abs(v1 - v2) * ma_diff_avg * ma_diff_avg * ma_diff_avg;

            /* запоминаем предыдущие значения в последующие разы */
            prev_ma1 = v1;
            prev_ma2 = v2;

            buffer.test(std::abs(tdf));
            if(buffer.full()) {
                T h = 0;
                std::vector<T> data = buffer.to_vector();
                for (size_t i = 0; i < data.size(); ++i) {
                    if (h < data[i]) h = data[i];
                }
                output = (h > 0.0) ? (tdf / h) : 0.0;
                return common::OK;
            };
            return common::NO_INIT;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in, T &out) noexcept {
            const int err = test(in);
            out = output;
            return err;
        }

        inline T get() noexcept {return output;};

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            ma1.clear();
            ma2.clear();
            buffer.clear();
            prev_ma1 = std::numeric_limits<T>::quiet_NaN();
            prev_ma2 = std::numeric_limits<T>::quiet_NaN();
            output = std::numeric_limits<T>::quiet_NaN();
        }
    };

    /** \brief Скользящая средняя относительной волатильности
     */
    template <typename T>
    class MAV {
    private:
        SMA<T> iSMA;
        T prev = std::numeric_limits<T>::quiet_NaN();
        T output = std::numeric_limits<T>::quiet_NaN();
        bool is_init = false;
    public:

        MAV() : iSMA() {};

        MAV(const size_t period) :
            iSMA(period) {

        }

        int update(const T in) noexcept {
            if(!is_init) {
                prev = in;
                is_init = true;
                return common::NO_INIT;
            }
            const T temp = (std::max(in, prev) / std::min(in, prev)) - 1.0;
            prev = in;
            T vol = 0;
            int err = iSMA.update(temp, vol);
            if(err != common::OK) return err;
            output = (vol * 100000.0d);
            return common::OK;
        }

        int update(const T in, T &out) noexcept {
            const int err = update(in);
            out = output;
            return err;
        }

        int test(const T in) noexcept {
            if(!is_init) return common::NO_INIT;
            const T temp = (std::max(in, prev) / std::min(in, prev)) - 1.0;
            T vol = 0;
            int err = iSMA.test(temp, vol);
            if(err != common::OK) return err;
            output = (vol * 100000.0d);
            return common::OK;
        }

        int test(const T in, T &out) noexcept {
            const int err = test(in);
            out = output;
            return err;
        }

        inline T get() noexcept {return output;};

        inline uint32_t get_index() noexcept {
            return std::isnan(output) ? 0 : (uint32_t)(output + 0.5d);
        };

        inline void clear() noexcept {
            is_init = false;
            iSMA.clear();
            prev = std::numeric_limits<T>::quiet_NaN();
            output = std::numeric_limits<T>::quiet_NaN();
        }
    };

    /** \brief Скользящая средняя Zscore
     */
    template <typename T>
    class MAZ {
    private:
        Zscore<T> iZscore;
        SMA<T> iSMA;
        T output = std::numeric_limits<T>::quiet_NaN();
    public:

        MAZ() : iSMA(), iZscore() {};

        /** \brief Конструктор индикатора
         * \param period_zscore Период Zscore
         * \param period_ma Период скользящей средней
         */
        MAZ(const size_t period_zscore, const size_t period_ma) :
            iZscore(period_zscore), iSMA(period_ma) {
        }

        int update(const T in) noexcept {
            T z, vol;
            int err = iZscore.update(in, z);
            if(err != common::OK) return err;
            err = iSMA.update(std::abs(z), vol);
            if(err != common::OK) return err;
            output = vol;
            return common::OK;
        }

        int update(const T in, T &out) noexcept {
            const int err = update(in);
            out = output;
            return err;
        }

        int test(const T in) noexcept {
            T z, vol;
            int err = iZscore.test(in, z);
            if(err != common::OK) return err;
            err = iSMA.test(std::abs(z), vol);
            if(err != common::OK) return err;
            output = vol;
            return common::OK;
        }

        int test(const T in, T &out) noexcept {
            const int err = test(in);
            out = output;
            return err;
        }

        inline T get() noexcept {return output;};

        inline uint32_t get_index() noexcept {
            return std::isnan(output) ? 0 : (uint32_t)(output + 0.5d);
        };

        inline void clear() noexcept {
            iZscore.clear();
            iSMA.clear();
            output = std::numeric_limits<T>::quiet_NaN();
        }
    };

    /** \brief Linear Regression Moving Average - линейно-регрессионная скользящая средняя
     * LRMA = 3.0 * LWMA - 2.0 * SMA
     * LWMA - Linear Weighted Moving Average(Close, Period)
     * SMA - Simple Moving Average(Close, Period)
     */
    template <typename T>
    class LRMA {
    private:
        SMA<T> iSMA;
        WMA<T> iWMA;
        T output = std::numeric_limits<T>::quiet_NaN();
    public:

        LRMA() : iSMA(), iWMA() {};

        LRMA(const size_t period) :
            iSMA(period), iWMA(period) {

        }

        int update(const T in) noexcept {
            T x1, x2;
            int err;
            if((err = iSMA.update(in, x1)) != common::OK) {
                output = std::numeric_limits<T>::quiet_NaN();
                return err;
            }
            if((err = iWMA.update(in, x2)) != common::OK) {
                output = std::numeric_limits<T>::quiet_NaN();
                return err;
            }
            output = 3.0 * x2 - 2.0 * x1;
            return common::OK;
        }

        int update(const T in, T &out) noexcept {
            const int err = update(in);
            out = output;
            return err;
        }

        int test(const T in) noexcept {
            T x1, x2;
            int err;
            if((err = iSMA.test(in, x1)) != common::OK) {
                output = std::numeric_limits<T>::quiet_NaN();
                return err;
            }
            if((err = iWMA.test(in, x2)) != common::OK) {
                output = std::numeric_limits<T>::quiet_NaN();
                return err;
            }
            output = 3.0 * x2 - 2.0 * x1;
            return common::OK;
        }

        int test(const T in, T &out) noexcept {
            const int err = test(in);
            out = output;
            return err;
        }

        inline T get() const noexcept {return output;};

        inline void clear() noexcept {
            iSMA.clear();
            iWMA.clear();
            output = std::numeric_limits<T>::quiet_NaN();
        }
    };

    /** \brief Percent Range Index Function
     */
	template<class T, class RSI_MA_TYPE, class MA_TYPE>
	class RSHILLMA {
    private:
        RSI<T, RSI_MA_TYPE> rsi_indicator;
        MA_TYPE ma_indicator;
        StdDev<T> std_dev_indicator;

        T out_tl = std::numeric_limits<T>::quiet_NaN();
        T out_ml = std::numeric_limits<T>::quiet_NaN();
        T out_bl = std::numeric_limits<T>::quiet_NaN();
        T out_rsi = std::numeric_limits<T>::quiet_NaN();
        T out = std::numeric_limits<T>::quiet_NaN();
        T deviation = 1.5;
    public:
        RSHILLMA() {};

        /** \brief Инициализация Percent Range Index Function
         * \param period_rsi        Период RSI, например 14
         * \param period_ma         Период MA, например 12
         * \param period_std_dev    Период стандартного отклонения, например 100
         * \param deviation_bb      Множитель стандартного отклонения для полос, например 1.5
         */
        RSHILLMA(
                const size_t period_rsi,
                const size_t period_ma,
                const size_t period_std_dev,
                const T deviation_bb) :
                rsi_indicator(period_rsi),
                ma_indicator(period_ma),
                std_dev_indicator(period_rsi),
                deviation(deviation_bb) {
        }

        /** \brief Обновить состояние индикатора
         * \param in Сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in) {
            int err_rsi = rsi_indicator.update(in);
            int err_std_dev = std_dev_indicator.update(in);
            if(err_rsi != common::OK || err_std_dev != common::OK) {
                out_tl = std::numeric_limits<T>::quiet_NaN();
                out_ml = std::numeric_limits<T>::quiet_NaN();
                out_bl = std::numeric_limits<T>::quiet_NaN();
                out_rsi = std::numeric_limits<T>::quiet_NaN();
                out = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            out_rsi = rsi_indicator.get();
            if(ma_indicator.update(out_rsi) != common::OK) {
                out_tl = std::numeric_limits<T>::quiet_NaN();
                out_ml = std::numeric_limits<T>::quiet_NaN();
                out_bl = std::numeric_limits<T>::quiet_NaN();
                out = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            const T d = std_dev_indicator.get() * deviation;
            out_ml = ma_indicator.get();
            out_tl = out_ml + d;
            out_bl = out_ml - d;
            const T pre_out = d == 0 ? 0 : (out_rsi - out_bl) / d - 1.0;
            out = pre_out > 1.0 ? 1.0 :  pre_out < -1.0 ? -1.0 : pre_out;
            return common::OK;
        }



        /** \brief Протестировать индикатор
         * \param in Сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in) {
            int err_rsi = rsi_indicator.test(in);
            int err_std_dev = std_dev_indicator.test(in);
            if(err_rsi != common::OK || err_std_dev != common::OK) {
                out_tl = std::numeric_limits<T>::quiet_NaN();
                out_ml = std::numeric_limits<T>::quiet_NaN();
                out_bl = std::numeric_limits<T>::quiet_NaN();
                out_rsi = std::numeric_limits<T>::quiet_NaN();
                out = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            out_rsi = rsi_indicator.get();
            if(ma_indicator.test(out_rsi) != common::OK) {
                out_tl = std::numeric_limits<T>::quiet_NaN();
                out_ml = std::numeric_limits<T>::quiet_NaN();
                out_bl = std::numeric_limits<T>::quiet_NaN();
                out = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            const T d = std_dev_indicator.get() * deviation;
            out_ml = ma_indicator.get();
            out_tl = out_ml + d;
            out_bl = out_ml - d;
            const T pre_out = d == 0 ? 0 : (out_rsi - out_bl) / d - 1.0;
            out = pre_out > 1.0 ? 1.0 :  pre_out < -1.0 ? -1.0 : pre_out;
            return common::OK;
        }

        inline T get() {return out;};
        inline T get_rsi() {return out_rsi;};
        inline T get_tl() {return out_tl;};
        inline T get_ml() {return out_ml;};
        inline T get_bl() {return out_bl;};

        /** \brief Очистить данные индикатора
         */
        void clear() {
            rsi_indicator.clear();
            ma_indicator.clear();
            std_dev_indicator.clear();
            out_tl = std::numeric_limits<T>::quiet_NaN();
            out_ml = std::numeric_limits<T>::quiet_NaN();
            out_bl = std::numeric_limits<T>::quiet_NaN();
            out_rsi = std::numeric_limits<T>::quiet_NaN();
            out = std::numeric_limits<T>::quiet_NaN();
        }
    };

    /** \brief Формирователь баров для усредненной цены
     */
    template<class T>
    class BarShaperV1 {
    public:

        /** \brief Данные бара
         */
        class Bar {
        public:
            T open = 0, high = 0, low = 0, close = 0;
            uint64_t timestamp = 0;
        };

    private:
        Bar bar;
        uint64_t period = 0;
        uint64_t last_bar = 0;

        bool is_open_equal_prev_close = false;
        bool is_use_bar_stop_time = false;
        bool is_fill = false;
        bool is_once = false;

    public:

        BarShaperV1() {};

        /** \brief Инициализировать формирователь баров
         * \param p     Период
         * \param oepc  Флаг, включает эквивалетность цены открытия цене закрытия предыдущего бара
         * \param ubst  Флаг, включает использование последней метки времени бара вместо начала бара, как времени бара
         * \param uf    Флаг, включает заполнение пропущенных баров
         */
        BarShaperV1(const uint64_t p, const bool oepc = false, const bool ubst = false, const bool uf = false) :
            period(p),
            is_open_equal_prev_close(oepc),
            is_use_bar_stop_time(ubst),
            is_fill(uf)  {};

        std::function<void(const Bar &bar)> on_close_bar;               /**< Функция обратного вызова в момент закрытия бара */
        std::function<void(const Bar &bar)> on_unformed_bar = nullptr;  /**< Функция обратного вызова для несформированного бара */

        /** \brief Обновить состояние индикатора
         * \param input     Текущая цена
         * \param timestamp Метка времени
         */
        void update(const T input, const uint64_t timestamp) {
            if (period == 0) return;
            if (last_bar == 0) {
                last_bar = timestamp / period;
                return;
            }
            const uint64_t current_bar = timestamp / period;
            if (current_bar > last_bar) {
                if (is_once) {
                    bar.timestamp = is_use_bar_stop_time ?
                        (last_bar * period + period) : (last_bar * period);
                    on_close_bar(bar);
                    if (is_fill) {
                        for (uint64_t b = (last_bar + 1); b < current_bar; ++b) {
                            bar.open = bar.low = bar.high = bar.close;
                            bar.timestamp = is_use_bar_stop_time ?
                                (b * period + period) : (b * period);
                            on_close_bar(bar);
                        }
                    }
                }
                if (is_open_equal_prev_close) {
                    if (bar.close != 0) {
                        bar.open = bar.close;
                        is_once = true;
                    }
                } else {
                    bar.open = input;
                    is_once = true;
                }
                bar.high = input;
                bar.low = input;
                bar.close = input;
                last_bar = current_bar;
            } else
            if (current_bar == last_bar) {
                if (is_once) {
                    bar.high = std::max(input, bar.high);
                    bar.low = std::min(input, bar.low);
                    bar.close = input;
                    if (on_unformed_bar != nullptr) {
                        bar.timestamp = is_use_bar_stop_time ?
                            (last_bar * period + period) : (last_bar * period);
                        on_unformed_bar(bar);
                    }
                } else {
                    bar.close = input;
                }
            }
        }
    };


    /** \brief График ренко
     */
    template<class T>
    class RenkoChart {
    public:

        /** \brief Данные бара
         */
        class Bar {
        public:
            T close = 0;
            uint64_t timestamp = 0;
        };

    private:
        uint64_t digits = 0;
        uint64_t step = 0;
        int64_t last_level = 0;
        Bar bar;

        std::array<double, 9> digits_to_value = {0.0, 0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001, 0.0000001, 0.00000001};

        bool is_once = false;
    public:

        std::function<void(const Bar &bar)> on_close_bar;               /**< Функция обратного вызова в момент закрытия бара */

        RenkoChart() {};

        /** \brief Инициализировать график ренко
         * \param d Количество разрядов
         * \param s Шаг графика
         */
        RenkoChart(const uint64_t d, const uint64_t s) : digits(d), step(s) {};

        void update(const T input, const uint64_t timestamp) {
            if (digits == 0) return;
            const int64_t level = (int64_t)(input / (digits_to_value[digits] * (double)step));
            if (last_level == 0) {
                last_level = level;
                return;
            }
            if (last_level != level) {
                if (!is_once) {
                    last_level = level;
                    is_once = true;
                    return;
                }
                if (last_level > level) {
                    bar.timestamp = timestamp;
                    for (int64_t l = last_level + 1; l <= level; ++l) {
                        bar.close = (double)l * digits_to_value[digits] * (double)step;
                        on_close_bar(bar);
                    }
                } else
                if (last_level < level) {
                    bar.timestamp = timestamp;
                    for (int64_t l = last_level - 1; l >= level; --l) {
                        bar.close = (double)l * digits_to_value[digits] * (double)step;
                        on_close_bar(bar);
                    }
                }
                last_level = level;
            }
        }

        void update(const T input) {
            if (digits == 0) return;
            const int64_t level = (int64_t)(input / (digits_to_value[digits] * (double)step));
            if (last_level == 0) {
                last_level = level;
                return;
            }
            if (last_level != level) {
                if (!is_once) {
                    last_level = level;
                    is_once = true;
                    return;
                }
                if (last_level > level) {
                    for (int64_t l = last_level + 1; l <= level; ++l) {
                        bar.close = (double)l * digits_to_value[digits] * (double)step;
                        on_close_bar(bar);
                    }
                } else
                if (last_level < level) {
                    for (int64_t l = last_level - 1; l >= level; --l) {
                        bar.close = (double)l * digits_to_value[digits] * (double)step;
                        on_close_bar(bar);
                    }
                }
                last_level = level;
            }
        }
    };

    /** \brief Мера склонности к чередовнию знаков (z-счет)
     *
     * Z - число СКО, на которое количество серий в выборке отклоняется
     * от своего математического ожидания
     * Если z > 3, то с вероятностью 0,9973 знаки имеют склонность к чередованию
     * Если z <-3, то с аналогичной вероятнсотью
     * проявляется склонность к сохранению знака
     * \param n общее число элементов в последовательности
     * \param r общее число серий положительных и отрицательных приращений
     * \param w общее число положительных приращений
     * \param l общее число отрицательных приращений
     * \return вернет Z
     */
    double calc_z_score(int n, int r, int w, int l) {
        double P = 2.0d * w * l;
        return (n * ((double)r - 0.5d) -  P) / sqrt((P * (P - (double)n))/
            ((double)n - 1.0d));
    }

    /** \brief Рассчитать долю капитала для стратегии
     * на основе меры склонности к чередовнию знаков
     * \param p вероятность правильного прогноза (от 0.0 до 1.0)
     * \param winperc процент выплаты брокера (от 0.0)
     * \return оптимальная доля капитала
     */
    double calc_z_score_capital_share(double p, double winperc) {
        return p - (1.0 - p) * (1.0/winperc);
    }
}

#endif // INDICATORSEASY_HPP_INCLUDED
