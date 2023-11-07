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
#ifndef XTECHNICAL_MIN_MAX_DIFF_HPP_INCLUDED
#define XTECHNICAL_MIN_MAX_DIFF_HPP_INCLUDED

#include "min_max.hpp"

namespace xta {

    enum class MinMaxDiffLineType {
        DiffLine    = 0,
        MaxLine     = 1,
        MinLine     = 2,
    };

    /** \brief Cкользящий Min Max Difference
     */
    template <class T = double>
    class MinMaxDiff final : public BaseIndicator<T> {
    private:
        MinMax<T>       m_min_max;
        DelayLine<T>    m_delay_line;

    public:

        MinMaxDiff() : BaseIndicator<T>(3) {};

        /** \brief Конструктор скользящего Min Max Difference
         * \param arg_period    Period
         * \param arg_backshift Backshift
         */
        MinMaxDiff(const size_t arg_period, const size_t arg_backshift = 0) :
                BaseIndicator<T>(3),
                m_min_max(arg_period, arg_backshift), m_delay_line(1) {
        }

        virtual ~MinMaxDiff() = default;

        /** \brief Update the state of the indicator
         * \param value New value for the indicator, such as price
         * \param type  Price type (affects the indicator's mode of operation - intra-bar or end-of-bar)
         * \return Returns true in case of success
         */
        inline bool update(const T value, const PriceType type = PriceType::Close) noexcept {
            m_delay_line.update(value, type);
            if (!m_delay_line.is_ready()) return false;
            BaseIndicator<T>::output_value[0] = std::abs(value - m_delay_line.get());
            m_min_max.update(BaseIndicator<T>::output_value[0], type);
            if (!m_min_max.is_ready()) return false;
            BaseIndicator<T>::output_value[1] = m_min_max.get_max();
            BaseIndicator<T>::output_value[2] = m_min_max.get_min();
            return true;
        }

        inline bool is_ready() const noexcept {
            return m_min_max.is_ready();
        }

        inline T get_diff() const noexcept {
            return BaseIndicator<T>::output_value[0];
        }

        inline T get_max() const noexcept {
            return BaseIndicator<T>::output_value[1];
        }

        inline T get_min() const noexcept {
            return BaseIndicator<T>::output_value[2];
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            m_delay_line.reset();
            m_min_max.reset();
            for (auto &item : BaseIndicator<T>::output_value) {
                item = get_empty_value<T>();
            }
        }

    }; // MinMaxDiff

}; // xtechnical

#endif // XTECHNICAL_MIN_MAX_DIFF_HPP_INCLUDED
