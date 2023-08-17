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
#ifndef XTECHNICAL_MFCSI_HPP_INCLUDED
#define XTECHNICAL_MFCSI_HPP_INCLUDED

#include "date_buffer.hpp"
#include <Eigen/Dense>

namespace xtechnical {

    enum class MFCSI_TWI8_PairIndex {
        AUDUSD = 0,
        EURUSD,
        GBPUSD,
        NZDUSD,
        USDCAD,
        USDCHF,
        USDJPY,
    };

    enum class MFCSI_TWI8_CurrencyIndex {
        USD = 0,
        EUR,
        GBP,
        JPY,
        CHF,
        AUD,
        CAD,
        NZD,
    };

    /** \brief MFCSI_TWI8 (Multi-Functional Currency Strength Indicator, Trade-Weighted Index)
     * Рассчитывает индексы валют USD,EUR,GBP,JPY,CHF,AUD,CAD,NZD
     * На основе валютных пар: AUDUSD,EURUSD,GBPUSD,NZDUSD,USDCAD,USDCHF,USDJPY
     * Используется метод Trade-Weighted Index
     * Индикатор работает либо в режиме скользящего окна, либо в режиме периодичного обнуления
     */
    template<class T = double>
    class MFCSI_TWI8 final : public BaseIndicator<T> {
    private:
        DateBuffer<T>       m_date_buffer;
        bool                m_reverse = false;
        bool                m_auto_calc = false;
        bool                m_ready = false;

    public:

        MFCSI_TWI8() {};

        /** \brief Конструктор MFCSI_TWI8
         * \param s     Размер окна или периода
         * \param tf    Таймфрейм
         * \param pm    Режим работы индикатора: если false, то скользящее окно, иначе периодичное обнуление
         * \param r     Реверс значения индикатора
         * \param ac    Включить авторасчет после каждого вызова update. Если false, вам нужно самим вызывать calc()!
         */
        MFCSI_TWI8(const size_t s = 1440, const size_t tf = 60, const bool pm = true, const bool r = false, const bool ac = false) :
            BaseIndicator<T>(8), m_date_buffer(s, tf, pm, 7), m_reverse(r), m_auto_calc(ac) {
        }

        virtual ~MFCSI_TWI8() = default;

        /** \brief Обновить состояние индикатора
         * \param index     Номер валютной пары, порядок от 0 до 6 (AUDUSD,EURUSD,GBPUSD,NZDUSD,USDCAD,USDCHF,USDJPY)
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

        /** \brief Произвести расчет индикатора
         * При отключенном авторасчете индикатора данный метод вызывать обязательно!
         * Вызывать метод нужно после того, как все значения котировок были загружены методом update
         * \return Вернет true после расчета. Готовность индкатора можно отслеживать без is_ready()
         */
        inline bool calc() noexcept {
            if (!m_date_buffer.is_ready()) return false;
            std::array<T, 8> currency_indexes;
            currency_indexes[0] = 1.0;
            // для оптмизации получаем коэффициенты
            std::array<T, 7> coeff;
            for (size_t i = 0; i < 4; ++i) {
                coeff[i] = m_date_buffer.back(i) / m_date_buffer.front(i);
            }
            for (size_t i = 4; i < 7; ++i) {
                coeff[i] = m_date_buffer.front(i) / m_date_buffer.back(i);
            }
            // вычисляем индекс USD
            for (size_t i = 0; i < 7; ++i) {
                currency_indexes[0] *= coeff[i];
            }
            currency_indexes[0] = 100 * std::pow(currency_indexes[0],0.125);
            // вычисляем остальные индексы
            currency_indexes[1] = currency_indexes[0] * coeff[1];
            currency_indexes[2] = currency_indexes[0] * coeff[2];
            currency_indexes[3] = currency_indexes[0] * coeff[6];
            currency_indexes[4] = currency_indexes[0] * coeff[5];
            currency_indexes[5] = currency_indexes[0] * coeff[0];
            currency_indexes[6] = currency_indexes[0] * coeff[4];
            currency_indexes[7] = currency_indexes[0] * coeff[3];
            // делаем реверс, если нужно
            if (m_reverse) {
                for (size_t i = 0; i < currency_indexes.size(); ++i) {
                    currency_indexes[i] = 200 - currency_indexes[i];
                }
            }
            // Запоминаем результат
            for (size_t i = 0; i < 8; ++i) {
                BaseIndicator<T>::output_value[i] = currency_indexes[i];
            }
            m_ready = true;
            return true;
        }

