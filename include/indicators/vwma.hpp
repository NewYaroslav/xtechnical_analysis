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
#ifndef XTECHNICAL_VWMA_HPP_INCLUDED
#define XTECHNICAL_VWMA_HPP_INCLUDED

#include "circular_buffer.hpp"

namespace xtechnical {

    /** \brief Volume Weighted MA
     */
    template <class T = double>
    class VWMA final : public BaseIndicator<T> {
    private:
        CircularBuffer<T>   buffer;
        CircularBuffer<T>   weights;
        size_t              period = 0;

    public:

        VWMA() {};

        /** \brief Initialize the VWMA (Volume Weighted MA) indicator
         * \param p Period
         */
        VWMA(const size_t p) :
            buffer(p), weights(p), period(p) {
        }

        virtual ~VWMA() = default;

        inline bool update(const T value, const PriceType type = PriceType::Close) {
            return false;
        }

        /** \brief Update the state of the indicator
         * \param value     New value for the indicator, such as price
         * \param weight    Weight or Volume
         * \param type      Price type (affects the indicator's mode of operation - intra-bar or end-of-bar)
         * \return Returns true in case of success
         */
        inline bool update(const T value, const T weight, const PriceType type = PriceType::Close) noexcept {
            if (period <= 1) {
                BaseIndicator<T>::output_value[0] = value;
                return true;
            }
            buffer.update(value, type);
            weights.update(weight, type);
            if (buffer.is_ready()) {
                const std::vector<T> ts = buffer.to_vector();
                const std::vector<T> wts = weights.to_vector();
                T sum = 0;
                T sum2 = 0;
                for (size_t i = 0; i < period; ++i) {
                    sum += ts[i] * wts[i];
                    sum2 += wts[i];
                }
                BaseIndicator<T>::output_value[0] = sum2 == 0 ? 0 : sum / sum2;
                return true;
            }
            const std::vector<T> ts = buffer.to_vector();
            const std::vector<T> wts = weights.to_vector();
            T sum = 0;
            T sum2 = 0;
            for (size_t i = 0; i < buffer.size(); ++i) {
                sum += ts[i] * wts[i];
                sum2 += wts[i];
            }
            BaseIndicator<T>::output_value[0] = sum2 == 0 ? 0 : sum / sum2;
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
            weights.reset();
            BaseIndicator<T>::output_value[0] = get_empty_value<T>();
        }
    };

}; // xtechnical

#endif // XTECHNICAL_VWMA_HPP_INCLUDED
