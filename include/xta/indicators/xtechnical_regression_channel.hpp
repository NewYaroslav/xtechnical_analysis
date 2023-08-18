#ifndef XTECHNICAL_REGRESSION_CHANNEL_HPP_INCLUDED
#define XTECHNICAL_REGRESSION_CHANNEL_HPP_INCLUDED

#include "../xtechnical_common.hpp"
#include "xtechnical_delay_line.hpp"
#include "../xtechnical_circular_buffer.hpp"

namespace xtechnical {

    /** \brief Быстрый алгоритм поиска Min и Max
     * Оригинал: 
	 * https://www.mql5.com/ru/code/viewcode/8417/45988/i-Regr.mq4
	 * https://www.mql5.com/ru/code/8417
     */
    template <class T>
    class RegressionChannel {
    private:
        std::vector<T> 						fx, sqh, sql;
		std::array<std::array<T,10>,10>		ai; 
		std::array<T,10>					b, x; 
		std::array<T,20>					sx;
		T output_tl = std::numeric_limits<T>::quiet_NaN();
        T output_ml = std::numeric_limits<T>::quiet_NaN();
		T output_bl = std::numeric_limits<T>::quiet_NaN();
		
        //T last_input = 0;
        size_t period	= 0;
		size_t degree 	= 0;
		double kstd 	= 2.0;
		size_t nn 		= 0;
		size_t max_mi 	= 0;
		size_t max_n 	= 0;
	
        DelayLine<T> 		delay_line;
		circular_buffer<T> 	buffer;
		
		size_t nn = 0;
		
    public:
        RegressionChannel() {};

		// Степень многочлена: 1- линейный; 2 - параболический; 3 - кубический
        RegressionChannel(const size_t p, const size_t d, const double c, const size_t o = 0) :
            period(p), degree(d), kstd(c), delay_line(o), buffer(p + 1) {
			nn = degree + 1;
			max_mi = nn * 2 - 2;
			max_n = period;
			std::fill(b.begin(), b.end(), 0);
			
			// sx
			sx[1] = period + 1;
			const size_t max_mi = nn * 2 - 2;
			for(size_t mi = 1; mi <= max_mi; ++mi) {
				double sum = 0;
				for(size_t n = 0; n <= period; ++n) {
					sum += std::pow(n, mi);
				}
				sx[mi + 1] = sum;
			} 
        };

