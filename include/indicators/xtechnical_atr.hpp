#ifndef XTECHNICAL_ATR_HPP_INCLUDED
#define XTECHNICAL_ATR_HPP_INCLUDED

#include "../xtechnical_common.hpp"
#include "xtechnical_true_range.hpp"

namespace xtechnical {

    /** \brief Истинный диапазон (True Range)
     */
    template <typename T, class MA_TYPE>
    class ATR {
    private:
        MA_TYPE 	    ma;
		TrueRange<T> 	tr;
        T output_value = std::numeric_limits<T>::quiet_NaN();
    public:
        ATR() {};

		ATR(const size_t period) : ma(period) {}

		inline int update(const T high, const T low, const T close) noexcept {
            tr.update(high, low, close);
			if (std::isnan(tr.get())) return common::NO_INIT;
			ma.update(tr.get());
			if (std::isnan(ma.get())) return common::NO_INIT;
			output_value = ma.get();
            return common::OK;
        }

		inline int update(const T high, const T low, const T close, T &out) noexcept {
            const int err = update(high, low, close);
			out = output_value;
            return err;
        }

        /** \brief Обновить состояние индикатора
         * \param in 	Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        inline int update(const T in) noexcept {
            tr.update(in);
			if (std::isnan(tr.get())) return common::NO_INIT;
			ma.update(tr.get());
			if (std::isnan(ma.get())) return common::NO_INIT;
			output_value = ma.get();
            return common::OK;
        }

        /** \brief Обновить состояние индикатора
         * \param in    Сигнал на входе
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        inline int update(const T in, T &out) noexcept {
            const int err = update(in);
            out = output_value;
            return err;
        }

		inline int test(const T high, const T low, const T close) noexcept {
            tr.test(high, low, close);
			if (std::isnan(tr.get())) return common::NO_INIT;
			ma.test(tr.get());
			if (std::isnan(ma.get())) return common::NO_INIT;
			output_value = ma.get();
            return common::OK;
        }

		inline int test(const T high, const T low, const T close, T &out) noexcept {
            const int err = test(high, low, close);
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
        inline int test(const T in) noexcept {
            tr.test(in);
			if (std::isnan(tr.get())) return common::NO_INIT;
			ma.test(tr.get());
			if (std::isnan(ma.get())) return common::NO_INIT;
			output_value = ma.get();
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
        inline int test(const T in, T &out) noexcept {
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
            output_value = std::numeric_limits<T>::quiet_NaN();
            tr.clear();
            ma.clear();
        }
    }; // ATR

}; // xtechnical

#endif // XTECHNICAL_ATR_HPP_INCLUDED
