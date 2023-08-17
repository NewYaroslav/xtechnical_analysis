#ifndef XTECHNICAL_SSA_HPP_INCLUDED
#define XTECHNICAL_SSA_HPP_INCLUDED

#include "../xtechnical_common.hpp"
#include <vector>
#include <Eigen/Dense>

namespace xtechnical {

    template<class T = double>
    class PriceAnalysis {
    public:
	
		class Config {
		public:
			int m1_max_dec_counter 		= 5;
			int m1_max_len_local_level 	= 240;
			double m1_level_detection_accuracy = 0.25;
			int m1_max_level_detection_counter = 5;
		} config;
		
		class TrendData {
		public:
			int direction = 0;
			int length = 0;
			bool is_init = false;
		};
		
		class LevelData {
		public:
			std::vector<int> distance;
			bool is_init = false;
		};
		
		class CandleData {
		public:
			
			
		};
	
    private:
		circular_buffer<T> m_buffer;
		
		bool compare_prices(const T a, const T b,
		
		bool findLevels(const std::vector<T>& prices) {
			if (prices.emlty()) return false;
			
			// Локальный тренд
			TrendData local_trend;
			
			LevelData local_level;
			// Параметры последнего бара
			CandleData m1_last_candle;
			
			// Последняя цена
			const T last_price = prices.back();
			// Сумма разницы цен и максимум суммы разницы цен
			T price_diff_sum = 0, max_price_diff_sum = 0;
			// Счетчик непрерывного уменьшения площади
			int dec_counter = 0;
			// Сумма размеров бара
			T sum_candle_size = 0;
			// Счетчик локального уровня
			int local_level_counter = 0;
			bool is_local_level_detection = false;
			
			const int max_index = prices.size() - 1;
			
			for (int i = max_index - 1; i > 0; ++i) {
				const int length = max_index - i + 1;
				
				const T &price = prices[i];
				const T &prev_price = prices[i - 1];
				
				const T diff_last_price = last_price - price;
				const T diff_last_price_abs = std::abs(diff_last_price);
				const T candle_size = price - prev_price;
				
				sum_candle_size += candle_size;
				const T avg_candle_size = sum_candle_size / (T)length;

				// Ищем локальный тренд
				if (!local_trend.is_init) {
					price_diff_sum += diff_last_price;
					const T price_diff_sum_abs = std::abs(price_diff_sum);
					if (price_diff_sum_abs > max_price_diff_sum) {
						max_price_diff_sum = price_diff_sum_abs;
						dec_counter = 0;
					} else {
						++dec_counter;
						if (dec_counter >= config.m1_max_dec_counter) {
							local_trend.direction = price_diff_sum > 0 ? 1 : price_diff_sum < 0 ? -1 : 0;
							local_trend.length = length - config.m1_max_dec_counter;
							local_trend.is_init = true;
						}
					}
				}
				
				// Ищем ближайшие уровни
				if (length <= config.m1_max_len_local_level) {
					const T diff = config.m1_level_detection_accuracy * avg_candle_size;
					if (diff_last_price_abs < diff) {
						// Обнаружен уровень
						is_local_level_detection = true;
						local_level_counter = 0;
					} else {
						if (is_local_level_detection) {
							++local_level_counter;
							if (local_level_counter >= config.m1_max_level_detection_counter) {
								local_level.is_init = true;
								local_level.distance.push_back(length - config.m1_max_level_detection_counter);
								is_local_level_detection = false;
								local_level_counter = 0;
							}
						}
					}
				}

			}
		}

    public:

        PriceAnalysis() {};

        PriceAnalysis(const size_t window_len) : m_buffer(window_len) {}

        template<class InputType = double>
        inline bool update(const InputType in, const common::PriceType type = common::PriceType::Close) noexcept {
            m_buffer.update(in, type);
			if(m_buffer.full()) {
                
            } else {
                
            }
            return m_buffer.full();
        }

        inline void clear() noexcept {
            m_buffer.clear();
            m_reconstructed.clear();
            m_forecast.clear();
            m_metric = 0;
        }

        inline const bool full() noexcept {
            return m_buffer.full();
        }

        bool calc(const size_t horizon,
                  const size_t start_period,
                  const size_t num_period,
                  const size_t step_period,
                  const MetricType metric = MetricType::None,
                  const bool ssa_rec = false,
                  const SSAMode mode = SSAMode::RestoredSeriesAddition,
                  const size_t r = 0) {
            if (!m_buffer.full()) return false;
            if (start_period == 0) return false;

            if (num_period <= 1) {

                auto input_data = m_buffer.get_vec();

                auto vec = ssa_multi_tick<Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic>,Eigen::Matrix<T,Eigen::Dynamic,1>>(
                    input_data, horizon, start_period, r, mode);

                switch (metric) {
                case MetricType::RSquared:
                    m_metric = r_squared<Eigen::Matrix<T,Eigen::Dynamic,1>>(input_data, vec.head(vec.size() - horizon));
                    break;
                case MetricType::MAE:
                    m_metric = MAE<Eigen::Matrix<T,Eigen::Dynamic,1>>(input_data, vec.head(vec.size() - horizon));
                    break;
                case MetricType::MSE:
                    m_metric = MSE<Eigen::Matrix<T,Eigen::Dynamic,1>>(input_data, vec.head(vec.size() - horizon));
                    break;
                default:
                    break;
                };

                if (ssa_rec) {
                    m_reconstructed.resize(vec.size());
                    std::memcpy(m_reconstructed.data(), vec.data(), vec.size() * sizeof(T));
                    m_forecast.resize(horizon);
                    std::copy_backward(m_reconstructed.end() - horizon, m_reconstructed.end(), m_forecast.end());
                    return true;
                }

                Eigen::Map<Eigen::Matrix<T,Eigen::Dynamic,1>> vec_map(vec.tail(horizon).data(), horizon);
                m_forecast.resize(horizon);
                std::copy_backward(vec_map.data(), vec_map.data() + vec_map.size(), m_forecast.end());
                return true;
            }

            auto input_data = m_buffer.get_vec();

            const size_t len = ssa_rec ? (input_data.size() + horizon): horizon;
            Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> mat_mean(num_period, len);

            for (size_t n_period = 0; n_period < num_period; ++n_period) {
                const size_t period = n_period * step_period + start_period;
                mat_mean.row(n_period) = ssa_multi_tick<Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic>,Eigen::Matrix<T,Eigen::Dynamic,1>>(
                    input_data, horizon, period, r, mode);
            }

            Eigen::Matrix<T,Eigen::Dynamic,1> means = mat_mean.colwise().mean();

            switch (metric) {
            case MetricType::RSquared:
                m_metric = r_squared<Eigen::Matrix<T,Eigen::Dynamic,1>>(input_data, means.head(means.size() - horizon));
                break;
            case MetricType::MAE:
                m_metric = MAE<Eigen::Matrix<T,Eigen::Dynamic,1>>(input_data, means.head(means.size() - horizon));
                break;
            case MetricType::MSE:
                m_metric = MSE<Eigen::Matrix<T,Eigen::Dynamic,1>>(input_data, means.head(means.size() - horizon));
                break;
            default:
                break;
            };

            if (ssa_rec) {
                m_reconstructed.resize(means.size());
                std::memcpy(m_reconstructed.data(), means.data(), means.size() * sizeof(T));
                m_forecast.resize(horizon);
                std::copy_backward(m_reconstructed.end() - horizon, m_reconstructed.end(), m_forecast.end());
                return true;
            }

            Eigen::Map<Eigen::Matrix<T,Eigen::Dynamic,1>> vec_map(means.tail(horizon).data(), horizon);
            m_forecast.resize(horizon);
            std::copy_backward(vec_map.data(), vec_map.data() + vec_map.size(), m_forecast.end());
            return true;
        }

        inline const T get_last_forecast() {
            if (m_forecast.empty()) return std::numeric_limits<T>::quiet_NaN();
            return m_forecast.back();
        }

        inline const std::vector<T> &get_forecast() {
            return m_forecast;
        }

        inline const std::vector<T> &get_reconstructed() {
            return m_reconstructed;
        }

        inline const T get_metric() {
            return m_metric;
        }

    };
}; // xtechnical

#endif // XTECHNICAL_SSA_HPP_INCLUDED
