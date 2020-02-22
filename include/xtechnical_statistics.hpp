#ifndef XTECHNICAL_STATISTICS_HPP_INCLUDED
#define XTECHNICAL_STATISTICS_HPP_INCLUDED

#include <cmath>
#include <algorithm>

namespace xtechnical_statistics {

    /** \brief Посчитать среднеквадратичное значение (RMS)
     * \param array_data Массив с данными
     * \return среднеквадратичное значение (RMS)
     */
    template<class T1, class T2>
    T1 calc_root_mean_square(const T2 &array_data) {
        const size_t size = array_data.size();
        if(size == 0) return (T1)0;
        T1 sum = 0;
        for(size_t i = 0; i < size; ++i) {
            T1 temp = array_data[i] * array_data[i];
            sum += temp;
        }
        sum /= (T1)size;
        return std::sqrt(sum);
    };

    /** \brief Посчитать среднее значение
     * \param array_data Массив с данными
     * \return среднее значение
     */
    template<class T1, class T2>
    T1 calc_mean_value(const T2 &array_data) {
        const size_t size = array_data.size();
        if(size == 0) return (T1)0;
        T1 sum = 0;
        for(size_t i = 0; i < size; ++i) {
            sum += array_data[i];
        }
        sum /= (T1)size;
        return sum;
    };

    /** \brief Посчитать гармоническое среднее
     * \param array_data Массив с данными
     * \return гармоническое среднее
     */
    template<class T1, class T2>
    T1 calc_harmonic_mean(const T2 &array_data) {
        const size_t size = array_data.size();
        T1 sum = 0;
        for(size_t i = 0; i < size; ++i) {
            sum += 1.0/array_data[i];
        }
        if(sum == 0) return (T1)0;
        return (T1)size/sum;
    };

    /** \brief Посчитать среднее геометрическое
     * \param array_data Массив с данными
     * \return среднее геометрическое
     */
    template<class T1, class T2>
    T1 calc_geometric_mean(const T2 &array_data) {
        const size_t size = array_data.size();
        if(size == 0) return (T1)0;
        T1 sum = 1.0;
        for(size_t i = 0; i < size; ++i) {
            sum *= array_data[i];
        }
        return std::pow(sum, 1.0/(T1)size);
    };

    /** \brief Посчитать медиану
     * \param array_data Массив с данными
     * \return медиана
     */
    template<class T1, class T2>
    T1 calc_median(T2 array_data) {
        const size_t size = array_data.size();
        std::sort(array_data.begin(),array_data.end());
        return array_data[size/2];
    };

    /** \brief Посчитать стандартное отклонение выборки
     * \param array_data Массив с данными
     * \return стандартное отклонение выборки
     */
    template<class T1, class T2>
    T1 calc_std_dev_sample(const T2 &array_data) {
        const size_t size = array_data.size();
		if(size < 2) return (T1)0;
        T1 mean = calc_mean_value<T1>(array_data);
        T1 sum = 0;
        for(size_t i = 0; i < size; ++i) {
            T1 diff = array_data[i] - mean;
            diff*=diff;
            sum += diff;
        }
        sum /= (T1)(size - 1);
        return std::sqrt(sum);
    };

    /** \brief Посчитать стандартное отклонение популяции
     * \param array_data Массив с данными
     * \return стандартное отклонение популяции
     */
    template<class T1, class T2>
    T1 calc_std_dev_population(const T2 &array_data) {
        const size_t size = array_data.size();
		if(size == 0) return (T1)0;
        T1 mean = calc_mean_value<T1>(array_data);
        T1 sum = 0;
        for(size_t i = 0; i < size; ++i) {
            T1 diff = array_data[i] - mean;
            diff*=diff;
            sum += diff;
        }
        sum /= (T1)(size);
        return std::sqrt(sum);
    };