        /** \brief Обновить состояние индикатора
         * Данная версия метода в данном индикаторе не работает
         */
        inline bool update(const T in_val, const PriceType type = PriceType::Close) noexcept {
            return false;
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
            m_ready = false;
        }
    }; // MFCSI_TWI8

    enum class MFCSI_ABS8_PairIndex {
        AUDCAD = 0,
        CADJPY,
        USDJPY,
        USDCHF,
        EURCHF,
        EURGBP,
        GBPNZD,
    };

    enum class MFCSI_ABS8_CurrencyIndex {
        AUD = 0,
        CAD,
        JPY,
        USD,
        CHF,
        EUR,
        GBP,
        NZD,
    };

    /** \brief MFCSI_ABS8 (Multi-Functional Currency Strength Indicator, Индексы валют относительно ABS)
     * По мотивам статьи: https://habr.com/ru/articles/450368/
     * Индикатор использует валютные пары: AUDCAD, CADJPY, USDJPY, USDCHF, EURCHF, EURGBP, GBPZND
     * Рассчитывает значения валют относительно ABS: AUD, CAD, JPY, USD, CHF, EUR, GBP, NZD
     * ABS - абсолютная валюта
     */
    template<class T = double>
    class MFCSI_ABS8 final : public BaseIndicator<T> {
    private:
        DateBuffer<T>           m_date_buffer;
        Eigen::Matrix<T,7,8>    m_matrix_f; // обратная матрица 7х8

        bool                m_auto_calc = false;
        bool                m_ready = false;

        void fill_matrix() {
            // заполняем положительные значения под диагональю
            for (int y = 0; y < m_matrix_f.rows(); ++y) {
                for (int x = 0; x <= y; ++x) {
                    // Задание значения для каждой ячейки
                    m_matrix_f(y, x) = (T)(m_matrix_f.rows() - y) / (T)m_matrix_f.cols();
                }
            }
            // заполняем отрицательные значения над диагональю
            for (int y = 0; y < m_matrix_f.rows(); ++y) {
                for (int x = y + 1; x < m_matrix_f.cols(); ++x) {
                    // Задание значения для каждой ячейки
                    m_matrix_f(y, x) = (T)(-y - 1) / (T)m_matrix_f.cols();
                }
            }
        }

    public:

        MFCSI_ABS8() {};

        /** \brief Конструктор MFCSI_ABS8
         * \param tf    Таймфрейм
         * \param ac    Включить авторасчет после каждого вызова update. Если false, вам нужно самим вызывать calc()!
         */
        MFCSI_ABS8(const size_t tf = 60, const bool ac = false) :
            BaseIndicator<T>(8), m_date_buffer(1, tf, false, 7), m_auto_calc(ac) {
            fill_matrix();
        }

        virtual ~MFCSI_ABS8() = default;

        /** \brief Обновить состояние индикатора
         * \param index     Номер валютной пары, порядок от 0 до 6 (AUDCAD, CADJPY, USDJPY, USDCHF, EURCHF, EURGBP, GBPZND)
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

        /** \brief Произвести расчет индикатора
         * При отключенном авторасчете индикатора данный метод вызывать обязательно!
         * Вызывать метод нужно после того, как все значения котировок были загружены методом update
         * \return Вернет true после расчета. Готовность индкатора можно отслеживать без is_ready()
         */
        inline bool calc() noexcept {
            if (!m_date_buffer.is_ready()) return false;
            // матрица 1х7
            Eigen::Matrix<T,1,7> price_vec;
            // AUDCAD, CADJPY, JPYUSD, USDCHF, CHFEUR, EURGBP, GBPZND
            price_vec(0) = m_date_buffer.get(0);
            price_vec(1) = m_date_buffer.get(1);
            price_vec(2) = 1.0/m_date_buffer.get(2);
            price_vec(3) = m_date_buffer.get(3);
            price_vec(4) = 1.0/m_date_buffer.get(4);
            price_vec(5) = m_date_buffer.get(5);
            price_vec(6) = m_date_buffer.get(6);

            price_vec.array().log();
            Eigen::Matrix<T,1,Eigen::Dynamic> result_vec = price_vec * m_matrix_f;
            result_vec.array().exp();
            for (size_t i = 0; i < result_vec.cols(); ++i) {
                BaseIndicator<T>::output_value[i] = result_vec(i);
            }
            m_ready = true;
            return true;
        }

        /** \brief Обновить состояние индикатора
         * Данная версия метода в данном индикаторе не работает
         */
        inline bool update(const T in_val, const PriceType type = PriceType::Close) noexcept {
            return false;
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
            m_ready = false;
        }
    }; // MFCSI_ABS
};

#endif // XTECHNICAL_MFCSI_HPP_INCLUDED
