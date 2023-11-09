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
#ifndef XTECHNICAL_BB_HPP_INCLUDED
#define XTECHNICAL_BB_HPP_INCLUDED

#include "circular_buffer.hpp"

namespace xta {

    enum class BbLineType {
        Upper = 0,
        Lower = 1,
        Mean = 2,
        StdDev = 3,
    };

    /** \brief Z-score
     */
    template <class T = double>
    class BollingerBands final : public BaseIndicator<T> {
    private:
        CircularBuffer<T> buffer;
        DelayLine<T> delay_line;
        T multiplier = 0;
        T sum_squared = 0;
        T sum = 0;
        T period_divider_1 = 0;
        T period_divider_2 = 0;
        size_t period = 0;
        bool is_simple_std = false;
        bool is_init = false;
        bool is_done = false;
    public:

        BollingerBands() : BaseIndicator<T>(4) {};

        /** \brief Initialize the Bollinger Bands indicator
         * \param arg_period    Period
         * \param arg_backshift Backshift
         * \param arg_simple_std    Use Simple Standard Deviation
         */
        BollingerBands(
                const size_t arg_period,
                const double arg_multiplier,
                const size_t arg_backshift = 0,
                const bool arg_simple_std = false) :
                    BaseIndicator<T>(4), buffer(arg_period),
                    delay_line(arg_backshift), multiplier(arg_multiplier),
                    period(arg_period), is_simple_std(arg_simple_std) {
            period_divider_1 = (1.0 / (T)period);
            period_divider_2 = is_simple_std ? period_divider_1 : (1.0 / (T)(period - 1));
        }

        virtual ~BollingerBands() = default;

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
            delay_line.update(value, type);
            if (!delay_line.is_ready()) return false;
            const T delay_value = delay_line.get();
            if (type == PriceType::IntraBar) {
                if (period <= 1) {
                    BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Upper)] = delay_value;
                    BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Lower)] = delay_value;
                    BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Mean)] = delay_value;
                    BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::StdDev)] = 0.0;
                    return false;
                }

                if (is_init) {
                    const T removed_value = buffer.front();
                    const T mean = (sum - removed_value + delay_value) * period_divider_1;
                    const T variance = ((sum_squared - (removed_value * removed_value) + (delay_value * delay_value)) - (T)period * (mean * mean)) * period_divider_2;
                    const T std_dev = variance > 0 ? std::sqrt(variance) : 0.0;
                    const T std_dev_mult = std_dev * multiplier;

                    BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Upper)] = mean + std_dev_mult;
                    BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Lower)] = mean - std_dev_mult;
                    BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Mean)] = mean;
                    BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::StdDev)] = std_dev;
                    return true;
                }

                if (buffer.size() == 0) {
                    BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Upper)] = delay_value;
                    BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Lower)] = delay_value;
                    BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Mean)] = delay_value;
                    BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::StdDev)] = 0.0;
                    return false;
                }

                const size_t count = buffer.size() + 1;

                const std::vector<T> ts = buffer.to_vector();
                T mean = 0;
                for (size_t i = 0; i < buffer.size(); ++i) {
                    mean += ts[i];
                }
                mean += delay_value;
                mean /= (T)count;

                T sum_diff = 0;
                for (size_t i = 0; i < buffer.size(); ++i) {
                    const T temp = ts[i] - mean;
                    sum_diff += temp * temp;
                }
                const T temp = delay_value - mean;
                sum_diff += temp * temp;
                const T variance = is_simple_std ? sum_diff / (T)count : sum_diff / (T)(count - 1);
                const T std_dev = variance > 0 ? std::sqrt(variance) : 0.0;
                const T std_dev_mult = std_dev * multiplier;

                BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Upper)] = mean + std_dev_mult;
                BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Lower)] = mean - std_dev_mult;
                BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Mean)] = mean;
                BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::StdDev)] = std_dev;

                if ((buffer.size() + 1) == period) {
                    is_done = true;
                }
                return is_done;
            }

            if (period <= 1) {
                BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Upper)] = delay_value;
                BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Lower)] = delay_value;
                BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Mean)] = delay_value;
                BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::StdDev)] = 0.0;
                return false;
            }

            if (is_init) {
                const T removed_value = buffer.front();
                buffer.update(delay_value, type);
                sum = sum - removed_value + delay_value;
                sum_squared = sum_squared - (removed_value * removed_value) + (delay_value * delay_value);
                const T mean = sum  * period_divider_1;
                const T variance = (sum_squared - (T)period * (mean * mean)) * period_divider_2;
                const T std_dev = variance > 0 ? std::sqrt(variance) : 0.0;
                const T std_dev_mult = std_dev * multiplier;

                BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Upper)] = mean + std_dev_mult;
                BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Lower)] = mean - std_dev_mult;
                BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Mean)] = mean;
                BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::StdDev)] = std_dev;
                return true;
            }

            buffer.update(delay_value, type);

            if (buffer.size() <= 1) {
                BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Upper)] = delay_value;
                BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Lower)] = delay_value;
                BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Mean)] = delay_value;
                BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::StdDev)] = 0.0;
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
            const T std_dev = variance > 0 ? std::sqrt(variance) : 0.0;
            const T std_dev_mult = std_dev * multiplier;
            BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Upper)] = mean + std_dev_mult;
            BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Lower)] = mean - std_dev_mult;
            BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Mean)] = mean;
            BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::StdDev)] = std_dev;

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
            delay_line.reset();
            BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Upper)] = get_empty_value<T>();
            BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Lower)] = get_empty_value<T>();
            BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::Mean)] = get_empty_value<T>();
            BaseIndicator<T>::output_value[static_cast<size_t>(BbLineType::StdDev)] = get_empty_value<T>();
            sum_squared = 0;
            sum = 0;
            is_init = false;
            is_done = false;
        }
    };

}; // xtechnical

#endif // XTECHNICAL_BB_HPP_INCLUDED
