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
#ifndef XTECHNICAL_CCI_HPP_INCLUDED
#define XTECHNICAL_CCI_HPP_INCLUDED

#include "circular_buffer.hpp"

namespace xta {

    /** \brief Commodity Channel Index
     */
    template <class T = double>
    class CCI final : public BaseIndicator<T> {
    private:
        CircularBuffer<T> buffer;
        T ad = 0;
        T sum = 0;
        T period_divider = 0;
        size_t period = 0;
        bool is_init = false;
        bool is_done = false;
    public:

        CCI() {};

        /** \brief Initialize the CCI (Commodity Channel Index) indicator
         * \param p Period
         */
        CCI(const size_t p) :
                buffer(std::max(std::size_t(1),p)), period(p) {
            period_divider = 1.0 / (T)period;
        }

        virtual ~CCI() = default;

        /** \brief Update the state of the indicator
         * \param value New value for the indicator, such as price
         * \param type  Price type (affects the indicator's mode of operation - intra-bar or end-of-bar)
         * \return Returns true in case of success
         */
        inline bool update(const T value, const PriceType type = PriceType::Close) noexcept {
            if (period <= 1) {
                BaseIndicator<T>::output_value[0] = 0;
                return false;
            }
            const T coeff = 0.015;
            if (type == PriceType::IntraBar) {
                if (is_init) {
                    const T removed_value = buffer.front();
                    const T mean = (sum - removed_value + value) * period_divider;
                    const T mad_value = (ad + std::abs(value - mean) - std::abs(removed_value - mean)) * period_divider;
                    BaseIndicator<T>::output_value[0] = mad_value == 0 ? 0 : (value - mean) / (coeff * mad_value);
                    return true;
                }

                const std::vector<T> ts = buffer.to_vector();
                T mean = 0;
                for (size_t i = 0; i < buffer.size(); ++i) {
                    mean += ts[i];
                }
                mean = (mean + value) / (T)(buffer.size() + 1);

                T mad_value = 0;
                for (size_t i = 0; i < buffer.size(); ++i) {
                    mad_value += std::abs(ts[i] - mean);
                }
                mad_value += std::abs(value - mean);
                mad_value /= (T)(buffer.size() + 1);
                BaseIndicator<T>::output_value[0] = mad_value == 0 ? 0 : (value - mean) / (coeff * mad_value);
                is_done = ((buffer.size() + 1) == period);
                return is_done;
            }

            if (is_init) {
                const T removed_value = buffer.front();
                buffer.update(value, type);
                sum = sum - removed_value + value;
                const T mean = sum * period_divider;
                ad = ad + std::abs(value - mean) - std::abs(removed_value - mean);
                const T mad_value = ad * period_divider;
                BaseIndicator<T>::output_value[0] = mad_value == 0 ? 0 : (value - mean) / (coeff * mad_value);
                return true;
            }

            buffer.update(value, type);

            const std::vector<T> ts = buffer.to_vector();
            sum = 0;
            for (size_t i = 0; i < buffer.size(); ++i) {
                sum += ts[i];
            }
            const T mean = sum /(T)buffer.size();

            ad = 0;
            for (size_t i = 0; i < buffer.size(); ++i) {
                ad += std::abs(ts[i] - mean);
            }

            const T mad_value = ad / (T)buffer.size();
            BaseIndicator<T>::output_value[0] = mad_value == 0 ? 0 : (value - mean) / (coeff * mad_value);

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
            ad = 0;
            sum = 0;
            is_init = false;
            is_done = false;
        }
    };
};

#endif // XTECHNICAL_CCI_HPP_INCLUDED
