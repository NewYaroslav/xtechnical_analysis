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
#ifndef XTECHNICAL_MOMENTUM_HPP_INCLUDED
#define XTECHNICAL_MOMENTUM_HPP_INCLUDED

#include "delay_line.hpp"

namespace xta {

    /** \brief Momentum
     */
    template <class T = double>
    class Momentum final : public BaseIndicator<T> {
    private:
        DelayLine<T> delay_line;
        size_t period = 0;
        bool is_rate_of_change = false;

    public:

        Momentum() {};

        /** \brief Initialize the Momentum indicator
         * \param p             Period
         * \param is_rc_mode    Use Rate of Change
         */
        Momentum(const size_t p, const bool is_rc_mode = false) :
            delay_line(p), period(p), is_rate_of_change(is_rc_mode) {

        }

        virtual ~Momentum() = default;

        /** \brief Update the state of the indicator
         * \param value New value for the indicator, such as price
         * \param type  Price type (affects the indicator's mode of operation - intra-bar or end-of-bar)
         * \return Returns true in case of success
         */
        inline bool update(const T value, const PriceType type = PriceType::Close) noexcept {
            if (period == 0) return false;
            delay_line.update(value, type);
            if (delay_line.is_ready()) {
                BaseIndicator<T>::output_value[0] = is_rate_of_change ? (delay_line.get() == 0 ? (value > 0 ? 100 : -100) : 100.0 * value / delay_line.get()) : (value - delay_line.get());
                return true;
            }
            return false;
        }

        inline bool is_ready() const noexcept {
            return delay_line.is_ready();
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            delay_line.reset();
            BaseIndicator<T>::output_value[0] = get_empty_value<T>();
        }
    };
};

#endif // XTECHNICAL_MOMENTUM_HPP_INCLUDED