    /** \brief Посчитать среднее абсолютное отклонение
     * \param array_data Массив с данными
     * \return среднее абсолютное отклонение
     */
    template<class T1, class T2>
    T1 calc_mean_absolute_deviation(const T2 &array_data) {
        T1 mean = calc_mean_value<T1>(array_data);
        const size_t size = array_data.size();
        if(size == 0) return (T1)0;
        T1 sum = 0;
        for(size_t i = 0; i < size; ++i) {
            sum += array_data[i] - mean;
        }
        sum /= (T1)size;
        return sum;
    };

    /** \brief Посчитать асимметрию
     * Это мера асимметрии распределения вероятностей вещественной случайной величины.
     * Значение асимметрии может быть положительным или отрицательным, или даже неопределенным.
     * Отрицательный перекос указывает, что хвост с левой стороны функции плотности вероятности длиннее правой стороны,
     * а размер значений, вероятно, включая медиану, находится справа от среднего значения.
     * Положительный перекос указывает, что хвост с правой стороны длиннее левой стороны,
     * а размер значений лежит слева от среднего значения.
     * Нулевое значение указывает на то, что значения относительно постоянно распределены по обе стороны от среднего значения,
     * обычно, но не обязательно, с симметричным распределением.
     * \param array_data Массив с данными
     * \return асимметрия
     */
    template<class T1, class T2>
    T1 calc_skewness(const T2 &array_data) {
        T1 mean = calc_mean_value<T1>(array_data);
        const size_t size = array_data.size();
        if(size < 2) return (T1)0;
        T1 sum = 0;
        for(size_t i = 0; i < size; ++i) {
            T1 diff = array_data[i] - mean;
            diff = diff*diff*diff;
            sum += diff;
        }
        size_t new_size = size - 1;
        new_size = new_size * new_size * new_size;
        sum /= (T1)new_size;
        return sum;
    };

    /** \brief Посчитать стандартную ошибку
     * В более общем смысле стандартная ошибка (SE) вместе со средним значением выборки
     * используется для оценки приблизительных доверительных интервалов для среднего значения.
     * Он также известен как стандартная ошибка среднего или измерения, часто обозначаемая SE
     * Оценка с более низким SE указывает на то, что она имеет более точное измерение.
     * \param array_data Массив с данными
     * \return стандартная ошибка
     */
    template<class T1, class T2>
    T1 calc_standard_error(const T2 &array_data) {
        T1 std_dev = calc_std_dev_sample<T1>(array_data);
        const size_t size = array_data.size();
        if(size == 0) return (T1)0;
        return std_dev/std::sqrt((T1)size);
    };

    /** \brief Посчитать ошибку выборки
     * \param size размер выборки
     * \return ошибка выборки
     */
    template<class T1, class T2>
    T1 calc_sampling_error(const T2 &size) {
        if(size == 0) return (T1)0;
        return std::sqrt(2500.0/(T1)size) * 1.96;
    };

    /** \brief Посчитать коэффициент вариации
     *  Метод измерения отношения стандартного отклонения к среднему также известен как относительное стандартное отклонение,
     * часто сокращенно обозначаемое как RSD.
     * CV важно в области вероятности и статистики визмерять относительную изменчивость наборов данных по шкале отношений.
     * \param array_data Массив с данными
     * \return коэффициент вариации
     */
    template<class T1, class T2>
    T1 calc_coefficient_variance(const T2 &array_data) {
        size_t size = array_data.size();
        if(size < 2) return (T1)0;
        T1 mean = calc_mean_value<T1>(array_data);
        T1 sum = 0;
        for(size_t i = 0; i < size; ++i) {
            T1 diff = array_data[i] - mean;
            diff*=diff;
            sum += diff;
        }
        sum /= (T1)(size - 1);
        return std::sqrt(sum)/mean;
    }

