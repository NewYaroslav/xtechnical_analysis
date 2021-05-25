#ifndef XTECHNICAL_CLUSTER_CLUSTER_HPP_INCLUDED
#define XTECHNICAL_CLUSTER_CLUSTER_HPP_INCLUDED

#include "../xtechnical_common.hpp"

namespace xtechnical {

    /** \brief Формирователь Кластеров
     */
    class ClusterShaper {
    public:

		/** \brief Кластер
		*/
		class Cluster {
		public:
			std::map<int, int> distribution;
			int open = 0;
			int close = 0;
			int high = 0;
			int low = 0;
			int volume = 0;
			int max_volume = 0;
			int max_index = 0;
			uint64_t timestamp = 0;
			double pips_size = 0.0;

			inline double get_close_price() noexcept {
				return (double)close * pips_size;
			}

			inline double get_open_price() noexcept {
				return (double)open * pips_size;
			}

			inline std::vector<double> get_array() const noexcept {
				if (distribution.empty()) return std::vector<double>();
				std::vector<double> temp;
				temp.reserve(distribution.size());
				int last_tick = 0;
				int index = 0;
				for (const auto &item : distribution) {
					if (index == 0) {
						temp.push_back(item.second);
						last_tick = item.first;
					} else {
						const int diff = (item.first - last_tick);
						last_tick = item.first;
						for(int i = 1; i < diff; ++i) {
							temp.push_back(0.0d);
						}
						temp.push_back(item.second);
					}
					++index;
				}
				return std::move(temp);
			}

			inline std::vector<double> get_normalized_array() const noexcept {
				std::vector<double> temp(get_array());
				if (max_volume > 0) {
					for (auto &item : temp) {
						item /= (double)max_volume;
					}
				} else return std::vector<double>(temp.size());
				return std::move(temp);
			}

			inline double get_max_volume_price() const noexcept {
				return (double)max_index * pips_size;
			}

			inline double get_center_mass_price() const noexcept {
				int64_t sum = 0;
				for (auto &item : distribution) {
					sum += item.first * item.second;
				}
				sum /= volume;
				return (double)sum * pips_size;
			}

			inline double get_center_mass() const noexcept {
				int64_t sum = 0;
				for (auto &item : distribution) {
					sum += item.first * item.second;
				}
				sum /= volume;
				return sum;
			}

			inline double get_center_mass_norm() const noexcept {
				const int diff = high - low;
				return diff == 0 ? 0.5 : (double)(get_center_mass() - std::min(high, low)) / (double)diff;
			}
		};

	private:
        Cluster cluster;
		uint64_t period = 0;
        uint64_t last_bar = 0;

        bool is_use_bar_stop_time = false;
        bool is_fill = false;
        bool is_once = false;
		double pips_size = 0;

    public:
        ClusterShaper() {};

        /** \brief Инициализировать формирователь баров
         * \param p     Период индикатора в секундах
         * \param ps  	Точность цены, например 0.00001
         * \param ubst  Флаг, включает использование последней метки времени бара вместо начала бара, как времени бара
         */
        ClusterShaper(const size_t p, const double ps, const bool ubst = false) :
			period(p), pips_size(ps), is_use_bar_stop_time(ubst)  {
        }

		std::function<void(const Cluster &cluster)> on_close_bar;               /**< Функция обратного вызова в момент закрытия бара */
        std::function<void(const Cluster &cluster)> on_unformed_bar = nullptr;	/**< Функция обратного вызова для несформированного бара */

