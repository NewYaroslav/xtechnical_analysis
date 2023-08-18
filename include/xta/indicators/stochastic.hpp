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
#ifndef XTECHNICAL_STOCHASTIC_HPP_INCLUDED
#define XTECHNICAL_STOCHASTIC_HPP_INCLUDED

#include "sma.hpp"

namespace xta {

    enum StochasticLineType {
        KLine = 0,
        DLine = 1,
    };

    /** \brief Stochastic
     */
    template <class T = double, template <class...> class MA_TYPE = xta::SMA>
    class Stochastic final : public BaseIndicator<T> {
    private:
        std::deque<std::pair<uint64_t, T>> U, L;
        T prev_value = get_empty_value<T>();
        uint64_t period = 0;
        uint64_t index = 0;
        MA_TYPE<T> ma_slow;
        bool use_slow = false;
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

        Stochastic() : BaseIndicator<T>(2) {};

        /** \brief Initialize the Stochastic indicator
         * \param arg_period_fast Period %K
         * \param arg_period_slow Period %D
         */
        Stochastic(
            const size_t arg_period_fast,
            const size_t arg_period_slow = 0) : BaseIndicator<T>(2),
                period((uint64_t)arg_period_fast),
                ma_slow(arg_period_slow) {
            use_slow = (arg_period_slow > 1);
        }

        virtual ~Stochastic() = default;

        /** \brief Update the state of the indicator
         * \param value New value for the indicator, such as price
         * \param type  Price type (affects the indicator's mode of operation - intra-bar or end-of-bar)
         * \return Returns true in case of success
         */
        inline bool update(const T value, const PriceType type = PriceType::Close) noexcept {
            if (type == PriceType::Close) {
                if (is_init) {
                    calculate_min_max(value);
                    const T max_value = U.size() > 0 ? U.front().second : value;
                    const T min_value = L.size() > 0 ? L.front().second : value;
                    const T ampl = max_value - min_value;
                    BaseIndicator<T>::output_value[0] = ampl == 0 ? 50.0 : (100.0 * (value - min_value) / ampl);
                    if (use_slow) {
                        ma_slow.update(BaseIndicator<T>::output_value[0]);
                        BaseIndicator<T>::output_value[1] = ma_slow.get();
                        return ma_slow.is_ready();
                    }
                    return true;
                }

                if (period <= 1) return false;

                if (index == 0) {
                    ++index;
                    prev_value = value;
                    return false;
                }

                calculate_min_max(value);
                if (index >= period) {
                    is_init = true;
                    const T max_value = U.size() > 0 ? U.front().second : value;
                    const T min_value = L.size() > 0 ? L.front().second : value;
                    const T ampl = max_value - min_value;
                    BaseIndicator<T>::output_value[0] = ampl == 0 ? 50.0 : (100.0 * (value - min_value) / ampl);
                    if (use_slow) {
                        ma_slow.update(BaseIndicator<T>::output_value[0]);
                        BaseIndicator<T>::output_value[1] = ma_slow.get();
                        is_done = ma_slow.is_ready();
                        return is_done;
                    }
                    is_done = true;
                    return true;
                }
                return false;
            }

            if (period <= 1) return false;

            T max_value = 0, min_value = 0;
            if (calculate_min_max_v2(value, max_value, min_value)) {
                const T ampl = max_value - min_value;
                BaseIndicator<T>::output_value[0] = ampl == 0 ? 50.0 : (100.0 * (value - min_value) / ampl);
                if (use_slow) {
                    ma_slow.update(BaseIndicator<T>::output_value[0], type);
                    BaseIndicator<T>::output_value[1] = ma_slow.get();
                    is_done = ma_slow.is_ready();
                    return is_done;
                }
                is_done = true;
                return true;
            }
            return false;
        }

        inline bool is_ready() const noexcept {
            return is_done;
        }

        /** \brief Reset the state of the indicator
         */
        inline void reset() noexcept {
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

#endif // XTECHNICAL_STOCHASTIC_HPP_INCLUDED
