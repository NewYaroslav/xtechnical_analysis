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
#ifndef XTECHNICAL_ENUMS_HPP_INCLUDED
#define XTECHNICAL_ENUMS_HPP_INCLUDED

namespace xtechnical {

    const double EMPTY_VALUE_F64 = std::numeric_limits<double>::quiet_NaN();
    const double EMPTY_VALUE_F32 = std::numeric_limits<float>::quiet_NaN();

    enum class PriceType {
        IntraBar,
        Close,
    };

    /// Набор возможных состояний ошибки
    enum ErrorState {
        OK = 0,                             ///< Ошибок нет, все в порядке
        NO_INIT = -1,                       ///< Не было инициализации, поэтому метод класса не может быть использован
        INVALID_PARAMETER = -2,             ///< Один из параметров неверно указан
        INDICATOR_NOT_READY_TO_WORK = -3,   ///< Индикатор не готов к работе
    };

    /// Типы нормализации данных
    enum NormType {
        MINMAX_UNSIGNED = 0,        ///< Нормализация данных. Данные приводятся к уровню от 0 до 1
        MINMAX_SIGNED = 1,          ///< Нормализация данных. Данные приводятся к уровню от -1 до 1
        Z_SCORE_TRANSFORMING = 2,   ///< Стандартизация данных. Преобразование данных с использованием z-показателя
    };

    enum CompMethod {
        COMPARE_WITH_ZERO_LINE = 0,
        COMPARE_WITH_STRAIGHT_LINE = 1,
        COMPARE_WITH_CENTER_LINE = 2,
        CALCULATE_ANGLE = 3,
    };

    enum DirType {
        BUY = 1,
        SELL = -1,
    };
}; // xtechnical

#endif // XTECHNICAL_COMMON_HPP_INCLUDED
