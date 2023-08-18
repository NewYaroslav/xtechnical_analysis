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
#ifndef XTECHNICAL_MACD_HPP_INCLUDED
#define XTECHNICAL_MACD_HPP_INCLUDED

#include "ema.hpp"
#include "sma.hpp"

namespace xta {

    enum MACDLineType {
        MACDLine        = 0,
        SignalLine      = 1,
        MACDHistogram   = 2
    };

     /** \brief MACD
     */
    template <class T = double, template <class...> class MA_TYPE = xta::EMA>
    class MACD final : public BaseIndicator<T> {
    private:
        MA_TYPE<T>  ma_fast;
        MA_TYPE<T>  ma_slow;
        SMA<T>      ma_signal;
        size_t      period_signal = 0;
        bool        is_done = false;

    public:

        MACD() : BaseIndicator<T>(3) {};

        MACD(
            const size_t arg_period_fast,
            const size_t arg_period_slow,
            const size_t arg_period_signal) : BaseIndicator<T>(3),
            ma_fast(arg_period_fast), ma_slow(arg_period_slow),
            ma_signal(arg_period_signal), period_signal(arg_period_signal) {
        }

        virtual ~MACD() = default;

        /** \brief Update the state of the indicator
         * \param value New value for the indicator, such as price
         * \param type  Price type (affects the indicator's mode of operation - intra-bar or end-of-bar)
         * \return Returns true in case of success
         */
        inline bool update(const T value, const PriceType type = PriceType::Close) noexcept {
            ma_fast.update(value, type);
            ma_slow.update(value, type);
            if (!ma_slow.is_ready() || !ma_fast.is_ready()) return false;
            BaseIndicator<T>::output_value[0] = ma_fast.get() - ma_slow.get();
            if (period_signal == 0) {
                is_done = true;
                return true;
            }
            ma_signal.update(BaseIndicator<T>::output_value[0], type);
            if (!ma_signal.is_ready()) return false;
            BaseIndicator<T>::output_value[1] = ma_signal.get();
            BaseIndicator<T>::output_value[2] = BaseIndicator<T>::output_value[0] - ma_signal.get();
            is_done = true;
            return true;
        }

        inline bool is_ready() const noexcept {
            return is_done;
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            ma_fast.reset();
            ma_slow.reset();
            ma_signal.reset();
            for (auto &item : BaseIndicator<T>::output_value) {
                item = get_empty_value<T>();
            }
            is_done = false;
        }
    };
}; // xtechnical

#endif // XTECHNICAL_MACD_HPP_INCLUDED
