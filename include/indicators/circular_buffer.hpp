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
#ifndef XTECHNICAL_CIRCULAR_BUFFER_V2_HPP_INCLUDED
#define XTECHNICAL_CIRCULAR_BUFFER_V2_HPP_INCLUDED

#include "../common/common.hpp"

namespace xtechnical {

    /** \brief Класс циклического буфера
     */
    template<class T>
    class CircularBuffer final : public BaseIndicator<T> {
    private:

        std::vector<T> buffer;      /**< Основной буфер */
        std::vector<T> buffer_test; /**< Буфер для теста */
        size_t buffer_size;         /**< Размер буфера */
        size_t buffer_size_div2;    /**< Индекс середины массива */
        size_t buffer_offset;       /**< Смещение в буфере для размера массива не кратного степени двойки */
        size_t count;               /**< Количество элементов в буфере */
        size_t count_test;          /**< Количество элементов в буфере для теста */
        size_t offset;              /**< Смещение в буфере */
        size_t offset_test;
        size_t mask;                /**< Маска */
        bool is_power_of_two;       /**< Флаг степени двойки */
        bool is_test;               /**< Флаг теста */

        inline const size_t cpl2(size_t x) const {
            x = x - 1;
            x = x | (x >> 1);
            x = x | (x >> 2);
            x = x | (x >> 4);
            x = x | (x >> 8);
            x = x | (x >> 16);
            return x + 1;
        }

        inline const bool check_power_of_two(const size_t value) const {
            return value && !(value & (value - 1));
        }

        template <typename ContainerT>
        typename std::enable_if<std::is_same<ContainerT, std::deque<typename ContainerT::value_type>>::value ||
                                std::is_same<ContainerT, std::vector<typename ContainerT::value_type>>::value>::type
        reserve_buffer(ContainerT& container, std::size_t size) {
            container.resize(size);
        }

        template <typename ContainerT>
        typename std::enable_if<!std::is_same<ContainerT, std::deque<typename ContainerT::value_type>>::value &&
                                !std::is_same<ContainerT, std::vector<typename ContainerT::value_type>>::value>::type
        reserve_buffer(ContainerT&, std::size_t) {
            // Ничего не делаем для контейнеров, не имеющих reserve
        }

        template <typename ContainerT>
        void copy_buffer(ContainerT& container) noexcept {
            const size_t max_index = buffer_size - 1;
            size_t start_index = 0;
            size_t stop_index = 0;

            if (is_test) {
                if (full()) {
                    if (is_power_of_two) {
                        start_index = (offset_test) & mask;
                        stop_index = (offset_test + max_index) & mask;
                    } else {
                        start_index = (offset_test - buffer_offset) & mask;
                        stop_index = (offset_test + (max_index - buffer_offset)) & mask;
                    }
                } else {
                    stop_index = max_index;
                }

                if (start_index > stop_index) {
                    for (size_t i = start_index; i < buffer_size; ++i) {
                        container[i - start_index] = buffer_test[i];
                    }
                    for (size_t i = 0; i <= stop_index; ++i) {
                        container[buffer_size - start_index + i] = buffer_test[i];
                    }
                } else {
                    for (size_t i = start_index; i <= stop_index; ++i) {
                        container[i - start_index] = buffer_test[i];
                    }
                }
            } else {
                if (full()) {
                    if (is_power_of_two) {
                        start_index = offset & mask;
                        stop_index = (offset + max_index) & mask;
                    } else {
                        start_index = (offset - buffer_offset) & mask;
                        stop_index = (offset + (max_index - buffer_offset)) & mask;
                    }
                } else {
                    stop_index = max_index;
                }

                if (start_index > stop_index) {
                    for (size_t i = start_index; i < buffer_size; ++i) {
                        container[i - start_index] = buffer[i];
                    }
                    for (size_t i = 0; i <= stop_index; ++i) {
                        container[buffer_size - start_index + i] = buffer[i];
                    }
                } else {
                    for (size_t i = start_index; i <= stop_index; ++i) {
                        container[i - start_index] = buffer[i];
                    }
                }
            }
        }

    public:

        typedef T value_t;

        /** \brief Конструктор циклического буфера
         */
        CircularBuffer() :
            buffer_size(0), buffer_size_div2(0), buffer_offset(0),
            count(0), count_test(0), offset(0), offset_test(0), mask(0),
            is_power_of_two(false), is_test(false) {};

        /** \brief Конструктор циклического буфера
         * \param user_size Размер циклического буфера
         */
        CircularBuffer(const size_t user_size) :
                buffer_size(user_size), buffer_size_div2(0), buffer_offset(0),
                count(0), count_test(0), offset(0), offset_test(0),
                is_power_of_two(false), is_test(false) {
            if(check_power_of_two(user_size)) {
                buffer.resize(buffer_size);
                buffer_test.resize(buffer_size);
                mask = user_size - 1;
                is_power_of_two = true;
            } else {
                const size_t new_size = cpl2(buffer_size);
                buffer.resize(new_size);
                buffer_test.resize(new_size);
                mask = new_size - 1;
                buffer_offset = buffer_size - new_size;
                is_power_of_two = false;
            }
            buffer_size_div2 = buffer_size / 2;
        };

