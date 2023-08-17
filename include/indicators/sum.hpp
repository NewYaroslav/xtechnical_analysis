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
#ifndef XTECHNICAL_SUM_HPP_INCLUDED
#define XTECHNICAL_SUM_HPP_INCLUDED

#include "circular_buffer.hpp"

namespace xtechnical {

    /** \brief SUM
     */
    template <class T = double>
    class SUM final : public BaseIndicator<T> {
    private:
        CircularBuffer<T> buffer;
        T prev_value = 0;
        size_t period = 0;

    public:

        SUM() {};

        /** \brief Initialize the SUM
         * \param p Period
         */
        SUM(const size_t p) :
                buffer(p + 1), period(p) {
        }

        virtual ~SUM() = default;

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
                if (type == PriceType::IntraBar) {
                    BaseIndicator<T>::output_value[0] = prev_value + (value - buffer.front());
                    return true;
                }
                prev_value = prev_value + (value - buffer.front());
                BaseIndicator<T>::output_value[0] = prev_value;
                return true;
            }
            if (type == PriceType::IntraBar) {
                BaseIndicator<T>::output_value[0] = (prev_value + value);
                return false;
            }
            prev_value += value;
            BaseIndicator<T>::output_value[0] = prev_value;
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
            prev_value = 0;
        }
    };

}; // xtechnical

#endif // XTECHNICAL_SUM_HPP_INCLUDED
