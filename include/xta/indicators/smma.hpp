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
#ifndef XTECHNICAL_SMMA_HPP_INCLUDED
#define XTECHNICAL_SMMA_HPP_INCLUDED

#include "../common/common.hpp"

namespace xta {

    /** \brief Smoothed Moving Average or Rolling Moving Average
     */
    template <class T = double>
    class SMMA final : public BaseIndicator<T> {
    private:
        T prev_value = 0;
        T period_divider = 0;
        T period_factor = 0;
        size_t period = 0;
        size_t count = 0;
        bool is_init = false;
    public:

        SMMA() {};

        /** \brief Initialize the SMMA (Smoothed Moving Average) indicator
         * \param p Period
         */
        SMMA(const size_t p) : period(p) {
            period_divider = 1.0 / (T)period;
            period_factor = (T)(period - 1);
        }

        virtual ~SMMA() = default;

        /** \brief Update the state of the indicator
         * \param value New value for the indicator, such as price
         * \param type  Price type (affects the indicator's mode of operation - intra-bar or end-of-bar)
         * \return Returns true in case of success
         */
        inline bool update(const T value, const PriceType type = PriceType::Close) noexcept {
            if (period <= 1) {
                BaseIndicator<T>::output_value[0] = value;
                return true;
            }
            if (is_init) {
                if (type == PriceType::IntraBar) {
                    BaseIndicator<T>::output_value[0] = (prev_value * period_factor + value) * period_divider;
                    return true;
                }
                prev_value = (prev_value * (T)(period - 1) + value) * period_divider;
                BaseIndicator<T>::output_value[0] = prev_value;
                return true;
            }
            if (count == period) {
                if (type == PriceType::IntraBar) {
                    BaseIndicator<T>::output_value[0] = (prev_value * period_factor + value) * period_divider;
                    return true;
                }
                prev_value = (prev_value * period_factor + value) * period_divider;
                BaseIndicator<T>::output_value[0] = prev_value;
                is_init = true;
                return true;
            }
            if (type == PriceType::IntraBar) {
                BaseIndicator<T>::output_value[0] = (prev_value + value) / (T)(count + 1);
                return false;
            }
            prev_value += value;
            ++count;
            BaseIndicator<T>::output_value[0] = prev_value / (T)count;
            if (count == period) {
                prev_value = BaseIndicator<T>::output_value[0];
            }
            return false;
        }

        inline bool is_ready() const noexcept {
            if (period <= 1) return true;
            return is_init;
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            BaseIndicator<T>::output_value[0] = get_empty_value<T>();
            count = 0;
            prev_value = 0;
            is_init = false;
        }
    };

}; // xtechnical

#endif // XTECHNICAL_SMMA_HPP_INCLUDED