        virtual ~CircularBuffer() = default;

        /** \brief Добавить значение в циклический буфер
         * \param value Значение
         */
        inline void push_back(const T value) {
            buffer[offset++] = value;
            if(offset > count) count = offset;
            offset &= mask;
        }

        /** \brief Получить размер циклического буфера
         * \return Размер циклического буфера. Может быть меньше максимального, если буфер еще не заполнился.
         */
        inline size_t size() const noexcept {
            return is_test ? std::min((size_t)count_test, (size_t)buffer_size) : std::min((size_t)count, (size_t)buffer_size);
        }

        /** \brief Проверить, если циклическй буфер пуст
         * \return Вернет true, если циклическй буфер пуст
         */
        inline bool empty() const noexcept {
            return is_test ? (count_test == 0) : (count == 0);
        }

        /** \brief Проверить, если циклическй буфер полн
         * \return Вернет true, если циклическй буфер полн
         */
        inline bool full() const noexcept {
            if (is_test) return (count_test >= buffer_size);
            return (count >= buffer_size);
        }

        inline bool is_ready() const noexcept {
            if (is_test) return (count_test >= buffer_size);
            return (count >= buffer_size);
        }

        void fill(const T value) {
            if(is_test) std::fill(buffer_test.begin(), buffer_test.end(), value);
            else std::fill(buffer.begin(), buffer.end(), value);
        }

        inline void set_mode(const PriceType type) noexcept {
            is_test = type == (PriceType::IntraBar);
        }

        inline bool update(const T value, const PriceType type = PriceType::Close) noexcept {
            if (type == PriceType::Close) {
                is_test = false;
                push_back(value);
                return full();
            }
            if(!is_test) {
                is_test = true;
                buffer_test = buffer;
                offset_test = offset;
                count_test = count;
                buffer_test[offset_test++] = value;
                if(offset_test > count_test) count_test = offset_test;
                offset_test &= mask;
            } else {
                buffer_test[offset] = value;
            }
            return full();
        }

        /** \brief Получить значение циклического буфера по индексу
         * \param pos Индекс элемента буфера
         * \return Значение циклического буфера
         */
        inline T &at(const size_t pos) {
            if (full()) {
                if (is_test) return buffer_test[(offset_test + (is_power_of_two ? pos : (pos - buffer_offset))) & mask];
                return buffer[(offset + (is_power_of_two ? pos : (pos - buffer_offset))) & mask];
            }
            if (is_test) return buffer_test[pos];
            return buffer[pos];
        }

        /** \brief Получить значение циклического буфера по индексу
         * \param pos Индекс элемента в буфере
         * \return Значение циклического буфера
         */
        T& operator[](std::size_t pos) {
            if (full()) {
                if(is_test) return buffer_test[(offset_test + (is_power_of_two ? pos : (pos - buffer_offset))) & mask];
                return buffer[(offset + (is_power_of_two ? pos : (pos - buffer_offset))) & mask];
            }
            if (is_test) return buffer_test[pos];
            return buffer[pos];
        }

        /** \brief Получить значение циклического буфера по индексу
         * \param pos Индекс элемента в буфере
         * \return Значение циклического буфера
         */
        const T& operator[](std::size_t pos) const {
            if (full()) {
                if (is_test) return buffer_test[(offset_test + (is_power_of_two ? pos : (pos - buffer_offset))) & mask];
                return buffer[(offset + (is_power_of_two ? pos : (pos - buffer_offset))) & mask];
            }
            if (is_test) return buffer_test[pos];
            return buffer[pos];
        }

        /** \brief Доступ к первому элементу
         * \return Возвращает ссылку на первый элемент циклического буфера
         */
        inline T &front() {
            if (full()) {
                if (is_test) return buffer_test[(offset_test - (is_power_of_two ? 0 : buffer_offset)) & mask];
                return buffer[(offset - (is_power_of_two ? 0 : buffer_offset)) & mask];
            }
            if (is_test) return buffer_test[0];
            return buffer[0];
        }

        /** \brief Доступ к первому элементу
         * \return Возвращает ссылку на первый элемент циклического буфера
         */
        inline const T &front() const noexcept {
            if (full()) {
                if (is_test) return buffer_test[(offset_test - (is_power_of_two ? 0 : buffer_offset)) & mask];
                return buffer[(offset - (is_power_of_two ? 0 : buffer_offset)) & mask];
            }
            if (is_test) return buffer_test[0];
            return buffer[0];
        }

        /** \brief Доступ к последнему элементу
         * \return Возвращает ссылку на первый элемент циклического буфера
         */
        inline T &back() noexcept {
            if (is_test) return buffer_test[(offset_test - 1) & mask];
            return buffer[(offset - 1) & mask];
        }

        /** \brief Доступ к последнему элементу
         * \return Возвращает ссылку на первый элемент циклического буфера
         */
        inline const T &back() const noexcept {
            if (is_test) return buffer_test[(offset_test - 1) & mask];
            return buffer[(offset - 1) & mask];
        }

