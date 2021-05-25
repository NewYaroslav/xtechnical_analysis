/*
* xtechnical_analysis - Technical analysis C++ library
*
* Copyright (c) 2018 Elektro Yar. Email: git.electroyar@gmail.com
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
#ifndef XTECHNICAL_MOVING_WINDOW_HPP_INCLUDED
#define XTECHNICAL_MOVING_WINDOW_HPP_INCLUDED

#include "xtechnical_normalization.hpp"
#include "xtechnical_common.hpp"

#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

#define INDICATORSEASY_DEF_RING_BUFFER_SIZE 1024

namespace xtechnical {

    /** \brief Скользящее окно
     */
    template <typename T>
    class MW {
    private:
        std::vector<T> data_;
        std::vector<T> data_test_;
        size_t period_ = 0;
        bool is_test_ = false;
    public:
        MW() {};

        /** \brief Инициализировать скользящее окно
         * \param period период
         */
        MW(const size_t period) : period_(period) {
            data_.reserve(period_);
        }

        /** \brief Проверить инициализацию буфера скользящего окна
         * \return Вернет true, если буфер скользящего окна
         * полностью заполнен значениями.
         */
        bool is_init() {
            return (data_.size() == period_);
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param out массив на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in, std::vector<T> &out) {
            is_test_ = false;
            if(period_ == 0) return common::NO_INIT;
            if(data_.size() < period_) {
                data_.push_back(in);
                if(data_.size() == period_) {
                    out = data_;
                    return common::OK;
                }
            } else {
                data_.push_back(in);
                data_.erase(data_.begin());
                out = data_;
                return common::OK;
            }
            return common::INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in) {
            is_test_ = false;
            if(period_ == 0) return common::NO_INIT;
            if(data_.size() < period_) {
                data_.push_back(in);
                if(data_.size() == period_) {
                    return common::OK;
                }
            } else {
                data_.push_back(in);
                data_.erase(data_.begin());
                return common::OK;
            }
            return common::INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем, что не влияет на внутреннее
         * состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in, std::vector<T> &out) {
            is_test_ = true;
            if(period_ == 0) return common::NO_INIT;
            data_test_ = data_;
            if(data_test_.size() < period_) {
                data_test_.push_back(in);
                if(data_test_.size() == period_) {
                    out = data_test_;
                    return common::OK;
                }
            } else {
                data_test_.push_back(in);
                data_test_.erase(data_test_.begin());
                out = data_test_;
                return common::OK;
            }
            return common::INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем, что не влияет на внутреннее
         * состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in) {
            is_test_ = true;
            if(period_ == 0) return common::NO_INIT;
            data_test_ = data_;
            if(data_test_.size() < period_) {
                data_test_.push_back(in);
                if(data_test_.size() == period_) {
                    return common::OK;
                }
            } else {
                data_test_.push_back(in);
                data_test_.erase(data_test_.begin());
                return common::OK;
            }
            return common::INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Получить данные внутреннего буфера индикатора
         * \param buffer буфер
         */
        void get_data(std::vector<T> &buffer) {
            if(is_test_) buffer = data_test_;
            else buffer = data_;
        }

        /** \brief Получить максимальное значение буфера
         * \param max_value Максимальное значение
         * \param period Период максимальных данных
         * \param offset Смещение в массиве. По умолчанию 0
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int get_max_value(
                T &max_value,
                const size_t period,
                const size_t offset = 0) {
            const size_t total_offset = period + offset;
            if(is_test_ && data_test_.size() < total_offset)
                return common::INVALID_PARAMETER;
            else if(!is_test_ && data_.size() < total_offset)
                return common::INVALID_PARAMETER;

            if(is_test_) max_value = *std::max_element(
                data_test_.end() - total_offset,
                data_test_.end() - offset);
            else max_value = *std::max_element(
                data_.end() - total_offset,
                data_.end() - offset);
            return common::OK;
        }

        /** \brief Получить минимальное значение буфера
         * \param min_value Минимальное значение
         * \param period Период минимальных данных
         * \param offset Смещение в массиве. По умолчанию 0
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int get_min_value(
                T &min_value,
                const size_t period,
                const size_t offset = 0) {
            const size_t total_offset = period + offset;
            if(is_test_ && data_test_.size() < total_offset)
                return common::INVALID_PARAMETER;
            else if(!is_test_ && data_.size() < total_offset)
                return common::INVALID_PARAMETER;

            if(is_test_) min_value = *std::min_element(
                data_test_.end() - total_offset,
                data_test_.end() - offset);
            else min_value = *std::min_element(
                data_.end() - total_offset,
                data_.end() - offset);
            return common::OK;
        }

        /** \brief Получить сумму
         * \param sum_value Сумма
         * \param period Период минимальных данных
         * \param offset Смещение в массиве. По умолчанию 0
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int get_sum(T &sum_value,
                const size_t period,
                const size_t offset = 0) {
            const size_t total_offset = period + offset;
            if(is_test_ && data_test_.size() < total_offset)
                return common::INVALID_PARAMETER;
            else if(!is_test_ && data_.size() < total_offset)
                return common::INVALID_PARAMETER;

            if(is_test_) sum_value = std::accumulate(
                data_test_.end() - total_offset,
                data_test_.end() - offset,
                (T)0);
            else sum_value = std::accumulate(
                data_.end() - total_offset,
                data_.end() - offset,
                (T)0);
            return common::OK;
        }

        /** \brief Получить нормализованные данные
         * \param buffer буфер
         * \param type Тип нормализации.
         * На выбор: MINMAX_UNSIGNED, MINMAX_SIGNED, Z_SCORE_TRANSFORMING
         * \param period Период минимальных данных
         * \param offset Смещение в массиве. По умолчанию 0
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int get_normalized_data(
                std::vector<T> &buffer,
                const uint32_t type,
                const size_t period,
                const size_t offset = 0) {
            const size_t total_offset = period + offset;
            if(is_test_ && data_test_.size() < total_offset)
                return common::INVALID_PARAMETER;
            else if(!is_test_ && data_.size() < total_offset)
                return common::INVALID_PARAMETER;

            std::vector<T> fragment;
            if(is_test_) fragment.assign(
                data_test_.end() - total_offset,
                data_test_.end() - offset);
            else fragment.assign(
                data_.end() - total_offset,
                data_.end() - offset);
            if( type == common::MINMAX_UNSIGNED ||
                type == common::MINMAX_UNSIGNED) {
                buffer.resize(fragment.size());
                xtechnical::normalization::calculate_min_max(
                    fragment,
                    buffer,
                    type);
            } else
            if(type == common::Z_SCORE_TRANSFORMING) {
                buffer.resize(fragment.size());
                xtechnical::normalization::calculate_zscore(fragment, buffer);
            }

            return common::OK;
        }

        /** \brief Получить нормализованные данные
         * \param buffer Буфер
         * \param type Тип нормализации.
         * На выбор: MINMAX_UNSIGNED, MINMAX_SIGNED, Z_SCORE_TRANSFORMING
         * \param min_level Заданный минимальный уровень
         * \param max_level Заданный максимальный уровень
         * \param period Период минимальных данных
         * \param offset Смещение в массиве. По умолчанию 0
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int get_normalized_data(
                std::vector<T> &buffer,
                const uint32_t type,
                const T min_level,
                const T max_level,
                const size_t period,
                const size_t offset = 0) {
            const size_t total_offset = period + offset;
            if(is_test_ && data_test_.size() < total_offset)
                return common::INVALID_PARAMETER;
            else if(!is_test_ && data_.size() < total_offset)
                return common::INVALID_PARAMETER;

            std::vector<T> fragment;
            if(is_test_) fragment.assign(
                data_test_.end() - total_offset,
                data_test_.end() - offset);
            else fragment.assign(
                data_.end() - total_offset,
                data_.end() - offset);
            if(type == common::MINMAX_UNSIGNED ||
                type == common::MINMAX_SIGNED) {
                buffer.resize(fragment.size());
                xtechnical::normalization::calculate_min_max(
                    fragment,
                    buffer,
                    min_level,
                    max_level,
                    type);
            } else
            if(type == common::Z_SCORE_TRANSFORMING) {
                buffer.resize(fragment.size());
                xtechnical::normalization::calculate_zscore(fragment, buffer);
            }

            return common::OK;
        }

        /** \brief Экспериментальный метод
         */
        int compare_data(
                T &compare_result,
                const uint32_t compare_type,
                const bool is_use_min_max_level,
                const T min_level,
                const T max_level,
                const size_t period,
                const size_t offset = 0) {
            std::vector<T> fragment;
            int err = common::OK;
            if(!is_use_min_max_level) err = get_normalized_data(
                fragment,
                common::MINMAX_SIGNED,
                period,
                offset);
            else err = get_normalized_data(
                fragment,
                common::MINMAX_SIGNED,
                min_level,
                max_level,
                period,
                offset);
            if(err != common::OK) return err;
            if(compare_type == common::COMPARE_WITH_ZERO_LINE) {
                compare_result = 0;
                for(size_t i = 0; i < period; ++i) {
                    compare_result += std::abs(fragment[i]);
                }
                compare_result /= (T)period;
                compare_result = 1.0 - compare_result;
            } else
            if(compare_type == common::COMPARE_WITH_STRAIGHT_LINE) {
                /* сначала найдем минимум или максимум */
                T new_max_level = -1.1;
                T new_min_level = 1.1;
                for(size_t i = 0; i < period; ++i) {
                    if(fragment[i] < new_min_level) new_min_level = fragment[i];
                    if(fragment[i] > new_max_level) new_max_level = fragment[i];
                }

                /* находим два значения */
                T result_min = 0;
                T result_max = 0;
                for(size_t i = 0; i < period; ++i) {
                    result_min += std::abs(fragment[i] - new_min_level);
                    result_max += std::abs(new_max_level - fragment[i]);
                }
                result_min /= (T)period;
                result_max /= (T)period;
                compare_result = std::max(result_min, result_max);
                compare_result = 1.0 - compare_result;
            } else
            if(compare_type == common::COMPARE_WITH_CENTER_LINE) {
                /* сначала найдем минимум или максимум */
                T new_max_level = -1.1;
                T new_min_level = 1.1;
                for(size_t i = 0; i < period; ++i) {
                    if(fragment[i] < new_min_level) new_min_level = fragment[i];
                    if(fragment[i] > new_max_level) new_max_level = fragment[i];
                }

                /* находим два значения */
                compare_result = 0;
                T center_line = (new_max_level - new_min_level) / 2.0;
                for(size_t i = 0; i < period; ++i) {
                    compare_result += std::abs(fragment[i] - center_line);
                }
                compare_result /= (T)period;
                compare_result = 1.0 - compare_result;
            } else
            if(compare_type == common::CALCULATE_ANGLE) {
                /* найдем приращение */
                T average_increase = 0;
                for(size_t i = 1; i < period; ++i) {
                    average_increase += (fragment[i] - fragment[i - 1]);
                }
                average_increase /= (T)(period - 1);
                /* найдем угол */
                compare_result = std::atan(-average_increase / -1.0);
            }
            return common::OK;
        }

        /** \brief Получить среднее значение буфера
         * \param average_value Среднее значение
         * \param period Период среднего значения
         * \param offset Смещение в массиве. По умолчанию 0
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int get_average(
                T &average_value,
                const size_t period,
                const size_t offset = 0) {
            const size_t total_offset = period + offset;
            if(is_test_ && data_test_.size() < total_offset)
                return common::INVALID_PARAMETER;
            else if(!is_test_ && data_.size() < total_offset)
                return common::INVALID_PARAMETER;

            if(is_test_) {
                T sum = std::accumulate(
                    data_test_.end() - period - offset,
                    data_test_.end() - offset,
                    T(0));
                average_value = sum / (T)period;
            } else {
                T sum = std::accumulate(
                    data_.end() - period - offset,
                    data_.end() - offset,
                    T(0));
                average_value = sum / (T)period;
            }
            return common::OK;
        }

        /** \brief Получить стандартное отклонение буфера
         * \param std_dev_value Стандартное отклонение
         * \param period Период стандартного отклонения
         * \param offset Смещение в массиве. По умолчанию 0
         */
        int get_std_dev(
                T &std_dev_value,
                const size_t period,
                const size_t offset = 0) {
            const size_t total_offset = period + offset;
            if(is_test_ && data_test_.size() < total_offset)
                return common::INVALID_PARAMETER;
            else if(!is_test_ && data_.size() < total_offset)
                return common::INVALID_PARAMETER;

            if(is_test_) {
                T ml = std::accumulate(
                    data_test_.end() - total_offset,
                    data_test_.end() - offset,
                    T(0));
                ml /= (T)period;
                T sum = 0;
                const size_t stop = data_test_.size() - offset;
                const size_t start = stop - period;
                for (size_t i = start; i < stop; ++i) {
                    T diff = (data_test_[i] - ml);
                    sum +=  diff * diff;
                }
                std_dev_value = std::sqrt(sum / (T)(period - 1));
            } else {
                T ml = std::accumulate(
                    data_.end() - total_offset,
                    data_.end() - offset,
                    T(0));
                ml /= (T)period;
                T sum = 0;
                const size_t stop = data_.size() - offset;
                const size_t start = stop - period;
                for (size_t i = start; i < stop; ++i) {
                    T diff = (data_[i] - ml);
                    sum +=  diff * diff;
                }
                std_dev_value = std::sqrt(sum / (T)(period - 1));
            }
            return common::OK;
        }

        /** \brief Получить массив средних значений
         * и стандартного отклонения буфера
         *
         * Минимальный период равен 2
         * \param average_data массив средних значений
         * \param std_data массив стандартного отклонения
         * \param min_period минимальный период
         * \param max_period максимальный период
         * \param step_period шаг периода
         */
        void get_average_and_std_dev_array(
                std::vector<T> &average_data,
                std::vector<T> &std_data,
                size_t min_period,
                size_t max_period,
                const size_t &step_period) {
            size_t reserve_size = 1 + (max_period - min_period)/step_period;
            --min_period;
            --max_period;
            average_data.clear();
            average_data.reserve(reserve_size);
            std_data.clear();
            std_data.reserve(reserve_size);
            if(is_test_) {
                T sum = 0;
                size_t num_element = 0;
                size_t data_size = data_.size();
                // начинаем список с конца
                for(int i = data_size - 1; i >= 0; --i) {
                    sum += data_test_[i]; // находим сумму элементов
                    if(num_element > max_period) break;
                    if(num_element >= min_period) {
                        ++num_element; // находим число элементов
                        T ml = (T)(sum/(T)num_element); // находим среднее
                        average_data.push_back(ml); // добавляем среднее
                        T sum_std = 0;
                        size_t max_len = data_size - num_element;
                        for(size_t j = data_size - 1; j >= max_len; --j) {
                            T diff = (data_test_[j] - ml);
                            sum_std += diff * diff;
                        }
                        std_data.push_back((T)std::sqrt(sum_std /
                            (T)(num_element - 1)));
                        min_period += step_period;
                    } else {
                        ++num_element;
                    }
                } // for i
            } else {
                T sum = 0;
                size_t num_element = 0;
                size_t data_size = data_.size();
                // начинаем список с конца
                for(int i = data_size - 1; i >= 0; --i) {
                    sum += data_[i]; // находим сумму элементов
                    if(num_element > max_period) break;
                    if(num_element >= min_period) {
                        ++num_element; // находим число элементов
                        T ml = (T)(sum/(T)num_element); // находим среднее
                        average_data.push_back(ml); // добавляем среднее
                        T sum_std = 0;
                        int max_len = data_size - num_element;
                        for(int j = data_size - 1; j >= max_len; j--) {
                            T diff = (data_[j] - ml);
                            sum_std += diff * diff;
                        }
                        std_data.push_back((T)std::sqrt(sum_std /
                            (T)(num_element - 1)));
                        min_period += step_period;
                    } else {
                        ++num_element;
                    }
                } // for i
            }
        }

        /** \brief Получить массив значений RSI
         * \param rsi_data массив значений RSI
         * \param min_period минимальный период
         * \param max_period максимальный период
         * \param step_period шаг периода
         */
        void get_rsi_array(
                std::vector<T> &rsi_data,
                size_t min_period,
                size_t max_period,
                const size_t &step_period) {
            size_t reserve_size = 1 + (max_period - min_period)/step_period;
            --min_period;
            --max_period;
            rsi_data.clear();
            rsi_data.reserve(reserve_size);
            if(is_test_) {
                T sum_u = 0, sum_d = 0;
                size_t num_element = 0;
                // начинаем список с конца
                for(size_t i = data_test_.size() - 1; i >= 1; --i) {
                    T u = 0, d = 0;
                    const T prev_ = data_test_[i - 1];
                    const T in_ = data_test_[i];
                    if(prev_ < in_) u = in_ - prev_;
                    else if(prev_ > in_) d = prev_ - in_;
                    sum_u += u;
                    sum_d += d;
                    if(num_element > max_period) break;
                    if(num_element >= min_period) {
                        ++num_element;
                        u = sum_u /(T)num_element;
                        d = sum_d /(T)num_element;
                        if(d == 0) rsi_data.push_back(100.0);
                        else rsi_data.push_back(((T)100.0 - ((T)100.0 /
                            ((T)1.0 + (u / d)))));
                        min_period += step_period;
                    } else {
                        ++num_element;
                    }
                } // for i
            } else {
                T sum_u = 0;
                T sum_d = 0;
                size_t num_element = 0;
                // начинаем список с конца
                for(size_t i = data_.size() - 1; i >= 1; --i) {
                    if(data_[i - 1] < data_[i])
                        sum_u += data_[i] - data_[i - 1];
                    else if(data_[i - 1] > data_[i])
                        sum_d += data_[i - 1] - data_[i];
                    if(num_element > max_period) break;
                    if(num_element >= min_period) {
                        ++num_element;
                        T u = sum_u /(T)num_element;
                        T d = sum_d /(T)num_element;
                        if(d == 0) rsi_data.push_back(100.0);
                        else rsi_data.push_back(((T)100.0 - ((T)100.0 /
                            ((T)1.0 + (u / d)))));
                        min_period += step_period;
                    } else {
                        ++num_element;
                    }
                } // for i
            }
        }

        /** \brief Получить значение RSI
         * \param rsi_value Значение RSI
         * \param period Период RSI
         */
        void get_rsi(T &rsi_value, const size_t period) {
            rsi_value = 50;
            if(is_test_) {
                T sum_u = 0, sum_d = 0;
                const size_t start_ind = data_test_.size() - 1;
                const size_t stop_ind = data_test_.size() - period;
                // начинаем список с конца
                for(size_t i = start_ind; i >= stop_ind; --i) {
                    T u = 0, d = 0;
                    const T prev_ = data_test_[i - 1];
                    const T in_ = data_test_[i];
                    if(prev_ < in_) u = in_ - prev_;
                    else if(prev_ > in_) d = prev_ - in_;
                    sum_u += u;
                    sum_d += d;
                } // for i
                T u = sum_u /(T)period;
                T d = sum_d /(T)period;
                if(d == 0) rsi_value = 100.0;
                else rsi_value = ((T)100.0 - ((T)100.0 / ((T)1.0 + (u / d))));
            } else {
                T sum_u = 0;
                T sum_d = 0;
                const size_t start_ind = data_.size() - 1;
                const size_t stop_ind = data_.size() - period;
                // начинаем список с конца
                for(size_t i = start_ind; i >= stop_ind; --i) {
                    if(data_[i - 1] < data_[i])
                        sum_u += data_[i] - data_[i - 1];
                    else if(data_[i - 1] > data_[i])
                        sum_d += data_[i - 1] - data_[i];
                } // for i
                T u = sum_u /(T)period;
                T d = sum_d /(T)period;
                if(d == 0) rsi_value = 100.0;
                else rsi_value = ((T)100.0 - ((T)100.0 / ((T)1.0 + (u / d))));
            }
        }

        /** \brief Получить zscore
         * \param std_dev_value Стандартное отклонение
         * \param period Период стандартного отклонения
         * \param offset Смещение в массиве. По умолчанию 0
         */
        int get_zscore_value(
                T &zscore_value,
                const size_t period,
                const size_t offset = 0) {
            const size_t total_offset = period + offset;
            if(is_test_ && data_test_.size() < total_offset)
                return common::INVALID_PARAMETER;
            else if(!is_test_ && data_.size() < total_offset)
                return common::INVALID_PARAMETER;
            T std_dev_value = 0;
            if(is_test_) {
                T ml = std::accumulate(
                    data_test_.end() - total_offset,
                    data_test_.end() - offset,
                    T(0));
                ml /= (T)period;
                T sum = 0;
                const size_t stop = data_test_.size() - offset;
                const size_t start = stop - period;
                for (size_t i = start; i < stop; ++i) {
                    T diff = (data_test_[i] - ml);
                    sum +=  diff * diff;
                }
                std_dev_value = std::sqrt(sum / (T)(period - 1));
                if(std_dev_value != 0) zscore_value = (data_test_.back() - ml) / std_dev_value;
            } else {
                T ml = std::accumulate(
                    data_.end() - total_offset,
                    data_.end() - offset,
                    T(0));
                ml /= (T)period;
                T sum = 0;
                const size_t stop = data_.size() - offset;
                const size_t start = stop - period;
                for (size_t i = start; i < stop; ++i) {
                    T diff = (data_[i] - ml);
                    sum +=  diff * diff;
                }
                std_dev_value = std::sqrt(sum / (T)(period - 1));
                if(std_dev_value != 0) zscore_value = (data_.back() - ml) / std_dev_value;
            }
            return common::OK;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            data_.clear();
            data_test_.clear();
        }
    };
}

#endif // XTECHNICAL_MOVING_WINDOW_HPP_INCLUDED
