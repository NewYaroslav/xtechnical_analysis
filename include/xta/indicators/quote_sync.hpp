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
#ifndef XTECHNICAL_QUOTE_SYNC_HPP_INCLUDED
#define XTECHNICAL_QUOTE_SYNC_HPP_INCLUDED

#include "../common/common.hpp"

namespace xta {

    /** \brief Класс синхронизатора котировок
     * Данный класс позволяет синхронизировать данные с разных валютных пар
     */
    template<class VAL_TYPE = double>
    class QuoteSync {
    private:

        class QuoteData {
        public:
            VAL_TYPE    value;
            uint64_t    open_date   = 0;
            uint64_t    time_ms     = 0;
            bool        is_filled   = false;

            QuoteData(
                    const VAL_TYPE &v,
                    const uint64_t d,
                    const uint64_t ms,
                    const bool f = false) :
                value(v), open_date(d), time_ms(ms), is_filled(f)  {
            };
        };

        std::vector<std::deque<QuoteData>>  m_buffer;
        std::vector<bool>                   m_update_flag;
        uint64_t                            m_timeframe         = 0;
        uint64_t                            m_last_open_date    = 0;
        uint64_t                            m_last_time_ms      = 0;
        bool                                m_auto_calc         = false;

    public:

        std::function<void(
            const size_t index,         // индекс символа
            const VAL_TYPE &value,      // значение бара или цены
            const uint64_t open_date,   // дата открытия бара
            const uint64_t delay_ms,    // задержка цены по времени
            const PriceType type,       // тип цены (внутри бара или закорытие бара)
            const bool is_update,       // флаг обновления текущего бара
            const bool is_gap)>         // флаг разрыва котировок по всем символам (например, это выходные дни)
                on_update = nullptr;

        /** \brief Конструктор синхронизатора котировок
         * \param s     Количество символов
         * \param tf    Таймфрейм
         * \param ac    Использовать авторасчет (включает автоматический вызов calc())
         */
        QuoteSync(const size_t s = 1, const size_t tf = 60, const bool ac = false) :
                m_timeframe(tf), m_auto_calc(ac) {
            m_buffer.resize(s);
            m_update_flag.resize(s, false);
        };

        virtual ~QuoteSync() = default;

        /** \brief Обновить состояние синхронизатора котировок
         * \param index     Индекс символа
         * \param value     Значение цены
         * \param time_ms   Время цены
         * \return Вернет true в случае успешного обновления состояния индикатора
         */
        inline bool update(const size_t index, const VAL_TYPE &value, const uint64_t time_ms) noexcept {
            if (index >= m_buffer.size()) return false;
            m_last_time_ms = std::max(m_last_time_ms, time_ms);
            const uint64_t timestamp = time_ms / 1000;
            // 1. Получаем время открытия бара
            const uint64_t open_date = timestamp - timestamp % m_timeframe;
            m_last_open_date = std::max(m_last_open_date, open_date);
            // 2. Запоминаем бары
            if (m_buffer[index].empty()) {
                // 2.1 Если буфер пустой, добавляем элемент
                m_buffer[index].push_back(QuoteData(value, open_date, time_ms));
            } else
            if (open_date == m_buffer[index].back().open_date) {
                // 2.2 Если буфер имеет уже данный бар, обновляем значения
                m_buffer[index].back().time_ms  = time_ms;
                m_buffer[index].back().value    = value;
            } else {
                if (open_date < m_buffer[index].back().open_date) return false;
                // 2.3 Если буфер имеет пропуски баров, заполняем его
                const uint64_t prev_date = open_date - m_timeframe;
                while (prev_date > m_buffer[index].back().open_date) {
                    m_buffer[index].push_back(
                        QuoteData(
                            m_buffer[index].back().value,
                            m_buffer[index].back().open_date + m_timeframe,
                            m_buffer[index].back().time_ms,
                            true)); // ставим флаг, что бар заполнен предыдущим значением
                }
                m_buffer[index].push_back(QuoteData(value, open_date, time_ms));
            }
            m_update_flag[index] = true;
            if (m_auto_calc) return calc();
            return true;
        }

        inline bool calc() noexcept {
            if (!on_update) return false;
            // Проверяем наступление события, когда данные есть по всем барам
            for (size_t s = 0; s < m_buffer.size(); ++s) {
                if (m_buffer[s].empty()) return false;
                if (m_buffer[s].back().open_date != m_last_open_date) return false;
            }
            // поиск минимального размера массива баров
            size_t min_size = std::numeric_limits<size_t>::max();
            for (size_t s = 0; s < m_buffer.size(); ++s) {
                min_size = std::min(min_size, m_buffer[s].size());
            }
            if (min_size > 1) {
                // вызываем обновление данных
                const size_t last_index = min_size - 2;
                for (size_t i = 0; i <= last_index; ++i) {
                    bool is_gap = true;
                    for (size_t s = 0; s < m_buffer.size(); ++s) {
                        const size_t pos = m_buffer[s].size() - min_size + i;
                        if (!m_buffer[s][pos].is_filled) {
                            is_gap = false;
                            break;
                        }
                    }
                    for (size_t s = 0; s < m_buffer.size(); ++s) {
                        const size_t pos = m_buffer[s].size() - min_size + i;
                        const uint64_t delay_ms = m_last_time_ms - m_buffer[s][pos].time_ms;
                        on_update(
                            s,
                            m_buffer[s][pos].value,
                            m_buffer[s][pos].open_date,
                            delay_ms,
                            PriceType::Close,
                            false,
                            is_gap);
                    }
                }
                // удаляем старые данные
                for (size_t s = 0; s < m_buffer.size(); ++s) {
                    m_buffer[s].erase(m_buffer[s].begin(), m_buffer[s].end() - 1);
                }
            }
            // вызываем последнее обновление для актуальных цен
            for (size_t s = 0; s < m_buffer.size(); ++s) {
                const size_t pos = m_buffer[s].size() - 1;
                const uint64_t delay_ms = m_last_time_ms - m_buffer[s][pos].time_ms;
                on_update(
                    s,
                    m_buffer[s][pos].value,
                    m_buffer[s][pos].open_date,
                    delay_ms,
                    PriceType::IntraBar,
                    m_update_flag[s],
                    false);
                m_update_flag[s] = false;
            }
            return true;
        }

        inline void reset() noexcept {
            for (auto &item : m_buffer) {
                item.clear();
            }
            for (auto &item : m_update_flag) {
                item = false;
            }
            m_last_open_date = 0;
            m_last_time_ms = 0;
        }

    };
};

#endif // QUOTE_SYNC_HPP_INCLUDED
