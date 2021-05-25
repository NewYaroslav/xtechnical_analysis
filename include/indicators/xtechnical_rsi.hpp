#ifndef XTECHNICAL_RSI_HPP_INCLUDED
#define XTECHNICAL_RSI_HPP_INCLUDED

#include "../xtechnical_common.hpp"

namespace xtechnical {

    /** \brief Индекс относительной силы (RSI)
     */
    template <typename T, class MA_TYPE>
    class RSI {
    private:
        MA_TYPE iU;
        MA_TYPE iD;
        bool is_init_ = false;
        bool is_update_ = false;
        T prev_ = 0;
        T output_value = std::numeric_limits<T>::quiet_NaN();
    public:

        RSI() {}

        /** \brief Инициализировать индикатор индекса относительной силы
         * \param period период индикатора
         */
        RSI(const size_t period) :
            iU(period), iD(period) {
        }

        /** \brief Инициализировать индикатор индекса относительной силы
         * \param period Период индикатора
         */
        inline void init(const size_t period) noexcept {
            is_update_ = false;
            iU = MA_TYPE(period);
            iD = MA_TYPE(period);
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in) noexcept {
            if(!is_update_) {
                prev_ = in;
                output_value = std::numeric_limits<T>::quiet_NaN();
                is_update_ = true;
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            T u = 0, d = 0;
            if(prev_ < in) {
                u = in - prev_;
            } else
            if(prev_ > in) {
                d = prev_ - in;
            }
            int erru, errd = 0;
            erru = iU.update(u, u);
            errd = iD.update(d, d);
            prev_ = in;
            if(erru != common::OK || errd != common::OK) {
                output_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            if(d == 0) {
                output_value = 100.0;
                return common::OK;
            }
            T rs = u / d;
            output_value = 100.0 - (100.0 / (1.0 + rs));
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
            return common::OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем, что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in) noexcept {
            if(!is_update_) {
                output_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            T u = 0, d = 0;
            if(prev_ < in) {
                u = in - prev_;
            } else
            if(prev_ > in) {
                d = prev_ - in;
            }
            int erru, errd = 0;
            erru = iU.test(u, u);
            errd = iD.test(d, d);
            if(erru != common::OK || errd != common::OK) {
                output_value = std::numeric_limits<T>::quiet_NaN();
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            if(d == 0) {
                output_value = 100.0;
                return common::OK;
            }
            T rs = u / d;
            output_value = 100.0 - (100.0 / (1.0 + rs));
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
            return common::OK;
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
            output_value = std::numeric_limits<T>::quiet_NaN();
            is_update_ = false;
            iU.clear();
            iD.clear();
        }
    };

}; // xtechnical

#endif // XTECHNICAL_RSI_HPP_INCLUDED
