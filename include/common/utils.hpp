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
#ifndef XTECHNICAL_UTILS_HPP_INCLUDED
#define XTECHNICAL_UTILS_HPP_INCLUDED

#include <limits>

namespace xtechnical {

    template<typename EnumT>
    constexpr size_t enum_to_index(EnumT value) {
        return static_cast<size_t>(value);
    }

    template <typename T>
    bool is_value_empty(const T& value);

    template <>
    bool is_value_empty<float>(const float& value) {
        return std::isnan(value);
    }

    template <>
    bool is_value_empty<double>(const double& value) {
        return std::isnan(value);
    }

    template <>
    bool is_value_empty<int32_t>(const int32_t& value) {
        static const int32_t EMPTY_VALUE = std::numeric_limits<int32_t>::min();
        return value == EMPTY_VALUE;
    }

    template <>
    bool is_value_empty<int64_t>(const int64_t& value) {
        static const int64_t EMPTY_VALUE = std::numeric_limits<int64_t>::min();
        return value == EMPTY_VALUE;
    }

    template <>
    bool is_value_empty<std::string>(const std::string& value) {
        return value.empty();
    }

    template <typename T>
    T get_empty_value() {
        return std::numeric_limits<T>::quiet_NaN();
    }

    template <>
    int32_t get_empty_value<int32_t>() {
        return std::numeric_limits<int32_t>::min();
    }

    template <>
    int64_t get_empty_value<int64_t>() {
        return std::numeric_limits<int64_t>::min();
    }

    template <>
    std::string get_empty_value<std::string>() {
        return std::string();
    }

    //------

    template <typename T>
    T get_infinity_value() {
        return std::numeric_limits<T>::infinity();
    }

    template <>
    int32_t get_infinity_value<int32_t>() {
        return std::numeric_limits<int32_t>::max();
    }

    template <>
    int64_t get_infinity_value<int64_t>() {
        return std::numeric_limits<int64_t>::max();
    }

    template <typename T>
    bool is_value_infinity(const T& value);

    template <>
    bool is_value_infinity<float>(const float& value) {
        return std::isinf(value);
    }

    template <>
    bool is_value_infinity<double>(const double& value) {
        return std::isinf(value);
    }

    template <>
    bool is_value_infinity<int32_t>(const int32_t& value) {
        static const int32_t EMPTY_VALUE = std::numeric_limits<int32_t>::max();
        return value == EMPTY_VALUE;
    }

    template <>
    bool is_value_infinity<int64_t>(const int64_t& value) {
        static const int64_t EMPTY_VALUE = std::numeric_limits<int64_t>::max();
        return value == EMPTY_VALUE;
    }

};

#endif // XTECHNICAL_UTILS_HPP_INCLUDED
