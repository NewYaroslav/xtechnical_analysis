#ifndef XTECHNICAL_FISHER_HPP_INCLUDED
#define XTECHNICAL_FISHER_HPP_INCLUDED

#include "../xtechnical_common.hpp"
#include "min_max.hpp"

namespace xtechnical {

    /** \brief Fisher Transform Indicator
     */
    template <class T>
    class FisherV1 {
    private:
        MinMax<T> max_high;
		MinMax<T> min_low;
		T output_value = std::numeric_limits<T>::quiet_NaN();
		T prev_value = 0;
		T prev_fish = 0;
    public:

        FisherV1() {}

        /** \brief Конструктор Fisher Transform Indicator
         * \param period	Период индикатора
         */
        FisherV1(const size_t period) :
            max_high(period), min_low(period) {
        }

        /** \brief Обновить состояние индикатора
         * \param high	Наивысшая цена бара
		 * \param low 	Наинизшая цена бара
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T high, const T low) noexcept {
			max_high.update(high);
			min_low.update(low);
			if (std::isnan(min_low.get_min()) ||
				std::isnan(max_high.get_max())) return common::INDICATOR_NOT_READY_TO_WORK;

			const T price = (high + low) / 2.0d;
			const T diff = max_high.get_max() - min_low.get_min();

			T value = diff == 0 ? (0.33 * 2 * (0 - 0.5) + 0.67 * prev_value) :
				(0.33 * 2 * ((price - min_low.get_min()) / (diff) - 0.5) + 0.67 * prev_value);

			value = std::min(std::max(value, -0.999), 0.999);

			if ((1 - value) == 0) output_value = 0.5 + 0.5 * prev_fish;
			else output_value = 0.5 * std::log((1 + value)/(1 - value)) + 0.5 * prev_fish;

			prev_value = value;
			prev_fish = output_value;
            return common::OK;
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
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
		inline int update(const T price) noexcept {
			return update(price, price);
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
         * \param high	Наивысшая цена бара
		 * \param low 	Наинизшая цена бара
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T high, const T low) noexcept {
			max_high.test(high);
			min_low.test(low);
			if (std::isnan(min_low.get_min()) ||
				std::isnan(max_high.get_max())) return common::INDICATOR_NOT_READY_TO_WORK;

			const T price = (high + low) / 2.0d;
			const T diff = max_high.get_max() - min_low.get_min();

			T value = diff == 0 ? (0.33 * 2 * (0 - 0.5) + 0.67 * prev_value) :
				(0.33 * 2 * ((price - min_low.get_min()) / (diff) - 0.5) + 0.67 * prev_value);

			value = std::min(std::max(value, -0.999), 0.999);

			if ((1 - value) == 0) output_value = 0.5 + 0.5 * prev_fish;
			else output_value = 0.5 * std::log((1 + value)/(1 - value)) + 0.5 * prev_fish;

            return common::OK;
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
            return common::OK;
        }

		/** \brief Протестировать индикатор
         * \param price	Цена бара
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
		inline int test(const T price) noexcept {
			return test(price, price);
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
			max_high.clear();
			min_low.clear();
			prev_value = 0;
			prev_fish = 0;
        }
    };

	/** \brief Fisher Transform Indicator
     */
    template <class T>
    class FisherV2 {
    private:
		MinMax<T> min_max;
		T output_value = std::numeric_limits<T>::quiet_NaN();
		T prev_value = 0;
		T prev_fish = 0;
    public:

        FisherV2() {}

        /** \brief Конструктор Fisher Transform Indicator
         * \param period	Период индикатора
         */
        FisherV2(const size_t period) :
            min_max(period) {
        }

        /** \brief Обновить состояние индикатора
         * \param price	Цена бара
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T price) noexcept {
			min_max.update(price);
			if (std::isnan(min_max.get_min()) ||
				std::isnan(min_max.get_max())) return common::INDICATOR_NOT_READY_TO_WORK;

			const T diff = min_max.get_max() - min_max.get_min();

			T value = diff == 0 ? (0.33 * 2 * (0 - 0.5) + 0.67 * prev_value) :
				(0.33 * 2 * ((price - min_max.get_min()) / (diff) - 0.5) + 0.67 * prev_value);

			value = std::min(std::max(value, -0.999), 0.999);

			if ((1 - value) == 0) output_value = 0.5 + 0.5 * prev_fish;
			else output_value = 0.5 * std::log((1 + value)/(1 - value)) + 0.5 * prev_fish;

			prev_value = value;
			prev_fish = output_value;
            return common::OK;
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
		int test(const T price) noexcept {
			min_max.test(price);
			if (std::isnan(min_max.get_min()) ||
				std::isnan(min_max.get_max())) return common::INDICATOR_NOT_READY_TO_WORK;

			const T diff = min_max.get_max() - min_max.get_min();

			T value = diff == 0 ? (0.33 * 2 * (0 - 0.5) + 0.67 * prev_value) :
				(0.33 * 2 * ((price - min_max.get_min()) / (diff) - 0.5) + 0.67 * prev_value);

			value = std::min(std::max(value, -0.999), 0.999);

			if ((1 - value) == 0) output_value = 0.5 + 0.5 * prev_fish;
			else output_value = 0.5 * std::log((1 + value)/(1 - value)) + 0.5 * prev_fish;
            return common::OK;
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
			min_max.clear();
			prev_value = 0;
			prev_fish = 0;
        }
    };

	/** \brief Fisher Transform Indicator
     */
    template <class T>
    class FisherV3 {
    private:
		MinMax<T> min_max;
		T output_value = std::numeric_limits<T>::quiet_NaN();
    public:

        FisherV3() {}

        /** \brief Конструктор Fisher Transform Indicator
         * \param period	Период индикатора
         */
        FisherV3(const size_t period) :
            min_max(period) {
        }

        /** \brief Обновить состояние индикатора
         * \param price	Цена бара
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T price) noexcept {
			min_max.update(price);
			if (std::isnan(min_max.get_min()) ||
				std::isnan(min_max.get_max())) return common::INDICATOR_NOT_READY_TO_WORK;

			const T diff = min_max.get_max() - min_max.get_min();
			const T value = diff == 0 ? 0 : ((2 * (price - min_max.get_min()) / diff) - 1);

			if ((1 - value) == 0) output_value = 0.5;
			else output_value = 0.5 * std::log((1 + value)/(1 - value));
            return common::OK;
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
		int test(const T price) noexcept {
			min_max.test(price);
			if (std::isnan(min_max.get_min()) ||
				std::isnan(min_max.get_max())) return common::INDICATOR_NOT_READY_TO_WORK;

			const T diff = min_max.get_max() - min_max.get_min();
			const T value = diff == 0 ? 0 : ((2 * (price - min_max.get_min()) / diff) - 1);

			if ((1 - value) == 0) output_value = 0.5;
			else output_value = 0.5 * std::log((1 + value)/(1 - value));
            return common::OK;
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
			min_max.clear();
        }
    };
}; // xtechnical

#endif // XTECHNICAL_FISHER_HPP_INCLUDED
