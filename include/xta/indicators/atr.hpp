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
#ifndef XTECHNICAL_ATR_HPP_INCLUDED
#define XTECHNICAL_ATR_HPP_INCLUDED

#include "true_range.hpp"
#include "sma.hpp"

namespace xta {

    /** \brief Average True Range
     */
    template <class T = double, template <class...> class MA_TYPE = xta::SMA>
    class ATR final : public BaseIndicator<T> {
    private:
        MA_TYPE<T>      ma_atr;
		TrueRange<T>    m_tr;

    public:

        ATR() : BaseIndicator<T>(1) {};

        ATR(const size_t p) : BaseIndicator<T>(1), ma_atr(p) {
        }

        virtual ~ATR() = default;

        /** \brief Обновить состояние индикатора
         * \param open  НЕ ИСПОЛЬЗУЕТСЯ! Цена открытия бара
         * \param high  Наивысшая цена бара
         * \param low   Наинизшая цена бара
         * \param close Цена закрытия бара
         * \param type  Тип цены
         * \return Вернет true в случае готовности индикатора
         */
        inline bool update(const T open, const T high, const T low, const T close, const PriceType type = PriceType::Close) noexcept {
            m_tr.update(open, high, low, close, type);
            ma_atr.update(m_tr.get(), type);
            if (!ma_atr.is_ready()) return false;
            BaseIndicator<T>::output_value[0] = ma_atr.get();
            return true;
        }

        inline bool update(const T value, const PriceType type = PriceType::Close) noexcept {
            m_tr.update(value, type);
            ma_atr.update(m_tr.get(), type);
            if (!ma_atr.is_ready()) return false;
            BaseIndicator<T>::output_value[0] = ma_atr.get();
            return true;
        }

        inline bool is_ready() const noexcept {
            return ma_atr.is_ready();
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            BaseIndicator<T>::output_value[0] = get_empty_value<T>();
            m_tr.reset();
            ma_atr.reset();
        }
    };
}; // xtechnical

#endif // ATR_HPP_INCLUDED
