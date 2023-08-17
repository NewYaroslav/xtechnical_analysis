#ifndef XTECHNICAL_SUPER_TREND_HPP_INCLUDED
#define XTECHNICAL_SUPER_TREND_HPP_INCLUDED

#include "../xtechnical_common.hpp"
#include "xtechnical_atr.hpp"
#include "cci.hpp"

namespace xtechnical {

    /** \brief Индекс товарного канала
     */
    template <typename T, class MA_TYPE>
    class SuperTrend {
    private:
        CCI<T, MA_TYPE> iCCI;
        ATR<T, MA_TYPE> iATR;
        T output_value = std::numeric_limits<T>::quiet_NaN();
        T output_cci = std::numeric_limits<T>::quiet_NaN();
    public:

        SuperTrend() {};

        /** \brief Конструктор класса индикатора SuperTrend
         * \param   period_cci  Период CCI. Стандартное значение 50
         * \param   period_atr  Период ATR. Стандартное значение 5
         * \param   coeff_cci   Коэффициент CCI, по умолчанию 0.015
         */
        SuperTrend(const size_t period_cci, const size_t period_atr, const T coeff_cci = 0.015) :
            iCCI(period_cci, coeff_cci), iATR(period_atr) {
        };

        inline int update(const T in) noexcept {
            iCCI.update(in);
            iATR.update(in);
            if (std::isnan(iCCI.get())) return common::NO_INIT;
            if (std::isnan(iATR.get())) return common::NO_INIT;
            output_cci = iCCI.get();
            if (output_cci >= 0) output_value = in + iATR.get();
            else output_value = in - iATR.get();
            return common::OK;
        }

        inline int update(const T high, const T low, const T close) noexcept {
            iCCI.update(high, low, close);
            iATR.update(high, low, close);
            if (std::isnan(iCCI.get())) return common::NO_INIT;
            if (std::isnan(iATR.get())) return common::NO_INIT;
            output_cci = iCCI.get();
            if (output_cci >= 0) output_value = high + iATR.get();
            else output_value = low - iATR.get();
            return common::OK;
        }

        inline int update(const T in, T &out) noexcept {
            const int err = update(in);
            out = output_value;
            return err;
        }

        inline int update(const T high, const T low, const T close, T &out) noexcept {
            const int err = update(high, low, close);
            out = output_value;
            return err;
        }

        inline int test(const T in) noexcept {
            iCCI.test(in);
            iATR.test(in);
            if (std::isnan(iCCI.get())) return common::NO_INIT;
            if (std::isnan(iATR.get())) return common::NO_INIT;
            output_cci = iCCI.get();
            if (output_cci >= 0) output_value = in + iATR.get();
            else output_value = in - iATR.get();
            return common::OK;
        }

        inline int test(const T high, const T low, const T close) noexcept {
            iCCI.test(high, low, close);
            iATR.test(high, low, close);
            if (std::isnan(iCCI.get())) return common::NO_INIT;
            if (std::isnan(iATR.get())) return common::NO_INIT;
            output_cci = iCCI.get();
            if (output_cci >= 0) output_value = high + iATR.get();
            else output_value = low - iATR.get();
            return common::OK;
        }

        inline int test(const T in, T &out) noexcept {
            const int err = test(in);
            out = output_value;
            return err;
        }

        inline int test(const T high, const T low, const T close, T &out) noexcept {
            const int err = test(high, low, close);
            out = output_value;
            return err;
        }

        /** \brief Получить значение индикатора
         * \return Значение индикатора
         */
        inline T get() const noexcept {
            return output_value;
        }

        /** \brief Получить значение индикатора CCI
         * \return Значение индикатора
         */
        inline T get_cci() const noexcept {
            return output_cci;
        }

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            iCCI.clear();
            iATR.clear();
            output_cci = std::numeric_limits<T>::quiet_NaN();
            output_value = std::numeric_limits<T>::quiet_NaN();
        }
    }; // CCI

}; // xtechnical

#endif // XTECHNICAL_SUPER_TREND_HPP_INCLUDED
