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
#ifndef XTECHNICAL_SMA_HPP_INCLUDED
#define XTECHNICAL_SMA_HPP_INCLUDED

#include "circular_buffer.hpp"

namespace xta {

    /** \brief Simple Moving Average
     */
    template <class T = double>
    class SMA final : public BaseIndicator<T> {
    private:
        CircularBuffer<T> buffer;
        T sum = 0;
        T period_divider = 0;
        size_t period = 0;
        size_t count = 0;
        bool is_init = false;
        bool is_done = false;

    public:

        SMA() {};

        /** \brief Initialize the SMA (Simple Moving Average) indicator
         * \param p Period
         */
        SMA(const size_t p) :
                buffer(p), period(p) {
            period_divider = 1.0 / (T)period;
        }

        virtual ~SMA() = default;

        /** \brief Update the state of the indicator
         * \param value New value for the indicator, such as price
         * \param type  Price type (affects the indicator's mode of operation - intra-bar or end-of-bar)
         * \return Returns true in case of success
         */
        inline bool update(const T value, const PriceType type = PriceType::Close) noexcept {
            if (period <= 1) {
                BaseIndicator<T>::output_value[0] = value;
                is_init = true;
                return true;
            }

            // https://habr.com/ru/articles/325590/
            const T removed_value = buffer.front();

            if (type == PriceType::IntraBar) {
                if (is_init) {
                    BaseIndicator<T>::output_value[0] = (sum - removed_value + value) * period_divider;
                    return true;
                }
                BaseIndicator<T>::output_value[0] = (sum + value) / (T)(count + 1);
                is_done = ((buffer.size() + 1) >= period);
                return is_done;
            }

            buffer.update(value, type);
            if (is_init) {
                sum = sum - removed_value + value;
                BaseIndicator<T>::output_value[0] = sum * period_divider;
                return true;
            }

            sum += value;
            ++count;
            BaseIndicator<T>::output_value[0] = sum / (T)count;

            if (buffer.is_ready()) {
                is_init = true;
                is_done = true;
                return true;
            }
            return false;
        }

        inline bool is_ready() const noexcept {
            return is_done;
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            buffer.reset();
            BaseIndicator<T>::output_value[0] = get_empty_value<T>();
            count = 0;
            sum = 0;
            is_init = false;
            is_done = false;
        }
    };

}; // xtechnical

#endif // XTECHNICAL_SMA_HPP_INCLUDED
