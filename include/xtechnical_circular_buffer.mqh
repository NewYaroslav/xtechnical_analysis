//+------------------------------------------------------------------+
//|                                   xtechnical_circular_buffer.mqh |
//|                        Copyright 2020, MetaQuotes Software Corp. |
//|                                             https://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2020, MetaQuotes Software Corp."
#property link      "https://www.mql5.com"
#property version   "1.00"
#property strict

class circular_buffer {
private:
     double buffer[];            /**< Основной буфер */
     double buffer_test[];       /**< Буфер для теста */
     uint buffer_size;           /**< Размер буфера */
     uint buffer_size_div2;      /**< Индекс середины массива */
     uint buffer_offset;         /**< Смещение в буфере для размера массива не кратного степени двойки */
     uint count;                 /**< Количество элементов в буфере */
     uint count_test;            /**< Количество элементов в буфере для теста */
     uint offset;                /**< Смещение в буфере */
     uint offset_test;
     uint mask;                  /**< Маска */
     bool is_power_of_two;       /**< Флаг степени двойки */
     bool is_test;               /**< Флаг теста */

     inline uint cpl2(uint x) {
         x = x - 1;
         x = x | (x >> 1);
         x = x | (x >> 2);
         x = x | (x >> 4);
         x = x | (x >> 8);
         x = x | (x >> 16);
         return x + 1;
     }

     inline bool check_power_of_two(const uint value) {
         return value && !(value & (value - 1));
     }
     
 public:

     /** \brief Конструктор циклического буфера
      */
     circular_buffer() :
         buffer_size(0), buffer_size_div2(0), buffer_offset(0),
         count(0), count_test(0), offset(0), offset_test(0), mask(0),
         is_power_of_two(false), is_test(false) {};

     /** \brief Конструктор циклического буфера
      * \param user_size Размер циклического буфера
      */
     circular_buffer(const uint user_size) :
             buffer_size(user_size), buffer_size_div2(0), buffer_offset(0),
             count(0), count_test(0), offset(0), offset_test(0),
             is_power_of_two(false), is_test(false) {
         if(check_power_of_two(user_size)) {
             ArrayResize(buffer, buffer_size);
             ArrayResize(buffer_test, buffer_size);
             mask = user_size - 1;
             is_power_of_two = true;
         } else {
             const uint new_size = cpl2(buffer_size);
             ArrayResize(buffer, new_size);
             ArrayResize(buffer_test, new_size);
             mask = new_size - 1;
             buffer_offset = buffer_size - new_size;
             is_power_of_two = false;
         }
         buffer_size_div2 = buffer_size / 2;
     };
     
     ~circular_buffer() {
         ArrayFree(buffer);
         ArrayFree(buffer_test);
     }

     /** \brief Добавить значение в циклический буфер
      * \param value Значение
      */
     inline void push_back(const double value) {
         buffer[offset++] = value;
         if(offset > count) count = offset;
         offset &= mask;
     }

     /** \brief Получить размер циклического буфера
      * \return Размер циклического буфера. Может быть меньше максимального, если буфер еще не заполнился.
      */
     inline uint size() {
         return is_test ? MathMin(count_test, buffer_size) : MathMin(count, buffer_size);
     }

     /** \brief Проверить, если циклическй буфер пуст
      * \return Вернет true, если циклическй буфер пуст
      */
     inline bool empty() {
         return is_test ? (count_test == 0) : (count == 0);
     }

     /** \brief Проверить, если циклическй буфер полн
      * \return Вернет true, если циклическй буфер полн
      */
     inline bool full() {
         if(is_test) return (count_test >= buffer_size);
         return (count >= buffer_size);
     }

     void fill(const double value) {
         if(is_test) {
            uint bz = ArraySize(buffer_test);
            for(uint i = 0; i < bz; ++i) {
               buffer_test[i] = value;
            }
         } else {
            uint bz = ArraySize(buffer);
            for(uint i = 0; i < bz; ++i) {
               buffer[i] = value;
            }
         }
     }

     /** \brief Обновить состояние циклического буфера
      * \param value Новое значение
      * \return Вернет true, если циклическй буфер полн
      */
     inline bool update(const double value) {
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
             ArrayCopy(buffer_test, buffer);
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
     inline double get(const uint index) {
         if(is_test) return buffer_test[(offset_test + (is_power_of_two ? index : (index - buffer_offset))) & mask];
         return buffer[(offset + (is_power_of_two ? index : (index - buffer_offset))) & mask];
     }

     /** \brief Доступ к первому элементу
      * \return Возвращает первый элемент циклического буфера
      */
     inline double front() {
         if(is_test) return buffer_test[(offset_test - (is_power_of_two ? 0 : buffer_offset)) & mask];
         return buffer[(offset - (is_power_of_two ? 0 : buffer_offset)) & mask];
     }

     /** \brief Доступ к последнему элементу
      * \return Возвращает последний элемент циклического буфера
      */
     inline double back() {
         if(is_test) return buffer_test[(offset_test - 1) & mask];
         return buffer[(offset - 1) & mask];
     }

     /** \brief Доступ к среднему элементу
      * \return Возвращает средний элемент циклического буфера
      */
     inline double middle() {
         if(is_test) {
             if(full()) return buffer_test[(offset_test + (is_power_of_two ? buffer_size_div2 : buffer_size_div2 - buffer_offset)) & mask];
             else return buffer_test[(offset_test + (is_power_of_two ? (count_test/2) : (count_test/2) - buffer_offset)) & mask];
         }
         if(full()) return buffer[(offset + (is_power_of_two ? buffer_size_div2 : buffer_size_div2 - buffer_offset)) & mask];
         else return buffer[(offset + (is_power_of_two ? (count/2) : (count/2) - buffer_offset)) & mask];
     }

     /** \brief Получить сумму
      * \return Возвращает сумму элементов циклического буфера
      */
     inline double sum() const {
         double temp = 0;
         if(is_test) {
             if(is_power_of_two) {
                 for(uint index = 0; index < buffer_size; ++index) {
                     temp += buffer_test[(offset_test + index) & mask];
                 }
                 return temp;
             } else {
                 for(uint index = 0; index < buffer_size; ++index) {
                     temp += buffer_test[(offset_test + (index - buffer_offset)) & mask];
                 }
                 return temp;
             }
         } else {
             if(is_power_of_two) {
                 for(uint index = 0; index < buffer_size; ++index) {
                     temp += buffer[(offset + index) & mask];
                 }
                 return temp;
             } else {
                 for(uint index = 0; index < buffer_size; ++index) {
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
     inline double sum(const uint start_index, const uint stop_index) const {
         double temp = 0;
         if(is_test) {
             if(is_power_of_two) {
                 for(uint index = start_index; index < stop_index; ++index) {
                     temp += buffer_test[(offset_test + index) & mask];
                 }
                 return temp;
             } else {
                 for(uint index = start_index; index < stop_index; ++index) {
                     temp += buffer_test[(offset_test + (index - buffer_offset)) & mask];
                 }
                 return temp;
             }
         } else {
             if(is_power_of_two) {
                 for(uint index = start_index; index < stop_index; ++index) {
                     temp += buffer[(offset + index) & mask];
                 }
                 return temp;
             } else {
                 for(uint index = start_index; index < stop_index; ++index) {
                     temp += buffer[(offset + (index - buffer_offset)) & mask];
                 }
                 return temp;
             }
         }
     }

     /** \brief Получить среднее значение
      * \return Возвращает среднее значение элементов циклического буфера
      */
     inline double mean() const {
         return sum() / (double)buffer_size;
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

