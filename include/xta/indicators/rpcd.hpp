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
#ifndef XTECHNICAL_RPCD_HPP_INCLUDED
#define XTECHNICAL_RPCD_HPP_INCLUDED

#include "circular_buffer.hpp"

namespace xta {

    enum RPCDLineType {
        RPCDLine        = 0,
        SmoothedLine    = 1,
        VolatilityLine  = 2,
    };

     /** \brief Relative Price Change Delta
     */
    template <class T = double, template <class...> class MA_TYPE = xta::SMA>
    class RPCD final : public BaseIndicator<T> {
    private:
        CircularBuffer<T>   buffer;
        MA_TYPE<T>          ma_slow;
        MA_TYPE<T>          ma_vol;
        size_t period = 0;
        size_t period_slow = 0;

    public:

        RPCD() : BaseIndicator<T>(3), buffer() {};

        /** \brief Initialize the Relative Price Change Delta indicator
         * \param period_delta  Period Delta
         * \param period_ma     Period MA
         */
        RPCD(const size_t period_delta, const size_t period_ma = 0) :
                BaseIndicator<T>(3),
                buffer(2*period_delta + 1),
                ma_slow(period_ma),
                ma_vol(period_ma),
                period(period_delta),
                period_slow(period_ma) {
        }

        virtual ~RPCD() = default;

        /** \brief Update the state of the indicator
         * \param value New value for the indicator, such as price
         * \param type  Price type (affects the indicator's mode of operation - intra-bar or end-of-bar)
         * \return Returns true in case of success
         */
        inline bool update(const T value, const PriceType type = PriceType::Close) noexcept {
            if (period == 0) {
                BaseIndicator<T>::output_value[0] = 0;
                return true;
            }
            buffer.update(value, type);
            if (buffer.is_ready()) {
                const T start = buffer.front();
                const T midle = buffer[period];
                const T stop = buffer.back();
                BaseIndicator<T>::output_value[0] = 100.0 * (stop - midle) / std::abs(midle - start);
                if (period_slow == 0) return true;
                ma_slow.update(BaseIndicator<T>::output_value[0], type);
                ma_vol.update(std::abs(BaseIndicator<T>::output_value[0]), type);
                BaseIndicator<T>::output_value[1] = ma_slow.get();
                BaseIndicator<T>::output_value[2] = ma_vol.get();
                return ma_slow.is_ready();
            }
            return false;
        }

        inline bool is_ready() const noexcept {
            if (period == 0) return true;
            if (period_slow == 0) return buffer.is_ready();
            return ma_slow.is_ready();
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            buffer.reset();
            ma_slow.reset();
            ma_vol.reset();
            for (auto &item : BaseIndicator<T>::output_value) {
                item = get_empty_value<T>();
            }
        }
    };
}; // xtechnical

#endif // XTECHNICAL_RPCD_HPP_INCLUDED
