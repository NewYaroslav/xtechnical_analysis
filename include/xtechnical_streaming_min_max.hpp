#ifndef XTECHNICAL_STREAMING_MIN_MAX_HPP_INCLUDED
#define XTECHNICAL_STREAMING_MIN_MAX_HPP_INCLUDED

#include <deque>

namespace xtechnical {

    /** \brief Streaming Maximum-Minimum Filter Using No More than Three Comparsions per Element.
     * URL: https://zelych.livejournal.com/2692.html
     * URL: https://arxiv.org/abs/cs/0610046v5
     * URL: https://arxiv.org/pdf/cs/0610046v5.pdf
     * \param a         Input array
     * \param minval    Output array
     * \param maxval    Output array
     * \param w         Window length
     */
    template<class T>
    void streaming_maximum_minimum_filter(T &a, T &minval, T &maxval, const size_t w) {
        std::deque<int> U, L;
        for (size_t i = 1; i < a.size(); ++i) {
            if (i >= w) {
                maxval[i - w] = a[U.size() > 0 ? U.front() : i - 1];
                minval[i - w] = a[L.size() > 0 ? L.front() : i - 1];
            } // end if
            if (a[i] > a[i - 1]) {
                L.push_back(i - 1);
                if (i == w + L.front()) L.pop_front();
                while (U.size() > 0) {
                    if (a[i] <= a[U.back()]) {
                        if (i == w + U.front()) U.pop_front();
                        break;
                    } // end if
                    U.pop_back();
                } // end while
            } else {
                U.push_back(i - 1) ;
                if (i == w + U.front()) U.pop_front();
                while (L.size() > 0) {
                    if (a[i] >= a[L.back()]) {
                        if (i == w + L.front()) L.pop_front();
                        break;
                    } // end if
                    L.pop_back();
                } // end while
            } // end if else
        } // end for
        maxval[a.size() - w] = a[U.size() > 0 ? U.front() : a.size() - 1];
        minval[a.size() - w] = a[L.size() > 0 ? L.front() : a.size() - 1];
    }

    /** \brief Streaming Maximum-Minimum Filter Using No More than Three Comparsions per Element.
     * URL: https://zelych.livejournal.com/2692.html
     * URL: https://arxiv.org/abs/cs/0610046v5
     * URL: https://arxiv.org/pdf/cs/0610046v5.pdf
     * \param window    Input array
     * \param minval    Output value
     * \param maxval    Output value
     */
    template<class T, class T2>
    void streaming_maximum_minimum_filter(T &window, T2 &minval, T2 &maxval) {
        std::deque<int> U, L;
        const size_t w = window.size();
        for (size_t i = 1; i < w; ++i) {
            if (window[i] > window[i - 1]) {
                L.push_back(i - 1) ;
                if (i == w + L.front()) L.pop_front();
                while (U.size() > 0) {
                    if (window[i] <= window[U.back()]) {
                        if (i == w + U.front()) U.pop_front();
                        break;
                    } // end if
                    U.pop_back();
                } // end while
            } else {
                U.push_back(i - 1);
                if (i == w + U.front()) U.pop_front();
                while (L.size() > 0) {
                    if (window[i] >= window[L.back()]) {
                        if (i == w + L.front()) L.pop_front();
                        break;
                    } // end if
                    L.pop_back();
                } // end while
            } // end if else
        } // end for
        maxval = window[U.size() > 0 ? U.front() : w - 1];
        minval = window[L.size() > 0 ? L.front() : w - 1];
    }

    template<class T>
    class StreamingMaximumMinimumFilter {
    private:
        T maxval = std::numeric_limits<T>::quiet_NaN(), minval = std::numeric_limits<T>::quiet_NaN();
        T last_input = 0;
        int64_t period = 0;
        int64_t offset = 0;
        std::deque<std::pair<int64_t, T>> U, L;
    public:
        StreamingMaximumMinimumFilter(const int64_t p) : period(p) {}

        void update(const T input) noexcept {
            if (offset == 0) {
                ++offset;
                last_input = input;
                return;
            }
            if (input > last_input) {
                L.push_back(std::make_pair(offset - 1, last_input));
                if (offset == period + L.front().first) L.pop_front() ;
                while (U.size() > 0) {
                    if (input <= U.back().second) {
                        if (offset == period + U.front().first) U.pop_front();
                        break ;
                    } // end if
                    U.pop_back() ;
                } // end while
            } else {
                U.push_back(std::make_pair(offset - 1, last_input)) ;
                if (offset == period + U.front().first) U.pop_front() ;
                while (L.size() > 0) {
                    if (input >= L.back().second) {
                        if (offset == period + L.front().first) L.pop_front();
                        break ;
                    } // end if
                    L.pop_back();
                } // end while
            } // end if else
            ++offset;
            if (offset >= period) {
                maxval = U.size() > 0 ? U.front().second : input;
                minval = L.size() > 0 ? L.front().second : input;
            }
            last_input = input;
        }

        inline T get_min() noexcept {
            return minval;
        }

        inline T get_max() noexcept {
            return maxval;
        }
    };
};

#endif // XTECHNICAL_STREAMING_MIN_MAX_HPP_INCLUDED
