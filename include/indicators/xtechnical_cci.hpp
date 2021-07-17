#ifndef XTECHNICAL_CCI_HPP_INCLUDED
#define XTECHNICAL_CCI_HPP_INCLUDED

#include "../xtechnical_common.hpp"
#include "xtechnical_true_range.hpp"

namespace xtechnical {

    /** \brief Индекс товарного канала
     */
    template <typename T, class MA_TYPE>
    class CCI {
    private:
        MA_TYPE ma;
        xtechnical::circular_buffer<T> buffer;
        T output_value = std::numeric_limits<T>::quiet_NaN();
        T coeff = 0.015;
    public:

        CCI() {};

        CCI(const size_t p, const T c = 0.015) :
            ma(p), buffer(p), coeff(c) {
        };

        inline int update(const T in) noexcept {
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
            output_value = (in - ma.get()) / (coeff * mad);
            return common::OK;
        }

        inline int update(const T high, const T low, const T close) noexcept {
            const T in = (high + low + close) / 3.0d;
            return update(in);
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
            output_value = (in - ma.get()) / (coeff * mad);
            return common::OK;
        }

        inline int test(const T high, const T low, const T close) noexcept {
            const T in = (high + low + close) / 3.0d;
            return test(in);
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

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            ma.clear();
            buffer.clear();
            output_value = std::numeric_limits<T>::quiet_NaN();
        }
    }; // CCI

}; // xtechnical

#endif // XTECHNICAL_CCI_HPP_INCLUDED
