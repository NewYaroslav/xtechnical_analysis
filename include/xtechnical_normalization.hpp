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
#ifndef XTECHNICAL_NORMALIZATION_HPP_INCLUDED
#define XTECHNICAL_NORMALIZATION_HPP_INCLUDED

#include "xtechnical_common.hpp"

#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

namespace xtechnical_normalization {
    using namespace xtechnical_common;

    /** \brief MinMax нормализация данных
     * \param in входные данные для нормализации
     * \param out нормализованный вектор
     * \param type тип нормализации (0 - нормализация данных к промежутку от 0 до 1, иначе от -1 до 1)
     * \return вернет 0 в случае успеха
     */
    template<class T1, class T2>
    int calculate_min_max(std::vector<T1> &in, std::vector<T2> &out, const int type) {
        if(in.size() == 0)
            return INVALID_PARAMETER;
        auto it_max_data = std::max_element(in.begin(), in.end());
        auto it_min_data = std::min_element(in.begin(), in.end());
        T1 max_data = in[0];
        T1 min_data = in[0];
        if(it_max_data != in.end() && it_min_data != in.end()) {
            max_data = *it_max_data;
            min_data = *it_min_data;
        }
        T1 ampl = max_data - min_data;
        out.resize(in.size());
        if(ampl != 0) {
            for(size_t i = 0; i < in.size(); i++) {
                out[i] = type == 0 ? (double)(in[i] - min_data) / ampl : 2.0 * ((double)(in[i] - min_data) / ampl) - 1.0;
            }
        } else {
            for(size_t i = 0; i < in.size(); i++) {
                out[i] = 0.0;
            }
        }
        return OK;
    }

    /** \brief Z-Score нормализация данных
     * \param in входные данные для нормализации
     * \param out нормализованный вектор
     * \param d множитель для стандартного отклонения
     * \return вернет 0 в случае успеха
     */
    template<class T1, class T2>
    int calculate_zscore(std::vector<T1> &in, std::vector<T2> &out, const double d = 1.0) {
        if(in.size() == 0)
            return INVALID_PARAMETER;
        T1 mean = std::accumulate(in.begin(), in.end(), T1(0));
        mean /= (T1)in.size();
        T1 diff = 0;
        for(size_t k = 0; k < in.size(); ++k) {
            diff += ((in[k] - mean) * (in[k] - mean));
        }

        T1 std_dev = diff > 0 ? std::sqrt(diff / (T1) (in.size() - 1)) : 0.0;

        out.resize(in.size());
        double dix =  d * std_dev;

        for(size_t k = 0; k < in.size(); ++k) {
            out[k] = dix != 0 ? (in[k] - mean) / dix : 0.0;
            if(out[k] > 1) out[k] = 1;
            if(out[k] < -1) out[k] = -1;
        }
        return OK;
    }

    /** \brief  Посчитать массив разности элементов
     * \param in входные данные для подсчета разницы
     * \param out массив с разностью элементов
     * \return вернет 0 в случае успеха
     */
    template<class T1, class T2>
    int calculate_difference(std::vector<T1> &in, std::vector<T2> &out) {
        if(in.size() < 2) {
            out.clear();
            return INVALID_PARAMETER;
        }
        out.resize(in.size() - 1);
        for(size_t i = 1; i < in.size(); i++) {
            out[i - 1] = in[i] - in[i - 1];
        }
        return OK;
    }

    /** \brief Нормализовать амплитуду
     * \param in входные данные
     * \param out получившееся данные
     * \param max_amplitude максимальная амплитуда
     * \return вернет 0 в случае успеха
     */
    template<class T1, class T2, class T3>
    int normalize_amplitudes(std::vector<T1>& in, std::vector<T2>& out, const T3 max_amplitude) {
        if(in.size() == 0) {
            out.clear();
            return INVALID_PARAMETER;
        }
        out.resize(in.size());
        T1 max_data = *std::max_element(in.begin(), in.end());
        T1 min_data = *std::min_element(in.begin(), in.end());
        T1 max_data_ampl = std::max(abs(min_data),abs(max_data));
        if(max_data_ampl == 0) return OK;
        T3 coeff = max_amplitude/max_data_ampl;
        for(size_t i = 0; i < in.size(); i++) {
            out[i] = coeff * in[i];
        }
        return OK;
    }

    /** \brief Логарифм от данных
     * \param in входные данные
     * \param out получившееся данные
     * \return вернет 0 в случае успеха
     */
    template<class T1, class T2>
    int calculate_log(std::vector<T1>& in, std::vector<T2>& out) {
        if(in.size() == 0) {
            out.clear();
            return INVALID_PARAMETER;
        }
        out.resize(in.size());
        for(size_t i = 0; i < in.size(); i++) {
            out[i] = std::log(in[i]);
        }
        return OK;
    }

}

#endif // NORMALIZATIONEASY_HPP_INCLUDED