        /** \brief Обновить состояние индикатора
         * \param input     Текущая цена
         * \param timestamp Метка времени в секундах
         */
        int update(const double input, const uint64_t timestamp) noexcept {
            if(period == 0) return common::NO_INIT;
			const uint64_t current_bar = timestamp / period;
			if (last_bar == 0) {
                last_bar = current_bar;
                return common::INDICATOR_NOT_READY_TO_WORK;
            }

			const int tick = (int)((input / pips_size) + 0.5d);

			if (current_bar > last_bar) {
				if (is_once) {
                    cluster.timestamp = is_use_bar_stop_time ?
                        (last_bar * period + period) : (last_bar * period);
                    on_close_bar(cluster);
					cluster.distribution.clear();
					cluster.distribution[tick] = 1;
					last_bar = current_bar;
					cluster.timestamp = is_use_bar_stop_time ?
                        (last_bar * period + period) : (last_bar * period);
					cluster.open = cluster.close = tick;
					cluster.volume = 1;
					cluster.max_volume = 1;
					cluster.max_index = tick;
					cluster.high = cluster.low = tick;
					cluster.pips_size = pips_size;
					return common::OK;
                }
				cluster.distribution.clear();
				cluster.distribution[tick] = 1;
				last_bar = current_bar;
				cluster.timestamp = is_use_bar_stop_time ?
                        (last_bar * period + period) : (last_bar * period);
				cluster.open = cluster.close = tick;
				cluster.volume = 1;
				cluster.max_volume = 1;
				cluster.max_index = tick;
				cluster.high = cluster.low = tick;
				cluster.pips_size = pips_size;
				is_once = true;
				return common::OK;
            } else
            if (current_bar == last_bar) {
                if (is_once) {
                    auto it = cluster.distribution.find(tick);
					if (it == cluster.distribution.end()) {
						cluster.distribution[tick] = 1;
					} else {
						++it->second;
						if (it->second > cluster.max_volume) {
							cluster.max_volume = it->second;
							cluster.max_index = tick;
						}
					}
					cluster.close = tick;
					if (tick > cluster.high) cluster.high = tick;
					if (tick < cluster.low) cluster.low = tick;
					++cluster.volume;
                    if (on_unformed_bar != nullptr) {
                        on_unformed_bar(cluster);
                    }
                }
            }
            return common::OK;
        }

		static inline std::vector<double> get_triangular_distribution(
				size_t length,
				size_t vertex_position) {
			if (length == 0) return std::vector<double>();
			if (length == 1) return std::vector<double>(1,1.0d);
			std::vector<double> temp(length, 0.0d);
			if (vertex_position >= length) vertex_position = length - 1;
			const double step_up = vertex_position <= 1 ? 1.0d : (1.0d / (double)vertex_position);
			const size_t diff = (length - 1) - vertex_position;
			const double step_dn = diff <= 1 ? 1.0d : (1.0d / (double)diff);
			double step = vertex_position == 0 ? 1.0d : 0.0d;
			for (size_t i = 0; i <= vertex_position; ++i) {
				temp[i] = step;
				step += step_up;
			}
			step = 1.0d - step_dn;
			for (size_t i = vertex_position + 1; i < length; ++i) {
				temp[i] = step;
				step -= step_dn;
			}
			return std::move(temp);
		}

		static inline double get_euclidean_distance(const  std::vector<double> &x, const std::vector<double> &y) {
			double sum = 0;
			for (size_t i = 0; i < x.size(); ++i) {
				const double diff = x[i] - y[i];
				sum += diff * diff;
			}
			return 1.0d / (1.0d + std::sqrt(sum));
		}

		static inline double get_cosine_similarity(const  std::vector<double> &x, const std::vector<double> &y) {
			double sum = 0;
			double sum_x = 0;
			double sum_y = 0;
			for (size_t i = 0; i < x.size(); ++i) {
				sum += x[i] * y[i];
				sum_x += x[i] * x[i];
				sum_y += y[i] * y[i];
			}
			return sum / (std::sqrt(sum_x) * std::sqrt(sum_y));
		}

        /** \brief Очистить данные индикатора
         */
        inline void clear() noexcept {
			cluster = Cluster();
			last_bar = 0;
			is_once = false;
        }
    }; // ClusterShaper

}; // xtechnical

#endif // XTECHNICAL_CLUSTER_CLUSTER_HPP_INCLUDED