    /** \brief Посчитать отношение сигнал / шум
     * Отношение сигнал / шум также называется SNR или S / N и определяется как отношение мощности сигнала к мощности шума,
     * которое измеряет искажение исходного сигнала.
     * Это определяющий фактор, когда дело доходит до измерения качества сигнала в каналах связи или средах.
     * Более высокий SNR гарантирует четкие приобретения с низким искажением и артефактами, вызванными нежелательным шумом.
     * Чем лучше отношение SNR, тем лучше выделяется сигнал,
     * тем лучше качество исходного сигнала или передаваемых информационных сигналов.
     * Измерение отношения S / R обычно используется в области науки и техники.
     * Отношение выше 1 указывает на большую мощность сигнала.
     * \param array_data Массив с данными
     * \return отношение сигнал / шум или SNR
     */
    template<class T1, class T2>
    T1 calc_signal_to_noise_ratio(const T2 &array_data) {
        const size_t size = array_data.size();
        if(size < 2) return (T1)0;
        T1 mean = calc_mean_value<T1>(array_data);
        T1 sum = 0;
        for(size_t i = 0; i < size; ++i) {
            T1 diff = array_data[i] - mean;
            diff*=diff;
            sum += diff;
        }
        sum /= (T1)(size - 1);
        return mean/std::sqrt(sum);
    }

    /** \brief Коэффициент эксцесса
     * Острый пик (значение выше 0) говорит о преимущественном стремлении к среднему значению
     * и отсутствия трендовой составляющей
     * Гладкий пик (значение меньше 0) говорито о тяжелых хвостах
     * распредленеия и возможном наличии тренда
     * \param array_data Массив с данными
     * \return отношение сигнал / шум или SNR
     */
    template<class T1, class T2>
    T1 calc_excess(const T2 &array_data) {
        const size_t size = array_data.size();
        if(size == 0) return (T1)0;
        T1 mean = calc_mean_value<T1>(array_data);
        T1 u4 = 0;
        T1 std_dev = 0;
        for(size_t i = 0; i < size; ++i) {
            T1 diff = array_data[i] - mean;
            diff *= diff;
            std_dev += diff;
            u4 += diff * diff;
        }
        u4 /= (T1)size;
        std_dev /= (T1)(size - 1);
        std_dev = std::sqrt(std_dev);
        std_dev *= std_dev;
        std_dev = std_dev * std_dev;
        return u4/std_dev - 3.0;
    }


    template<class T1, class T2>
    T1 calc_laplace(T2 t) {
        constexpr double pi = 3.14159265358979323846;
        constexpr double pi_x2 = pi * 2.0;
        constexpr double c = std::sqrt(pi_x2);
        return std::exp(-t * t * 0.5) / c;
    }

    template<class T1, class T2>
    T1 calc_integral_laplace(T2 t, T2 precision) {
        constexpr double pi = 3.14159265358979323846;
        constexpr double pi_x2 = pi * 2.0;
        constexpr double c = std::sqrt(pi_x2);
        double result = 0;
        for(double i = 0; i < t; i += precision){
            double temp = (i + precision);
            result += precision * std::abs(std::exp(-0.5 * i * i) + std::exp(-0.5 * temp * temp)) /2.0;
        }
        result *= 1.0 / c;
        return result;
    }

    /** \brief Посчитать вероятность, с которой винрейт играемой стратегии выше заданного числа
     *
     * \param threshold_winrate Заданнывй винрейт
     * \param win_bet Количество удачных ставок
     * \param number_bet Количество ставок
     * \return Вероятность
     */
    template<class T1, class T2>
    T1 calc_probability_winrate(const T1 threshold_winrate, const T2 win_bet, const T2 number_bet, double precision = 0.01) {
        if(number_bet <= 0) return 0;
        const double w = (double)win_bet/(double)number_bet;
        const double t = (w - threshold_winrate) * std::sqrt(((double)number_bet / w) / (1.0 - w));
        return calc_integral_laplace<T1>(t, precision) + 0.5;
    }
};

#endif // XTECHNICAL_STATISTICS_HPP_INCLUDED
