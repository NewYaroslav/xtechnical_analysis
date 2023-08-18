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
#ifndef XTECHNICAL_STD_DEV_HPP_INCLUDED
#define XTECHNICAL_STD_DEV_HPP_INCLUDED

#include "circular_buffer.hpp"

namespace xta {

    /** \brief Standard Deviation
     */
    template <class T = double>
    class StdDev final : public BaseIndicator<T> {
    private:
        CircularBuffer<T> buffer;
        T sum_squared = 0;
        T sum = 0;
        T period_divider_1 = 0;
        T period_divider_2 = 0;
        size_t period = 0;
        bool is_simple_std = false;
        bool is_init = false;
        bool is_done = false;
    public:

        StdDev() {};

        /** \brief Initialize the StdDev (Standard Deviation) indicator
         * \param arg_period        Period
         * \param arg_simple_std    Use Simple Standard Deviation
         */
        StdDev(const size_t p, const bool arg_simple_std = false) :
                buffer(p), period(p), is_simple_std(arg_simple_std) {
            period_divider_1 = (1.0 / (T)period);
            period_divider_2 = is_simple_std ? period_divider_1 : (1.0 / (T)(period - 1));
        }

        virtual ~StdDev() = default;

        inline bool is_simple() const noexcept {
            return is_simple_std;
        }

        inline bool is_sample() const noexcept {
            return !is_simple_std;
        }

        /** \brief Update the state of the indicator
         * \param value New value for the indicator, such as price
         * \param type  Price type (affects the indicator's mode of operation - intra-bar or end-of-bar)
         * \return Returns true in case of success
         */
        inline bool update(const T value, const PriceType type = PriceType::Close) noexcept {
            if (type == PriceType::IntraBar) {
                if (period <= 1) {
                    BaseIndicator<T>::output_value[0] = 0;
                    return false;
                }

                if (is_init) {
                    const T removed_value = buffer.front();
                    const T mean = (sum - removed_value + value) * period_divider_1;
                    const T variance = ((sum_squared - (removed_value * removed_value) + (value * value)) - (T)period * (mean * mean)) * period_divider_2;
                    BaseIndicator<T>::output_value[0] = variance > 0 ? std::sqrt(variance) : 0.0;
                    return true;
                }

                if (buffer.size() == 0) {
                    BaseIndicator<T>::output_value[0] = 0;
                    return false;
                }

                const size_t count = buffer.size() + 1;

                const std::vector<T> ts = buffer.to_vector();
                T mean = 0;
                for (size_t i = 0; i < buffer.size(); ++i) {
                    mean += ts[i];
                }
                mean += value;
                mean /= (T)count;

                T sum_diff = 0;
                for (size_t i = 0; i < buffer.size(); ++i) {
                    const T temp = ts[i] - mean;
                    sum_diff += temp * temp;
                }
                const T temp = value - mean;
                sum_diff += temp * temp;
                const T variance = is_simple_std ? sum_diff / (T)count : sum_diff / (T)(count - 1);
                BaseIndicator<T>::output_value[0] = variance > 0 ? std::sqrt(variance) : 0.0;
                if ((buffer.size() + 1) == period) {
                    is_done = true;
                }
                return is_done;
            }

            if (period <= 1) {
                BaseIndicator<T>::output_value[0] = 0;
                return false;
            }

            if (is_init) {
                const T removed_value = buffer.front();
                buffer.update(value, type);
                sum = sum - removed_value + value;
                sum_squared = sum_squared - (removed_value * removed_value) + (value * value);
                const T mean = sum  * period_divider_1;
                const T variance = (sum_squared - (T)period * (mean * mean)) * period_divider_2;
                BaseIndicator<T>::output_value[0] = variance > 0 ? std::sqrt(variance) : 0.0;
                return true;
            }

            buffer.update(value, type);

            if (buffer.size() <= 1) {
                BaseIndicator<T>::output_value[0] = 0;
                return false;
            }

            const std::vector<T> ts = buffer.to_vector();
            sum = 0;
            for (size_t i = 0; i < buffer.size(); ++i) {
                sum += ts[i];
            }
            const T mean = sum / (T)buffer.size();

            sum_squared = 0;
            T sum_diff = 0;
            for (size_t i = 0; i < buffer.size(); ++i) {
                const T temp = ts[i] - mean;
                sum_diff += temp * temp;
                sum_squared += ts[i] * ts[i];
            }

            const T variance = is_simple_std ? sum_diff / (T)buffer.size() : sum_diff / (T)(buffer.size() - 1);
            BaseIndicator<T>::output_value[0] = variance > 0 ? std::sqrt(variance) : 0.0;

            if (buffer.is_ready()) {
                is_init = true;
                is_done = true;
            }

            return is_done;
        }

        inline bool is_ready() const noexcept {
            return is_done;
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            buffer.reset();
            BaseIndicator<T>::output_value[0] = get_empty_value<T>();
            sum_squared = 0;
            sum = 0;
            is_init = false;
            is_done = false;
        }
    };

}; // xtechnical

#endif // XTECHNICAL_STD_DEV_HPP_INCLUDED
