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
#ifndef XTECHNICAL_LWMA_HPP_INCLUDED
#define XTECHNICAL_LWMA_HPP_INCLUDED

#include "circular_buffer.hpp"

namespace xta {

    /** \brief Linear Weighted Moving Average
     */
    template <class T = double>
    class LWMA final : public BaseIndicator<T> {
    private:
        CircularBuffer<T>   buffer;
        std::vector<T>      weights;
        T                   factor = 0;
        size_t              period = 0;

    public:

        LWMA() {};

        /** \brief Initialize the LWMA (Linear Weighted Moving Average) indicator
         * \param p Period
         */
        LWMA(const size_t p) : buffer(p), period(p) {
            weights.resize(period);
            for (size_t i = 0; i < period; ++i) {
                weights[i] = i + 1;
            }
            factor = 2.0 / (T)(period * (period + 1));
        }

        virtual ~LWMA() = default;

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
            buffer.update(value, type);
            if (buffer.is_ready()) {
                const std::vector<T> ts = buffer.to_vector();
                T sum = 0;
                for (size_t i = 0; i < period; ++i) {
                    sum += ts[i] * weights[i];
                }
                BaseIndicator<T>::output_value[0] = sum * factor;
                return true;
            }
            const std::vector<T> ts = buffer.to_vector();
            T sum = 0;
            for (size_t i = 0; i < buffer.size(); ++i) {
                sum += ts[i] * weights[i];
            }
            BaseIndicator<T>::output_value[0] = sum * factor;
            return false;
        }

        inline bool is_ready() const noexcept {
            if (period <= 1) return true;
            return buffer.is_ready();
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            buffer.reset();
            BaseIndicator<T>::output_value[0] = get_empty_value<T>();
        }
    };

}; // xtechnical

#endif // XTECHNICAL_LWMA_HPP_INCLUDED
