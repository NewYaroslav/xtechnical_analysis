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
#ifndef XTECHNICAL_USDX_HPP_INCLUDED
#define XTECHNICAL_USDX_HPP_INCLUDED

#include "date_buffer.hpp"

namespace xtechnical {

    enum USDX_PairIndex {
        EURUSD = 0,
        USDJPY,
        GBPUSD,
        USDCAD,
        USDCHF,
        USDSEK,
    };

    /** \brief USDX
     * Рассчитывает индекс доллара (USDX или DXY), используя следующие валютные пары:
     * EURUSD, USDJPY, GBPUSD, USDCAD, USDCHF, USDSEK.
     * Описание: https://www.earnforex.com/ru/metatrader-%D0%B8%D0%BD%D0%B4%D0%B8%D0%BA%D0%B0%D1%82%D0%BE%D1%80%D1%8B/USDX/
     */
    template<class T = double>
    class USDX final : public BaseIndicator<T> {
    private:
        DateBuffer<T>   m_date_buffer;
        bool            m_auto_calc = false;
        bool            m_ready = false;

    public:

        USDX() {};

        /** \brief Конструктор USDX
         * \param tf    Таймфрейм
         * \param ac    Включить авторасчет после каждого вызова update. Если false, вам нужно самим вызывать clac()!
         */
        USDX(const size_t tf = 60, const bool ac = false) :
            BaseIndicator<T>(1), m_date_buffer(1, tf, false, 6), m_auto_calc(ac)  {
        }

        virtual ~USDX() = default;

        /** \brief Обновить состояние индикатора
         * \param index     Номер валютной пары, порядок от 0 до 5 (EURUSD, USDJPY, GBPUSD, USDCAD, USDCHF, USDSEK)
         * \param value     Курс валютной пары
         * \param timestamp Метка времени бара или тика
         * \param type      Тип бара (внутри бара или закрытие бара) - в данном индикаторе не влияет на работу
         * \return Вернет true в случае успешного обновления индикатора. При отключенном авторасчете готовность индкатора отслеживать можно только по is_ready() или результату calc()
         */
        inline bool update(const size_t index, const T value, const uint64_t timestamp, const PriceType type = PriceType::Close) noexcept {
            if (!m_auto_calc) {
                m_ready = false;
                return m_date_buffer.update(index, value, timestamp, type);
            }
            if (!m_date_buffer.update(index, value, timestamp, type)) return false;
            return calc();
        }

        /** \brief Обновить состояние индикатора
         * Данная версия метода в данном индикаторе не работает
         */
        inline bool update(const T in_val, const PriceType type = PriceType::Close) noexcept {
            return false;
        }

        /** \brief Произвести расчет индикатора
         * При отключенном авторасчете индикатора данный метод вызывать обязательно!
         * Вызывать метод нужно после того, как все значения котировок были загружены методом update
         * \return Вернет true после расчета. Готовность индкатора можно отслеживать без is_ready()
         */
        inline bool calc() noexcept {
            if (!m_date_buffer.is_ready()) return false;
            // EURUSD, USDJPY, GBPUSD, USDCAD, USDCHF, USDSEK
            // 50,14348112 × EURUSD-0,576 × USDJPY0,136 × GBPUSD-0,119 × USDCAD 0,091 × USDSEK 0,042 × USDCHF 0,036
            const std::vector<double> coefficients = {
                -0.576, 0.136, -0.119, 0.091, 0.036, 0.042
            };
            BaseIndicator<T>::output_value[0] = 50.14348112;
            for (size_t i= 0; i < coefficients.size(); ++i) {
                BaseIndicator<T>::output_value[0] *= std::pow(m_date_buffer.back(i), coefficients[i]);
            }
            m_ready = true;
            return true;
        }

        /** \brief Проверить готовность индикатора
         * \return Если индикатор готов к работе, вернет true
         */
        inline bool is_ready() const noexcept {
            if (m_auto_calc) return m_date_buffer.is_ready();
            return m_ready;
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            for (auto &item : BaseIndicator<T>::output_value) {
                item = get_empty_value<T>();
            }
            m_date_buffer.reset();
        }
    }; // USDX
};

#endif // USDX_HPP_INCLUDED
