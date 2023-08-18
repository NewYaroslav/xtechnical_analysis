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
#ifndef XTECHNICAL_DEMARKER_HPP_INCLUDED
#define XTECHNICAL_DEMARKER_HPP_INCLUDED

#include "sma.hpp"

namespace xta {

    template <class T = double>
    class DeMarker final : public BaseIndicator<T> {
    private:
        SMA<T> sma_max;
        SMA<T> sma_min;
        T prev_value = get_empty_value<T>();
        size_t period = 0;

    public:

        DeMarker() {};

        /** \brief Initialize the DeMarker
         * \param p Period
         */
        DeMarker(const size_t p) :
                sma_max(p), sma_min(p), period(p) {
        }

        virtual ~DeMarker() = default;

        /** \brief Update the state of the indicator
         * \param value New value for the indicator, such as price
         * \param type  Price type (affects the indicator's mode of operation - intra-bar or end-of-bar)
         * \return Returns true in case of success
         */
        inline bool update(const T value, const PriceType type = PriceType::Close) noexcept {
            if (period <= 1) {
                BaseIndicator<T>::output_value[0] = 0.5;
                return false;
            }
            if (is_value_empty(prev_value)) {
                if (type == PriceType::Close) prev_value = value;
                BaseIndicator<T>::output_value[0] = 0.5;
                return false;
            }

            T de_max = 0, de_min = 0;
            if (value > prev_value) de_max = value - prev_value;
            if (value < prev_value) de_min = prev_value - value;

            sma_max.update(de_max, type);
            sma_min.update(de_min, type);

            if (type == PriceType::Close) {
                prev_value = value;
            }

            const T temp = sma_max.get() + sma_min.get();
            BaseIndicator<T>::output_value[0] = temp == 0 ? 0.5 : (sma_max.get() / temp);
            return sma_max.is_ready();
        }

        inline bool is_ready() const noexcept {
            if (period <= 1) return true;
            return sma_max.is_ready();
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            sma_max.reset();
            sma_min.reset();
            BaseIndicator<T>::output_value[0] = get_empty_value<T>();
            prev_value = get_empty_value<T>();
        }
    };

};

#endif // XTECHNICAL_DEMARKER_HPP_INCLUDED