        int update(const T input) noexcept {
            if(delay_line.update(input) != common::OK) {
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
			buffer.update(delay_line.get());
			if (!buffer.full()) return common::INDICATOR_NOT_READY_TO_WORK;
			const std::vector<T> data = buffer.to_vector();
			
			// syx
			double sum = 0;
			for(size_t n = 0; n <= period; ++n) {
				sum += data[n];
			}
			b[1] = sum;
			for(size_t mi = 2; mi <= nn; ++mi) {
				sum = 0;
				for(n = 0; n <= period; ++n) {
					sum += data[period - n] * std::pow(n, mi - 1);
				}
				b[mi] = sum;
			}
			// Matrix
			for(size_t jj = 1; jj <= nn; ++jj) {
				for(size_t ii = 1; ii <= nn; ++ii) {
					const size_t kk = ii + jj - 1;
					ai[ii][jj] = sx[kk];
				}
			} 
			// Gauss
			const size_t max_kk = nn - 1;
			for(size_t kk = 1; kk <= max_kk; ++kk) {
				size_t ll = 0;
				double mm = 0;
				for(size_t ii = kk; ii <= nn; ++ii) {
					const double temp = std::abs(ai[ii,kk]);
					if (temp > mm) {
						mm = temp;
						ll = ii;
					}
				}
				if (ll == 0) return common::INDICATOR_NOT_READY_TO_WORK;  
				if (ll != kk) {
					for(size_t jj = 1; jj <= nn; ++jj) {
						std::swap(ai[kk][jj], ai[ll][jj]);
					}
					std::swap(b[kk], b[ll]);
				}  
				for(size_t ii = kk + 1; ii <= nn; ++ii) {
					const double qq = ai[ii][kk] / ai[kk][kk];
					for(size_t jj = 1; jj <= nn; ++jj) {
						if (jj == kk) ai[ii][jj] = 0;
						else ai[ii][jj] = ai[ii][jj] - qq * ai[kk][jj];
					}
					b[ii] = b[ii] - qq * b[kk];
				}
			}  
			x[nn] = b[nn] / ai[nn][nn];
			for(int ii = (int)nn - 1; ii >= 1; --ii) {
				double tt = 0;
				for(int jj = 1; jj <= nn-ii; ++jj) {
					tt = tt + ai[ii][ii + jj] * x[ii + jj];
					x[ii] = (1.0 / ai[ii][ii]) * (b[ii] - tt);
				}
			}
			//
			
			for (size_t n = 0; n <= period; ++n) {
				sum = 0;
				for (size_t kk = 1; kk <= degree; ++kk) {
					sum += x[kk + 1] * std::pow(n, kk);
				}
				fx[n] = x[1] + sum;
			}
			
			// Std
			sq = 0.0;
			for (n = 0; n <= period; ++n) {
				sq += std::pow(data[period - n] - fx[n],2);
			}
			sq = std::sqrt(sq / (period + 1)) * kstd;

			/*
			for (n = 0; n <= period; ++n) {
				sqh[n] = fx[n] + sq;
				sql[n] = fx[n] - sq;
			}
			*/
			
			output_ml = fx[period] + sum;
			output_tl = output_ml + sq;
			output_bl = output_ml - sq;
			return common::OK;
        }

        /** \brief Обновить состояние индикатора
         * \param input     Сигнал на входе
         * \param min_value Минимальный сигнал на выходе за период
         * \param max_value Максимальный сигнал на выходе за период
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T input, T &min_value, T &max_value) noexcept {
            const int err = update(input);
            min_value = output_min_value;
            max_value = output_max_value;
            return err;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем, что не влияет на внутреннее состояние индикатора
         * \param input     Сигнал на входе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(T input) noexcept {
            if(delay_line.test(input) != common::OK) {
                return common::INDICATOR_NOT_READY_TO_WORK;
            }
            input = delay_line.get();
            if (index == 0) return common::INDICATOR_NOT_READY_TO_WORK;
            std::deque<std::pair<int64_t, T>> tU(U), tL(L);
            if (input > last_input) {
                tL.push_back(std::make_pair(index - 1, last_input));
                if (index == period + tL.front().first) tL.pop_front() ;
                while (tU.size() > 0) {
                    if (input <= tU.back().second) {
                        if (index == period + tU.front().first) tU.pop_front();
                        break ;
                    } // end if
                    tU.pop_back() ;
                } // end while
            } else {
                tU.push_back(std::make_pair(index - 1, last_input)) ;
                if (index == period + tU.front().first) tU.pop_front() ;
                while (tL.size() > 0) {
                    if (input >= tL.back().second) {
                        if (index == period + tL.front().first) tL.pop_front();
                        break ;
                    } // end if
                    tL.pop_back();
                } // end while
            } // end if else
            if ((index + 1) >= period) {
                output_max_value = tU.size() > 0 ? tU.front().second : input;
                output_min_value = tL.size() > 0 ? tL.front().second : input;
                return common::OK;
            }
            return common::INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем, что не влияет на внутреннее состояние индикатора
         * \param input     Сигнал на входе
         * \param min_value Минимальный сигнал на выходе за период
         * \param max_value Максимальный сигнал на выходе за период
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T input, T &min_value, T &max_value) {
            const int err = test(input);
            min_value = output_min_value;
            max_value = output_max_value;
            return err;
        }

        /** \brief Получить минимальное значение индикатора
         * \return Минимальное значение индикатора
         */
        inline T get_min() const noexcept {
            return output_min_value;
        }

        /** \brief Получить максимальное значение индикатора
         * \return Максимальное значение индикатора
         */
        inline T get_max() const noexcept {
            return output_max_value;
        }

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
            output_min_value = std::numeric_limits<T>::quiet_NaN();
            output_max_value = std::numeric_limits<T>::quiet_NaN();
            last_input = 0;
            index = 0;
            U.clear();
            L.clear();
            delay_line.clear();
        }
    };
}; // xtechnical

#endif // XTECHNICAL_REGRESSION_CHANNEL_HPP_INCLUDED
