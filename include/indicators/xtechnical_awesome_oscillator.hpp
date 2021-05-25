#ifndef XTECHNICAL_AWESOME_OSCILLATOR_HPP_INCLUDED
#define XTECHNICAL_AWESOME_OSCILLATOR_HPP_INCLUDED

#include "../xtechnical_common.hpp"
#include "xtechnical_sma.hpp"

namespace xtechnical {
    using namespace xtechnical_common;

    /** \brief Awesome Oscillator
     */
    template <class T, class MA_TYPE = SMA<T>>
    class AwesomeOscillator {
    private:
        MA_TYPE fast;
		MA_TYPE slow;
		T output_value = std::numeric_limits<T>::quiet_NaN();
    public:

        AwesomeOscillator() {}

        /** \brief Конструктор Awesome Oscillator
         * \param fast_period	Период медленной МА
		 * \param slow_period	Период быстрой МА
         */
        AwesomeOscillator(const size_t fast_period, const size_t slow_period) :
            fast(fast_period), slow(slow_period) {
        }
		
		/** \brief Обновить состояние индикатора
         * \param price	Цена бара
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
		inline int update(const T price) noexcept {
			fast.update(price);
			slow.update(price);
			if (std::isnan(fast.get()) || 
				std::isnan(slow.get())) return INDICATOR_NOT_READY_TO_WORK;
			output_value = fast.get() - slow.get();
		}

        /** \brief Обновить состояние индикатора
         * \param high	Наивысшая цена бара
		 * \param low 	Наинизшая цена бара
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T high, const T low) noexcept {
			const T price = (high + low) / 2.0d;
            return update(price);
        }
		
		/** \brief Обновить состояние индикатора
         * \param high	Наивысшая цена бара
		 * \param low 	Наинизшая цена бара
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
		inline int update(const T high, const T low, T &out) noexcept {
            const int err = update(high, low);
            out = output_value;
            return err;
        }
		
        /** \brief Обновить состояние индикатора
         * \param price	Цена бара
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        inline int update(const T price, T &out) noexcept {
            const int err = update(price);
            out = output_value;
            return err;
        }
		
		/** \brief Протестировать индикатор
         * \param price	Цена бара
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
		inline int test(const T price) noexcept {
			fast.test(price);
			slow.test(price);
			if (std::isnan(fast.get()) || 
				std::isnan(slow.get())) return INDICATOR_NOT_READY_TO_WORK;
			output_value = fast.get() - slow.get();
		}
		
		/** \brief Протестировать индикатор
         * \param high	Наивысшая цена бара
		 * \param low 	Наинизшая цена бара
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T high, const T low) noexcept {
			const T price = (high + low) / 2.0d;
            return test(price);
        }
		
		/** \brief Протестировать индикатор
         * \param high	Наивысшая цена бара
		 * \param low 	Наинизшая цена бара
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
		inline int test(const T high, const T low, T &out) noexcept {
            const int err = test(high, low);
            out = output_value;
            return OK;
        }
		
        /** \brief Протестировать индикатор
         * \param price	Цена бара
         * \param out   Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        inline int test(const T price, T &out) noexcept {
            const int err = test(price);
            out = output_value;
            return err;
        }

        /** \brief Получить значение индикатора
         * \return Значение индикатора
         */
        inline T get() const noexcept {
            return output_value;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() noexcept {
            output_value = std::numeric_limits<T>::quiet_NaN();
			fast.clear();
			slow.clear();
        }
    };
}; // xtechnical

#endif // XTECHNICAL_AWESOME_OSCILLATOR_HPP_INCLUDED