        /** \brief Доступ к среднему элементу
         * \return Возвращает ссылку на средний элемент циклического буфера
         */
        inline T &middle() noexcept {
            if (full()) {
                if (is_test) return buffer_test[(offset_test + (is_power_of_two ? buffer_size_div2 : buffer_size_div2 - buffer_offset)) & mask];
                return buffer[(offset + (is_power_of_two ? buffer_size_div2 : buffer_size_div2 - buffer_offset)) & mask];
            }
            if (is_test) return buffer_test[(count_test/2)];
            return buffer[(count/2)];
        }

        /** \brief Доступ к среднему элементу
         * \return Возвращает ссылку на первый элемент циклического буфера
         */
        inline const T &middle() const noexcept {
            if (full()) {
                if (is_test) return buffer_test[(offset_test + (is_power_of_two ? buffer_size_div2 : buffer_size_div2 - buffer_offset)) & mask];
                return buffer[(offset + (is_power_of_two ? buffer_size_div2 : buffer_size_div2 - buffer_offset)) & mask];
            }
            if (is_test) return buffer_test[(count_test/2)];
            return buffer[(count/2)];
        }

        /** \brief Получить сумму
         * \return Возвращает сумму элементов циклического буфера
         */
        inline const T sum() const noexcept {
            T temp = 0;
            if(is_test) {
                if (full()) {
                    if(is_power_of_two) {
                        for(size_t index = 0; index < buffer_size; ++index) {
                            temp += buffer_test[(offset_test + index) & mask];
                        }
                        return temp;
                    }
                    for(size_t index = 0; index < buffer_size; ++index) {
                        temp += buffer_test[(offset_test + (index - buffer_offset)) & mask];
                    }
                    return temp;
                }
                if(is_power_of_two) {
                    for(size_t index = 0; index < size(); ++index) {
                        temp += buffer_test[index];
                    }
                    return temp;
                }
                for(size_t index = 0; index < size(); ++index) {
                    temp += buffer_test[(index - buffer_offset) & mask];
                }
                return temp;
            }
            if (full()) {
                if(is_power_of_two) {
                    for(size_t index = 0; index < buffer_size; ++index) {
                        temp += buffer[(offset + index) & mask];
                    }
                    return temp;
                }
                for(size_t index = 0; index < buffer_size; ++index) {
                    temp += buffer[(offset + (index - buffer_offset)) & mask];
                }
                return temp;
            }

            if(is_power_of_two) {
                for(size_t index = 0; index < size(); ++index) {
                    temp += buffer[index];
                }
                return temp;
            }
            for(size_t index = 0; index < size(); ++index) {
                temp += buffer[(index - buffer_offset) & mask];
            }
            return temp;
        }

        /** \brief Получить сумму
         * \param start_index   Начальный индекс
         * \param stop_index    Конечный индекс
         * \return Возвращает сумму элементов циклического буфера
         */
        inline const T sum(const size_t start_index, const size_t stop_index) const noexcept {
            T temp = 0;
            if(is_test) {
                if (full()) {
                    if(is_power_of_two) {
                        for(size_t index = start_index; index < stop_index; ++index) {
                            temp += buffer_test[(offset_test + index) & mask];
                        }
                        return temp;
                    }
                    for(size_t index = start_index; index < stop_index; ++index) {
                        temp += buffer_test[(offset_test + (index - buffer_offset)) & mask];
                    }
                    return temp;
                }
                if(is_power_of_two) {
                    for(size_t index = start_index; index < stop_index; ++index) {
                        temp += buffer_test[index];
                    }
                    return temp;
                }
                for(size_t index = start_index; index < stop_index; ++index) {
                    temp += buffer_test[(index - buffer_offset) & mask];
                }
                return temp;
            }
            if (full()) {
                if(is_power_of_two) {
                    for(size_t index = start_index; index < stop_index; ++index) {
                        temp += buffer[(offset + index) & mask];
                    }
                    return temp;
                }
                for(size_t index = start_index; index < stop_index; ++index) {
                    temp += buffer[(offset + (index - buffer_offset)) & mask];
                }
                return temp;
            }
            if(is_power_of_two) {
                for(size_t index = start_index; index < stop_index; ++index) {
                    temp += buffer[index];
                }
                return temp;
            }
            for(size_t index = start_index; index < stop_index; ++index) {
                temp += buffer[(index - buffer_offset) & mask];
            }
            return temp;
        }

        /** \brief Получить среднее значение
         * \return Возвращает среднее значение элементов циклического буфера
         */
        inline const T mean() const noexcept {
            return sum() / (T)size();
        }

        template <template <class...> class ContainerT>
        ContainerT<T> to() {
            ContainerT<T> temp;
            reserve_buffer(temp, buffer_size);
            copy_buffer(temp);
            return std::move(temp);
        }

        inline std::vector<T> to_vector() noexcept {
            std::vector<T> temp;
            temp.resize(buffer_size);
            copy_buffer(temp);
            return std::move(temp);
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            count = 0;
            count_test = 0;
            offset = 0;
            offset_test = 0;
            is_test = false;
        }
    };
};

#endif // XTECHNICAL_CIRCULAR_BUFFER_HPP_INCLUDED
