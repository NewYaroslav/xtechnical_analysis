#ifndef XTECHNICAL_BODY_FILTER_HPP_INCLUDED
#define XTECHNICAL_BODY_FILTER_HPP_INCLUDED

#include "../xtechnical_common.hpp"

namespace xtechnical {

    /** \brief Фильтр размера тела бара
     */
    template <typename T, class MA_TYPE>
    class BodyFilter {
    private:
        MA_TYPE ma;
        T output_value = std::numeric_limits<T>::quiet_NaN();
        size_t period = 0;
    public:
        BodyFilter() {};

		BodyFilter(const size_t p) : ma(p), period(p) {}

		inline int update(const T open, const T high, const T low, const T close) noexcept {
            const T body = std::abs(open - close);
            const T total = std::abs(high - low);
            const T body_per = total == 0 ? 1.0 : body / total;
            if (period <= 1) output_value = body_per;
            else {
                ma.update(body_per);
                if (std::isnan(ma.get())) return common::NO_INIT;
                output_value = ma.get();
            }
            return common::OK;
        }

		inline int update(const T open, const T high, const T low, const T close, T &out) noexcept {
            const int err = update(high, low, close);
			out = output_value;
            return err;
        }

        inline int test(const T open, const T high, const T low, const T close) noexcept {
            const T body = std::abs(open - close);
            const T total = std::abs(high - low);
            const T body_per = total == 0 ? 1.0 : body / total;
            if (period <= 1) output_value = body_per;
            else {
                ma.test(body_per);
                if (std::isnan(ma.get())) return common::NO_INIT;
                output_value = ma.get();
            }
            return common::OK;
        }

		inline int test(const T open, const T high, const T low, const T close, T &out) noexcept {
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
            output_value = std::numeric_limits<T>::quiet_NaN();
            ma.clear();
        }
    }; // BodyFilter

}; // xtechnical

#endif // XTECHNICAL_BODY_FILTER_HPP_INCLUDED
