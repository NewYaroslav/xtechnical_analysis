//+------------------------------------------------------------------+
//|                                               xtechnical_sma.mqh |
//|                        Copyright 2020, MetaQuotes Software Corp. |
//|                                             https://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2020, MetaQuotes Software Corp."
#property link      "https://www.mql5.com"
#property version   "1.00"
#property strict

#include "xtechnical_circular_buffer.mqh"

/** \brief Простая скользящая средняя
*/
class xSMA {
private:
   circular_buffer buffer;
   double last_data;
   double output_value;
   uint period;
public:

   xSMA() {
      last_data = 0;
      output_value = 0;
      period = 0;
   };

   /** \brief Инициализировать простую скользящую среднюю
    * \param user_period период
    */
   xSMA(const uint user_period) :
      buffer(user_period + 1), period(user_period) {
      last_data = 0;
      output_value = 0;
   }
   
   ~xSMA() {
   
   }

  /** \brief Обновить состояние индикатора
   * \param in сигнал на входе
   * \param out сигнал на выходе
   * \return вернет 0 в случае успеха, иначе см. ErrorType
   */
   bool update(const double in, double &out) {
      if(period == 0) {
          out = output_value = 0;
          return false;
      }
      buffer.update(in);
      if(buffer.full()) {
          last_data = last_data + (in - buffer.front());
          out = output_value = last_data/(double)period;
      } else {
          last_data += in;
          out = output_value = 0;
          return false;
      }
      return true;
   }

   /** \brief Обновить состояние индикатора
    * \param in сигнал на входе
    * \return вернет true в случае успеха
    */
   bool update(const double in) {
      if(period == 0) {
          output_value = 0;
          return false;
      }
      buffer.update(in);
      if(buffer.full()) {
          last_data = last_data + (in - buffer.front());
          output_value = last_data/(double)period;
      } else {
          last_data += in;
          //Print("in: ", in);
          //Print("last_data: ", last_data);
          output_value = 0;
          return false;
      }
      return true;
   }

   /** \brief Протестировать индикатор
    *
    * Данная функция отличается от update тем,
    * что не влияет на внутреннее состояние индикатора
    * \param in сигнал на входе
    * \param out сигнал на выходе
    * \return вернет true в случае успеха
    */
   bool test(const double in, double &out) {
      if(period == 0) {
          out = output_value = 0;
          return false;
      }
      buffer.test(in);
      if(buffer.full()) {
          out = output_value = (last_data + (in - buffer.front()))/(double)period;
      } else {
          out = output_value = 0;
          return false;
      }
      return true;
   }

   /** \brief Протестировать индикатор
    *
    * Данная функция отличается от update тем,
    * что не влияет на внутреннее состояние индикатора
    * \param in сигнал на входе
    * \return вернет 0 в случае успеха, иначе см. ErrorType
    */
   int test(const double in) {
      if(period == 0) {
          output_value = 0;
          return false;
      }
      buffer.test(in);
      if(buffer.full()) {
          output_value = (last_data + (in - buffer.front()))/(double)period;
      } else {
          output_value = 0;
          return false;
      }
      return true;
   }

   /** \brief Получить значение индикатора
    * \return Значение индикатора
    */
   inline double get() const {
      return output_value;
   }

   /** \brief Очистить данные индикатора
    */
   void clear() {
      buffer.clear();
      output_value = 0;
      last_data = 0;
   }
};
