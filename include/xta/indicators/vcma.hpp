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
#ifndef XTECHNICAL_VCMA_HPP_INCLUDED
#define XTECHNICAL_VCMA_HPP_INCLUDED

#include "../common/common.hpp"

namespace xta {

    /** \brief VCMA
     */
    template <class T = double>
    class VCMA final : public BaseIndicator<T> {
    private:
        T sum = 0;
        T sum_weight = 0;
        size_t count = 0;
        bool is_test = false;

    public:

        VCMA() {};

        virtual ~VCMA() = default;

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
            if (type == PriceType::IntraBar) {
                is_test = true;
                const T temp = sum_weight + weight;
                if (temp == 0) {
                    BaseIndicator<T>::output_value[0] = 0;
                    return true;
                }
                BaseIndicator<T>::output_value[0] = (sum + value * weight) / (sum_weight + weight);
                return true;
            }
            sum += value * weight;
            sum_weight += weight;
            ++count;
            if (sum_weight == 0) {
                BaseIndicator<T>::output_value[0] = 0;
                return true;
            }
            BaseIndicator<T>::output_value[0] = sum / sum_weight;
            is_test = false;
            return true;
        }

        inline bool is_ready() const noexcept {
            return (count > 0);
        }

        inline size_t size() const noexcept {
            return is_test ? (count + 1) : count;
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            BaseIndicator<T>::output_value[0] = get_empty_value<T>();
            count = 0;
            sum = 0;
            sum_weight = 0;
            is_test = false;
        }
    };

};

#endif // XTECHNICAL_VCMA_HPP_INCLUDED
