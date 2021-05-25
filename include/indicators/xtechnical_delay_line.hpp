#ifndef XTECHNICAL_DELAY_LINE_HPP_INCLUDED
#define XTECHNICAL_DELAY_LINE_HPP_INCLUDED

#include "../xtechnical_common.hpp"
#include "../xtechnical_circular_buffer.hpp"

namespace xtechnical {

     /** \brief Линия задержки
     */
    template <typename T>
    class DelayLine {
    private:
        circular_buffer<T> buffer;
        size_t period = 0;
        T output_value = std::numeric_limits<T>::quiet_NaN();
    public:

        DelayLine() : buffer() {};

        /** \brief Конструктор линии задержки
         * \param p     Период
         */
        DelayLine(const size_t p) :
            buffer(p + 1), period(p) {
        }

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in) noexcept {
            if(period == 0) {
                output_value = in;
                return common::OK;
            }
            buffer.update(in);
            if(buffer.full()) {
                output_value = buffer.front();
                return common::OK;
            } else {
                output_value = std::numeric_limits<T>::quiet_NaN();
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
                return common::OK;
            }
            buffer.test(in);
            if(buffer.full()) {
                output_value = buffer.front();
                return common::OK;
            } else {
                output_value = std::numeric_limits<T>::quiet_NaN();
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
}; // xtechnical

#endif // XTECHNICAL_FAST_MIN_MAX_HPP_INCLUDED
