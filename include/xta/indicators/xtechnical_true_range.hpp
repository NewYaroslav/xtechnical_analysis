#ifndef XTECHNICAL_TRUE_RANGE_HPP_INCLUDED
#define XTECHNICAL_TRUE_RANGE_HPP_INCLUDED

#include "../xtechnical_common.hpp"

namespace xtechnical {

    /** \brief Истинный диапазон (True Range)
     */
    template <typename T>
    class TrueRange {
    private:
        T last_data = std::numeric_limits<T>::quiet_NaN();
        T output_value = std::numeric_limits<T>::quiet_NaN();

    public:
        TrueRange() {};

		inline int update(const T high, const T low, const T close) noexcept {
            output_value = std::max(std::max(high - low, high - close), close - low);
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
            if (std::isnan(last_data)) {
				last_data = in;
				return common::NO_INIT;
			}
			output_value = std::abs(in - last_data);
			last_data = in;
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
            output_value = std::max(std::max(high - low, high - close), close - low);
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
            if (std::isnan(last_data)) return common::NO_INIT;
			output_value = std::abs(in - last_data);
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
            last_data = std::numeric_limits<T>::quiet_NaN();
        }
    }; // TrueRange

}; // xtechnical

#endif // XTECHNICAL_TRUE_RANGE_HPP_INCLUDED
