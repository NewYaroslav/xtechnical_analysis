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
#ifndef XTECHNICAL_BASE_INDICATOR_HPP_INCLUDED
#define XTECHNICAL_BASE_INDICATOR_HPP_INCLUDED

#include "utils.hpp"

namespace xtechnical {

    template<class T>
    class BaseIndicator {
    protected:
        std::vector<T> output_value;

    public:

        BaseIndicator(const size_t n = 1) {
            output_value.resize(n, get_empty_value<T>());
        }

        virtual ~BaseIndicator() = default;

        /** \brief Update the state of the indicator
         * \param value New value for the indicator, such as price
         * \param type  Price type (affects the indicator's mode of operation - intra-bar or end-of-bar)
         * \return Returns true in case of success
         */
        virtual bool update(const T in_val, const PriceType type = PriceType::Close) = 0;

        virtual bool update(const T in_val, const T in_weight, const PriceType type = PriceType::Close) {
            return false;
        }

        virtual bool update(const size_t index, const T value, const uint64_t timestamp, const PriceType type = PriceType::Close) {
            return false;
        }

        bool update(const T in_val, T &out_val, const PriceType type = PriceType::Close) {
            if (update(in_val, type)) {
                out_val = output_value[0];
                return true;
            }
            return false;
        }

        virtual bool calc() {
            return false;
        }

        __attribute__((deprecated)) bool test(const T in_val) {
            return update(in_val, PriceType::IntraBar);
        }

        __attribute__((deprecated)) bool test(const T in_val, const T in_weight) {
            return update(in_val, in_weight, PriceType::IntraBar);
        }

        __attribute__((deprecated)) bool test(const T in_val, T &out_val) {
            if (update(in_val, PriceType::IntraBar)) {
                out_val = output_value[0];
                return true;
            }
            return false;
        }

        __attribute__((deprecated)) bool test(const T in_val, const T in_weight, T &out_val) {
            if (update(in_val, in_weight, PriceType::IntraBar)) {
                out_val = output_value[0];
                return true;
            }
            return false;
        }

        /** \brief Get the value of the indicator
         * \return The value of the indicator
         */
        inline T get(const size_t pos = 0) const noexcept {
            return output_value[pos];
        };

        virtual void reset() = 0;

        /** \brief Reset the state of the indicator
         */
        __attribute__((deprecated)) inline void clear() noexcept {
            reset();
        }

        virtual bool is_ready() const = 0;
    };

};

#endif // XTECHNICAL_BASE_INDICATOR_HPP_INCLUDED
