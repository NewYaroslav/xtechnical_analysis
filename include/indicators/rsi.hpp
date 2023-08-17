/*
* xtechnical_analysis - Technical analysis C++ library
*
* Copyright (c) 2018-2023 Elektro Yar. Email: git.electroyar@gmail.com
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
#ifndef XTECHNICAL_RSI_HPP_INCLUDED
#define XTECHNICAL_RSI_HPP_INCLUDED

#include "sma.hpp"

namespace xtechnical {

    /** \brief Relative Strength Index
     */
    template <class T = double, template <class...> class MA_TYPE = xtechnical::SMA>
    class RSI final : public BaseIndicator<T> {
    private:
        MA_TYPE<T> ma_u;
        MA_TYPE<T> ma_d;
        size_t period = 0;
        T prev_value = get_empty_value<T>();
        bool is_init = false;
        bool is_done = false;
    public:

        RSI() {};

        /** \brief Initialize the RSI (Relative Strength Index) indicator
         * \param p Period
         */
        RSI(const size_t p) : ma_u(p), ma_d(p), period(p) {

        }

        virtual ~RSI() = default;

        /** \brief Update the state of the indicator
         * \param value New value for the indicator, such as price
         * \param type  Price type (affects the indicator's mode of operation - intra-bar or end-of-bar)
         * \return Returns true in case of success
         */
        inline bool update(const T value, const PriceType type = PriceType::Close) noexcept {
            if (period <= 1) {
                BaseIndicator<T>::output_value[0] = 50;
                return false;
            }
            if (type == PriceType::Close) {
                if (!is_init && is_value_empty<T>(prev_value)) {
                    prev_value = value;
                    return false;
                }
                T value_u = prev_value < value ? value - prev_value : 0;
                T value_d = prev_value > value ? prev_value - value : 0;
                prev_value = value;
                ma_u.update(value_u, type);
                ma_d.update(value_d, type);
                if (is_init) {
                    BaseIndicator<T>::output_value[0] = ma_d.get() == 0 ? (ma_u.get() == 0 ? 50 : 100) : 100.0 - (100.0 / (1.0 + (ma_u.get() / ma_d.get())));
                    return true;
                }
                if (!ma_u.is_ready()) return false;
                BaseIndicator<T>::output_value[0] = ma_d.get() == 0 ? (ma_u.get() == 0 ? 50 : 100) : 100.0 - (100.0 / (1.0 + (ma_u.get() / ma_d.get())));
                is_init = true;
                is_done = true;
                return true;
            }

            if (!is_init && is_value_empty<T>(prev_value)) {
                BaseIndicator<T>::output_value[0] = 50;
                return false;
            }

            T value_u = prev_value < value ? value - prev_value : 0;
            T value_d = prev_value > value ? prev_value - value : 0;
            ma_u.update(value_u, type);
            ma_d.update(value_d, type);
            if (ma_u.is_ready()) {
                BaseIndicator<T>::output_value[0] = ma_d.get() == 0 ? (ma_u.get() == 0 ? 50 : 100) : 100.0 - (100.0 / (1.0 + (ma_u.get() / ma_d.get())));
                is_done = true;
            }
            return is_done;
        }

        inline bool is_ready() const noexcept {
            return is_done;
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            ma_u.reset();
            ma_d.reset();
            BaseIndicator<T>::output_value[0] = get_empty_value<T>();
            prev_value = get_empty_value<T>();
            is_init = false;
            is_done = false;
        }
    };
}; // xtechnical

#endif // XTECHNICAL_RSI_HPP_INCLUDED
