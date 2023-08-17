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
#ifndef XTECHNICAL_LOW_PASS_FILTER_HPP_INCLUDED
#define XTECHNICAL_LOW_PASS_FILTER_HPP_INCLUDED

#include "../common/common.hpp"

namespace xtechnical {

    /** \brief Low Pass Filter
     */
    template <class T = double>
    class LowPassFilter final : public BaseIndicator<T> {
    private:
        T m_prev_value = 0;
        T m_alfa = 0;
        T m_beta = 0;
        size_t m_period = 0;
        size_t m_count = 0;
        bool m_is_init = false;
        bool m_is_done = false;

    public:

        LowPassFilter() {};

        /** \brief Initialize the Low Pass Filter indicator
         * \param period        Period
         * \param error_signal  Error Signal
         */
        LowPassFilter(
                const size_t period,
                const T error_signal = 0.03) : m_period(period) {
            const T ntay = std::log(1.0 / error_signal);
            m_alfa = std::exp(-ntay / (T)period);
            m_beta = 1.0 - m_alfa;
        }

        virtual ~LowPassFilter() = default;

        /** \brief Update the state of the indicator
         * \param value New value for the indicator, such as price
         * \param type  Price type (affects the indicator's mode of operation - intra-bar or end-of-bar)
         * \return Returns true in case of success
         */
        inline bool update(const T value, const PriceType type = PriceType::Close) noexcept {
            if (m_period <= 1) {
                BaseIndicator<T>::output_value[0] = value;
                return true;
            }

            if (is_init) {
                BaseIndicator<T>::output_value[0] = m_alfa * m_prev_value + m_beta * value;
                if (type == PriceType::IntraBar) {
                    if ((m_count + 1) == m_period) {
                        m_is_done = true;
                    }
                    return m_is_done;
                }
                m_prev_value = BaseIndicator<T>::output_value[0];
                if (m_count < m_period) {
                    ++m_count;
                    if (m_count == m_period) {
                        m_is_done = true;
                    }
                }
                return m_is_done;
            }
            if (type == PriceType::IntraBar) return false;
            m_prev_value = value;
            m_is_init = true;
            return true;
        }

        inline bool is_ready() const noexcept {
            if (m_period <= 1) return true;
            return m_is_done;
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            BaseIndicator<T>::output_value[0] = get_empty_value<T>();
            m_count = 0;
            m_prev_value = 0;
            m_is_init = false;
            m_is_done = false;
        }
    };

}; // xtechnical

#endif // XTECHNICAL_LOW_PASS_FILTER_HPP_INCLUDED
