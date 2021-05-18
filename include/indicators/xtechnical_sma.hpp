#ifndef XTECHNICAL_SMA_HPP_INCLUDED
#define XTECHNICAL_SMA_HPP_INCLUDED

#include "../xtechnical_common.hpp"

namespace xtechnical {
    using namespace xtechnical_common;

    /** \brief Простая скользящая средняя
     */
    template <typename T>
    class SMA {
    private:
        xtechnical::circular_buffer<T> buffer;
        T last_data = 0;
        T output_value = std::numeric_limits<T>::quiet_NaN();
        size_t period = 0;
    public:
        SMA() {};

        /** \brief Инициализировать простую скользящую среднюю
         * \param p     Период
         */
        SMA(const size_t p) :
                buffer(p + 1), period(p) {
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in) noexcept {
            if(period == 0) {
                output_value = std::numeric_limits<T>::quiet_NaN();
                return NO_INIT;
            }
            buffer.update(in);
            if(buffer.full()) {
                last_data = last_data + (in - buffer.front());
                output_value = last_data/(T)period;
            } else {
                last_data += in;
                output_value = std::numeric_limits<T>::quiet_NaN();
                return INDICATOR_NOT_READY_TO_WORK;
            }
            return OK;
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
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in) noexcept {
            if(period == 0) {
                output_value = std::numeric_limits<T>::quiet_NaN();
                return NO_INIT;
            }
            buffer.test(in);
            if(buffer.full()) {
                output_value = (last_data + (in - buffer.front()))/(T)period;
            } else {
                output_value = std::numeric_limits<T>::quiet_NaN();
                return INDICATOR_NOT_READY_TO_WORK;
            }
            return OK;
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

}; // xtechnical

#endif // XTECHNICAL_SMA_HPP_INCLUDED
