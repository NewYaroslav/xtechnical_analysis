#ifndef XTECHNICAL_FAST_MIN_MAX_HPP_INCLUDED
#define XTECHNICAL_FAST_MIN_MAX_HPP_INCLUDED

#include "../xtechnical_common.hpp"
#include "xtechnical_delay_line.hpp"

namespace xtechnical {

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
            if(delay_line.update(input) != common::OK) {
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            input = delay_line.get();
            if (index == 0) {
                ++index;
                last_input = input;
                return common::INDICATOR_NOT_READY_TO_WORK;
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
                return common::OK;
            }
            last_input = input;
            return common::INDICATOR_NOT_READY_TO_WORK;
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
            if(delay_line.test(input) != common::OK) {
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            input = delay_line.get();
            if (index == 0) return common::INDICATOR_NOT_READY_TO_WORK;
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
                return common::OK;
            }
            return common::INDICATOR_NOT_READY_TO_WORK;
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
}; // xtechnical

#endif // XTECHNICAL_FAST_MIN_MAX_HPP_INCLUDED
