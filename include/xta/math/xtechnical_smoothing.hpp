#ifndef XTECHNICAL_SMOOTHING_HPP_INCLUDED
#define XTECHNICAL_SMOOTHING_HPP_INCLUDED

#include "xtechnical_ordinary_least_squares.hpp"
#include "../indicators/sma.hpp"
#include "../indicators/rsi.hpp"

namespace xtechnical {

    enum class SmoothingType {
        LINE = 0,
        PARABOLA = 1,
    };

    template <typename T>
    void smoothing(const SmoothingType type, const T &input, T &output) {
        switch(type) {
        case SmoothingType::LINE: {
                auto temp(input);
                std::iota(temp.begin(), temp.end(), 0);
                std::vector<double> coeff(2);
                calc_ols(temp, input, OlsFunctionType::LINE, coeff);
                for (size_t i = 0; i < input.size(); ++i) output[i] = calc_ols_line(coeff, i);
            }
            break;
        case SmoothingType::PARABOLA: {
                auto temp(input);
                std::iota(temp.begin(), temp.end(), 0);
                std::vector<double> coeff(3);
                calc_ols(temp, input, OlsFunctionType::PARABOLA, coeff);
                for (size_t i = 0; i < input.size(); ++i) output[i] = calc_ols_line(coeff, i);
            }
            break;
        default:
            break;
        };
    }

    template <class T1, class T2>
    void smoothing_cycle_sma(const size_t period, const size_t offset, const T1 &input, T2 &output) {
        using NumType = typename T2::value_type;
        SMA<NumType> ma(period);
        size_t index = 0;
        while (true) {
            if (!std::isnan(ma.get()) && index == (input.size() - offset)) break;
            ma.update(input[index++]);
            if (index >= input.size()) index = 0;
        }
        for (size_t i = 0; i < input.size(); ++i) {
            output[i] = ma.get();
            ma.update(input[index++]);
            if (index >= input.size()) index = 0;
        }
    }

    /** \brief Посчитать простую скользящую среднюю (SMA)
     *
     * Данная функция для расчетов использует последние N = period значений
     * \param input     Массив значений
     * \param output    Значение SMA
     * \param period    Период SMA
     * \param start_pos Начальная позиция в массиве
     * \return Вернет true в случае успеха
     */
    template <typename T1, typename T2>
    bool calculate_sma(
            T1 &input,
            T2 &output,
            const size_t period,
            const size_t start_pos = 0) noexcept {
        if(input.size() <= start_pos + period) return false;
        using NumType = typename T1::value_type;
        auto sum = std::accumulate(
            input.begin() + start_pos,
            input.begin() + start_pos + period,
            NumType(0));
        output = sum / (T2)period;
        return true;
    }

    /** \brief Заполнить буфер средним значением данных
     * \param input     Массив значений
     * \param output    Массив средних значений
     * \param period    Период SMA
     * \param start_pos Начальная позиция в массиве
     * \return Вернет true в случае успеха
     */
    template <typename T1, typename T2>
    bool fill_sma(
            T1 &input,
            T2 &output,
            const size_t period,
            const size_t start_pos = 0) noexcept {
        using NumType = typename T2::value_type;
        NumType mean = 0;
        if (!calculate_sma(input, mean, period, start_pos)) return false;
        std::fill(output.begin(),output.end(), mean);
        return true;
    }

    /** \brief Расчитать стандартное отклонение
     * \param input входные данные индикатора
     * \param output стандартное отклонение
     * \param period период STD
     * \param start_pos начальная позиция в массиве
     * \return вернет 0 в случае успеха
     */
    template<typename T1, typename T2>
    bool calculate_std_dev(
            T1 &input,
            T2 &output,
            const size_t period,
            const size_t start_pos = 0) noexcept {
        if(input.size() < start_pos + period) return false;
        using NumType = typename T1::value_type;
        auto mean = std::accumulate(
            input.begin() + start_pos,
            input.begin() + start_pos + period,
            NumType(0));
        mean /= (NumType)period;
        double _std_dev = 0;
        for(int i = 0; i < (int)input.size(); i++) {
            double diff = (input[i] - mean);
            _std_dev +=  diff * diff;
        }
        output = std::sqrt(_std_dev / (T2)(period - 1));
        return true;
    }

    /** \brief Расчитать стандартное отклонение и среднее значение
     * \param input входные данные индикатора
     * \param output стандартное отклонение
     * \param period период STD
     * \param start_pos начальная позиция в массиве
     * \return вернет 0 в случае успеха
     */
    template<typename T1, typename T2>
    bool calculate_std_dev_and_mean(
            T1 &input,
            T2 &std_dev,
            T2 &mean,
            const size_t period,
            const size_t start_pos = 0) {
        if(input.size() < start_pos + period) return false;
        using NumType = typename T1::value_type;
        mean = (T2)std::accumulate(
            input.begin() + start_pos,
            input.begin() + start_pos + period,
            NumType(0));
        mean /= (NumType)period;
        double _std_dev = 0;
        for(int i = 0; i < (int)input.size(); i++) {
            double diff = (input[i] - mean);
            _std_dev +=  diff * diff;
        }
        std_dev = std::sqrt(_std_dev / (T2)(period - 1));
        return true;
    }


    template <class T1, class T2>
    bool calc_ring_rsi(const T1 &in, T2 &out, const size_t &period) {
        size_t input_size = in.size();
        size_t output_size = out.size();
        if( input_size == 0 || input_size < period ||
            output_size != input_size)
            return false;
        using NumType = typename T1::value_type;
        RSI<NumType,SMA> iRSI(period);
        for(size_t i = input_size - period; i < input_size; ++i) {
            iRSI.update(in[i]);
        }
        for(size_t i = 0; i < input_size; ++i) {
            iRSI.update(in[i]);
            out[i] = iRSI.get();
        }
        return true;
    }
}

#endif // XTECHNICAL_SMOOTHING_HPP_INCLUDED
