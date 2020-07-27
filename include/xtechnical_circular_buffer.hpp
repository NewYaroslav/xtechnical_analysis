#ifndef XTECHNICAL_CIRCULAR_BUFFER_HPP_INCLUDED
#define XTECHNICAL_CIRCULAR_BUFFER_HPP_INCLUDED

#include <vector>

namespace xtechnical {
    /** \brief Класс циклического буфера
     */
    template<class T>
    class circular_buffer {
    private:
        std::vector<T> buffer;      /**< Основной буфер */
        std::vector<T> buffer_test; /**< Буфер для теста */
        uint32_t buffer_size;       /**< Размер буфера */
        uint32_t buffer_size_div2;  /**< Индекс середины массива */
        uint32_t buffer_offset;     /**< Смещение в буфере для размера массива не кратного степени двойки */
        uint32_t count;             /**< Количество элементов в буфере */
        uint32_t count_test;        /**< Количество элементов в буфере для теста */
        uint32_t offset;            /**< Смещение в буфере */
        uint32_t offset_test;
        uint32_t mask;              /**< Маска */
        bool is_power_of_two;       /**< Флаг степени двойки */
        bool is_test;               /**< Флаг теста */

        inline const uint32_t cpl2(uint32_t x) const {
            x = x - 1;
            x = x | (x >> 1);
            x = x | (x >> 2);
            x = x | (x >> 4);
            x = x | (x >> 8);
            x = x | (x >> 16);
            return x + 1;
        }

        inline const bool check_power_of_two(const uint32_t value) const {
            return value && !(value & (value - 1));
        }
    public:
        typedef T value_t;

        /** \brief Конструктор циклического буфера
         */
        circular_buffer() :
            buffer_size(0), buffer_size_div2(0), buffer_offset(0),
            count(0), count_test(0), offset(0), offset_test(0), mask(0),
            is_power_of_two(false), is_test(false) {};

        /** \brief Конструктор циклического буфера
         * \param user_size Размер циклического буфера
         */
        circular_buffer(const size_t user_size) :
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
        inline size_t size() const {
            return is_test ? std::min((size_t)count_test, (size_t)buffer_size) : std::min((size_t)count, (size_t)buffer_size);
        }

        /** \brief Проверить, если циклическй буфер пуст
         * \return Вернет true, если циклическй буфер пуст
         */
        inline bool empty() const {
            return is_test ? (count_test == 0) : (count == 0);
        }

        /** \brief Проверить, если циклическй буфер полн
         * \return Вернет true, если циклическй буфер полн
         */
        inline bool full() const {
            if(is_test) return (count_test >= buffer_size);
            return (count >= buffer_size);
        }

        void fill(const T value) {
            if(is_test) std::fill(buffer_test.begin(), buffer_test.end(), value);
            else std::fill(buffer.begin(), buffer.end(), value);
        }

        /** \brief Обновить состояние циклического буфера
         * \param value Новое значение
         * \return Вернет true, если циклическй буфер полн
         */
        inline bool update(const T value) {
            is_test = false;
            push_back(value);
            return full();
        }

        /** \brief Протестировать состояние циклического буфера
         * \param value Новое значение
         * \return Вернет true, если циклическй буфер полн
         */
        inline bool test(const double value) {
            if(!is_test) {
                is_test = true;
                buffer_test = buffer;
                offset_test = offset;
                count_test = count;
                buffer_test[offset_test++] = value;
                if(offset_test > count_test) count_test = offset_test;
                offset_test &= mask;
            } else {
                buffer_test[(offset_test - 1) & mask] = value;
            }
            return full();
        }

        /** \brief Получить значение циклического буфера по индексу
         * \param index Индекс
         * \return Значение циклического буфера
         */
        inline T &get(const uint32_t index) {
            if(is_test) return buffer_test[(offset_test + (is_power_of_two ? index : (index - buffer_offset))) & mask];
            return buffer[(offset + (is_power_of_two ? index : (index - buffer_offset))) & mask];
        }

        /** \brief Получить значение циклического буфера по индексу
         * \param index Индекс
         * \return Значение циклического буфера
         */
        T& operator[](std::size_t index) {
            if(is_test) return buffer_test[(offset_test + (is_power_of_two ? index : (index - buffer_offset))) & mask];
            return buffer[(offset + (is_power_of_two ? index : (index - buffer_offset))) & mask];
        }

        /** \brief Получить значение циклического буфера по индексу
         * \param index Индекс
         * \return Значение циклического буфера
         */
        const T& operator[](std::size_t index) const {
            if(is_test) return buffer_test[(offset_test + (is_power_of_two ? index : (index - buffer_offset))) & mask];
            return buffer[(offset + (is_power_of_two ? index : (index - buffer_offset))) & mask];
        }

        /** \brief Доступ к первому элементу
         * \return Возвращает ссылку на первый элемент циклического буфера
         */
        inline T &front() {
            if(is_test) return buffer_test[(offset_test - (is_power_of_two ? 0 : buffer_offset)) & mask];
            return buffer[(offset - (is_power_of_two ? 0 : buffer_offset)) & mask];
        }

