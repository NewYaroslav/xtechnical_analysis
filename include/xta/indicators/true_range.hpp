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
#ifndef XTECHNICAL_TRUE_RANGE_HPP_INCLUDED
#define XTECHNICAL_TRUE_RANGE_HPP_INCLUDED

#include "delay_line.hpp"

namespace xta {

    /** \brief True Range
     */
    template <class T = double>
    class TrueRange final : public BaseIndicator<T> {
    private:
        DelayLine<T> m_delay_line;

    public:

        TrueRange() : BaseIndicator<T>(1), m_delay_line(1) {};

        virtual ~TrueRange() = default;

        /** \brief Обновить состояние индикатора
         * \param open  НЕ ИСПОЛЬЗУЕТСЯ! Цена открытия бара
         * \param high  Наивысшая цена бара
         * \param low   Наинизшая цена бара
         * \param close Цена закрытия бара
         * \param type  Тип цены
         * \return Вернет true в случае готовности индикатора
         */
        inline bool update(const T open, const T high, const T low, const T close, const PriceType type = PriceType::Close) noexcept {
            m_delay_line.update(close, type);
            if (!m_delay_line.is_ready()) return false;
            BaseIndicator<T>::output_value[0] = std::max(std::max(high - low, high - m_delay_line.get()), m_delay_line.get() - low);
            return true;
        }

        inline bool update(const T value, const PriceType type = PriceType::Close) noexcept {
            m_delay_line.update(value, type);
            if (!m_delay_line.is_ready()) return false;
            BaseIndicator<T>::output_value[0] = std::abs(value - m_delay_line.get());
            return true;
        }

        inline bool is_ready() const noexcept {
            return m_delay_line.is_ready();
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            BaseIndicator<T>::output_value[0] = get_empty_value<T>();
            m_delay_line.reset();
        }
    };
}; // xtechnical

#endif // XTECHNICAL_TRUE_RANGE_HPP_INCLUDED
