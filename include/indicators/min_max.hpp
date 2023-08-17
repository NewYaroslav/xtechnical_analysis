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
#ifndef XTECHNICAL_MIN_MAX_HPP_INCLUDED
#define XTECHNICAL_MIN_MAX_HPP_INCLUDED

#include "delay_line.hpp"

namespace xtechnical {

    enum class MinMaxLineType {
        MaxLine = 0,
        MinLine = 1
    };

    /** \brief Min Max
     */
    template <class T = double>
    class MinMax final : public BaseIndicator<T> {
    private:
        DelayLine<T> delay_line;
        std::deque<std::pair<uint64_t, T>> U, L;
        T prev_value = get_empty_value<T>();
        uint64_t period = 0;
        uint64_t index = 0;
        bool is_init = false;
        bool is_done = false;

        inline void calculate_min_max(const T &value) {
            if (value > prev_value) {
                L.push_back(std::make_pair(index - 1, prev_value));
                if (index == period + L.front().first) L.pop_front();
                while (U.size() > 0) {
                    if (value <= U.back().second) {
                        if (index == period + U.front().first) U.pop_front();
                        break;
                    } // end if
                    U.pop_back();
                } // end while
            } else {
                U.push_back(std::make_pair(index - 1, prev_value));
                if (index == period + U.front().first) U.pop_front();
                while (L.size() > 0) {
                    if (value >= L.back().second) {
                        if (index == period + L.front().first) L.pop_front();
                        break;
                    } // end if
                    L.pop_back();
                } // end while
            } // end if else
            ++index;
            prev_value = value;
        }

        inline bool calculate_min_max_v2(const T &value, T &calc_max, T &calc_min) {
            if (index == 0) return false;
            std::deque<std::pair<uint64_t, T>> tU(U), tL(L);
            if (value > prev_value) {
                tL.push_back(std::make_pair(index - 1, prev_value));
                if (index == period + tL.front().first) tL.pop_front() ;
                while (tU.size() > 0) {
                    if (value <= tU.back().second) {
                        if (index == period + tU.front().first) tU.pop_front();
                        break ;
                    } // end if
                    tU.pop_back() ;
                } // end while
            } else {
                tU.push_back(std::make_pair(index - 1, prev_value)) ;
                if (index == period + tU.front().first) tU.pop_front() ;
                while (tL.size() > 0) {
                    if (value >= tL.back().second) {
                        if (index == period + tL.front().first) tL.pop_front();
                        break ;
                    } // end if
                    tL.pop_back();
                } // end while
            } // end if else
            if ((index + 1) >= period) {
                calc_max = tU.size() > 0 ? tU.front().second : value;
                calc_min = tL.size() > 0 ? tL.front().second : value;
                return true;
            }
            return false;
        }

    public:

        MinMax() : BaseIndicator<T>(2) {};

        /** \brief Initialize the MinMax indicator
         * \param arg_period    Period
         * \param arg_backshift Backshift
         */
        MinMax(const size_t arg_period, const size_t arg_backshift = 0) :
            BaseIndicator<T>(2),
            period((uint64_t)arg_period), delay_line(arg_backshift) {
        }

        virtual ~MinMax() = default;

        /** \brief Update the state of the indicator
         * \param value New value for the indicator, such as price
         * \param type  Price type (affects the indicator's mode of operation - intra-bar or end-of-bar)
         * \return Returns true in case of success
         */
        inline bool update(const T value, const PriceType type = PriceType::Close) noexcept {
            delay_line.update(value, type);
            if (!delay_line.is_ready()) return false;
            if (type == PriceType::Close) {
                if (is_init) {
                    calculate_min_max(delay_line.get());
                    BaseIndicator<T>::output_value[0] = U.size() > 0 ? U.front().second : delay_line.get();
                    BaseIndicator<T>::output_value[1] = L.size() > 0 ? L.front().second : delay_line.get();
                    return true;
                }

                if (period <= 1) return false;

                if (index == 0) {
                    ++index;
                    prev_value = delay_line.get();
                    return false;
                }

                calculate_min_max(delay_line.get());
                if (index >= period) {
                    is_init = true;
                    BaseIndicator<T>::output_value[0] = U.size() > 0 ? U.front().second : delay_line.get();
                    BaseIndicator<T>::output_value[1] = L.size() > 0 ? L.front().second : delay_line.get();
                    is_done = true;
                    return true;
                }
                return false;
            }

            if (period <= 1) return false;

            if (calculate_min_max_v2(delay_line.get(), BaseIndicator<T>::output_value[0], BaseIndicator<T>::output_value[1])) {
                is_done = true;
                return true;
            }
            return false;
        }

        inline bool is_ready() const noexcept {
            return is_done;
        }

        inline T get_max() const noexcept {
            return BaseIndicator<T>::output_value[0];
        }

        inline T get_min() const noexcept {
            return BaseIndicator<T>::output_value[1];
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
            delay_line.reset();
            U.clear();
            L.clear();
            prev_value = get_empty_value<T>();
            index = 0;
            is_init = false;
            is_done = false;
            for (auto &item : BaseIndicator<T>::output_value) {
                item = get_empty_value<T>();
            }
        }
    };

};

#endif // XTECHNICAL_MIN_MAX_HPP_INCLUDED