        /** \brief Доступ к первому элементу
         * \return Возвращает ссылку на первый элемент циклического буфера
         */
        inline const T &front() const {
            if(is_test) return buffer_test[(offset_test - (is_power_of_two ? 0 : buffer_offset)) & mask];
            return buffer[(offset - (is_power_of_two ? 0 : buffer_offset)) & mask];
        }

        /** \brief Доступ к последнему элементу
         * \return Возвращает ссылку на первый элемент циклического буфера
         */
        inline T &back() {
            if(is_test) return buffer_test[(offset_test - 1) & mask];
            return buffer[(offset - 1) & mask];
        }

        /** \brief Доступ к последнему элементу
         * \return Возвращает ссылку на первый элемент циклического буфера
         */
        inline const T &back() const {
            if(is_test) return buffer_test[(offset_test - 1) & mask];
            return buffer[(offset - 1) & mask];
        }

        /** \brief Доступ к среднему элементу
         * \return Возвращает ссылку на первый элемент циклического буфера
         */
        inline T &middle() {
            if(is_test) {
                if(full()) return buffer_test[(offset_test + (is_power_of_two ? buffer_size_div2 : buffer_size_div2 - buffer_offset)) & mask];
                else return buffer_test[(offset_test + (is_power_of_two ? (count_test/2) : (count_test/2) - buffer_offset)) & mask];
            }
            if(full()) return buffer[(offset + (is_power_of_two ? buffer_size_div2 : buffer_size_div2 - buffer_offset)) & mask];
            else return buffer[(offset + (is_power_of_two ? (count/2) : (count/2) - buffer_offset)) & mask];
        }

        /** \brief Доступ к среднему элементу
         * \return Возвращает ссылку на первый элемент циклического буфера
         */
        inline const T &middle() const {
            if(is_test) {
                if(full()) return buffer_test[(offset_test + (is_power_of_two ? buffer_size_div2 : buffer_size_div2 - buffer_offset)) & mask];
                return buffer_test[(offset_test + (is_power_of_two ? (count_test/2) : (count_test/2) - buffer_offset)) & mask];
            }
            if(full()) return buffer[(offset + (is_power_of_two ? buffer_size_div2 : buffer_size_div2 - buffer_offset)) & mask];
            return buffer[(offset + (is_power_of_two ? (count/2) : (count/2) - buffer_offset)) & mask];
        }

        /** \brief Получить сумму
         * \return Возвращает сумму элементов циклического буфера
         */
        inline const T sum() const {
            T temp = 0;
            if(is_test) {
                if(is_power_of_two) {
                    for(uint32_t index = 0; index < buffer_size; ++index) {
                        temp += buffer_test[(offset_test + index) & mask];
                    }
                    return temp;
                } else {
                    for(uint32_t index = 0; index < buffer_size; ++index) {
                        temp += buffer_test[(offset_test + (index - buffer_offset)) & mask];
                    }
                    return temp;
                }
            } else {
                if(is_power_of_two) {
                    for(uint32_t index = 0; index < buffer_size; ++index) {
                        temp += buffer[(offset + index) & mask];
                    }
                    return temp;
                } else {
                    for(uint32_t index = 0; index < buffer_size; ++index) {
                        temp += buffer[(offset + (index - buffer_offset)) & mask];
                    }
                    return temp;
                }
            }
        }

        /** \brief Получить сумму
         * \param start_index Начальный индекс
         * \param stop_index Конечный индекс
         * \return Возвращает сумму элементов циклического буфера
         */
        inline const T sum(const uint32_t start_index, const uint32_t stop_index) const {
            T temp = 0;
            if(is_test) {
                if(is_power_of_two) {
                    for(uint32_t index = start_index; index < stop_index; ++index) {
                        temp += buffer_test[(offset_test + index) & mask];
                    }
                    return temp;
                } else {
                    for(uint32_t index = start_index; index < stop_index; ++index) {
                        temp += buffer_test[(offset_test + (index - buffer_offset)) & mask];
                    }
                    return temp;
                }
            } else {
                if(is_power_of_two) {
                    for(uint32_t index = start_index; index < stop_index; ++index) {
                        temp += buffer[(offset + index) & mask];
                    }
                    return temp;
                } else {
                    for(uint32_t index = start_index; index < stop_index; ++index) {
                        temp += buffer[(offset + (index - buffer_offset)) & mask];
                    }
                    return temp;
                }
            }
        }

        /** \brief Получить среднее значение
         * \return Возвращает среднее значение элементов циклического буфера
         */
        inline const T mean() const {
            return sum() / (T)buffer_size;
        }

        /** \brief Очистить данные циклического буфера
         */
        inline void clear() {
            count = 0;
            count_test = 0;
            offset = 0;
            offset_test = 0;
            is_test = false;
            //fill(0);
        }
    };
};

#endif // XTECHNICAL_CIRCULAR_BUFFER_HPP_INCLUDED
