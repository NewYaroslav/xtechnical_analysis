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
#ifndef XTECHNICAL_DATE_BUFFER_HPP_INCLUDED
#define XTECHNICAL_DATE_BUFFER_HPP_INCLUDED

#include "../common/common.hpp"

namespace xtechnical {

    /** \brief Класс буфера по дате
     * Данный класс позволяет синхронизировать данные с разных валютных пар
     */
    template<class VAL_TYPE = double>
    class DateBuffer final : public BaseIndicator<VAL_TYPE> {
    private:
        std::vector<std::deque<std::pair<VAL_TYPE, uint64_t>>> m_buffer;
        uint64_t m_size = 0;
        uint64_t m_period = 0;
        uint64_t m_timeframe = 0;
        uint64_t m_start_time = 0;
        bool     m_periodic_mode = false;

    public:

        /** \brief Конструктор DateBuffer
         * При использовальнии периода, данные накапливаются в пределах периода.
         * Если период не используется, то класс работает как скользящее окно.
         * Пробелы в данных заполняются. Если по какому то из буферов данные не полные,
         * то метод is_ready() для проверки готовности класса возвращает false
         * \param s     Размер буфера или периода, картен таймфрейму
         * \param tf    Таймфрейм
         * \param pm    Режим периода (указать false, если не используется)
         * \param nb    Количество буферов
         */
        DateBuffer(const size_t s = 1, const size_t tf = 60, const size_t pm = false, const size_t nb = 1) :
                BaseIndicator<VAL_TYPE>(nb), m_size(s),  m_period(s * tf), m_timeframe(tf), m_periodic_mode(pm) {
            if (!m_periodic_mode) m_period -= tf;
            m_buffer.resize(nb);
        };

        virtual ~DateBuffer() = default;

        /** \brief
         *
         * \param index
         * \param value
         * \param timestamp
         * \param type
         * \return Вернет true в случае успешного обновления состояния индикатора
         */
        inline bool update(const size_t index, const VAL_TYPE value, const uint64_t timestamp, const PriceType type = PriceType::Close) noexcept {
            if (index >= m_buffer.size()) return false;
            // Получаем время открытия бара
            const uint64_t open_time = timestamp - timestamp % m_timeframe;
            // Обновляем начальное время
            if (m_periodic_mode) {
                const uint64_t period_time = timestamp - timestamp % m_period;
                m_start_time = std::max(m_start_time, period_time);
                // для режима периода обновляем размер массива
                const uint64_t period_size = ((open_time - m_start_time) / m_timeframe) + 1;
                if (period_time == open_time) m_size = 1;
                else m_size = std::max(m_size, period_size);
            } else {
                if (open_time >= m_period) m_start_time = std::max(m_start_time, (open_time - m_period));
            }
            // Добавляем элемент
            if (m_buffer[index].empty()) {
                m_buffer[index].push_back(std::make_pair(value, open_time));
                BaseIndicator<VAL_TYPE>::output_value[index] = value;
                return true;
            }
            if (open_time == m_buffer[index].back().second) {
                m_buffer[index].back().first = value;
            } else {
                const uint64_t prev_time = open_time - m_timeframe;
                while (prev_time > m_buffer[index].back().second) {
                    m_buffer[index].push_back(std::make_pair(
                        m_buffer[index].back().first,
                        m_buffer[index].back().second + m_timeframe));
                }
                m_buffer[index].push_back(std::make_pair(value, open_time));
            }
            /*  Удаляем старые элементы
                Как это работает:
                Если есть элемент, расположенный ниже даты начала буфера
                то такой элемент нужно удалить, но только в том случае
                если в буфере есть еще элементы
             */
            auto it_front = m_buffer[index].begin();
            auto it_next = std::next(it_front);
            while ( it_next != m_buffer[index].end() &&
                    it_front->second < m_start_time) {
                m_buffer[index].erase(it_front);
                it_front = m_buffer[index].begin();
                it_next = std::next(it_front);
            }
            BaseIndicator<VAL_TYPE>::output_value[index] = m_buffer[index].front().first;
            return true;
        }

        inline bool update(const VAL_TYPE in_val, const PriceType type = PriceType::Close) noexcept {
            return false;
        }

        inline std::vector<VAL_TYPE> to_vector(const size_t index = 0) const noexcept {
            std::vector<VAL_TYPE> temp(m_buffer[index].size());
            for (size_t i = 0; i < m_buffer[index].size(); ++i) {
                temp[i] = m_buffer[index][i].first;
            }
            return std::move(temp);
        }

        inline const VAL_TYPE front(const size_t index = 0) const noexcept {
            return m_buffer[index].front().first;
        }

        /** \brief Доступ к последнему элементу
         * \return Возвращает ссылку на первый элемент циклического буфера
         */
        inline const VAL_TYPE back(const size_t index = 0) const noexcept {
            return m_buffer[index].back().first;
        }

        inline bool is_ready() const noexcept {
            for (auto &item : m_buffer) {
                if (item.empty()) return false;
                if (m_start_time != item[0].second) return false;
                if (item.size() != m_size) return false;
            }
            return true;
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            for (auto &item : BaseIndicator<VAL_TYPE>::output_value) {
                item = get_empty_value<VAL_TYPE>();
            }
            for (auto &item : m_buffer) {
                item.clear();
            }
            m_start_time = 0;
        }

    };
};

#endif // XTECHNICAL_DATE_BUFFER_HPP_INCLUDED
