#ifndef XTECHNICAL_INDEXES_HPP_INCLUDED
#define XTECHNICAL_INDEXES_HPP_INCLUDED

#include "delay_line.hpp"
#include "stochastic.hpp"

namespace xtechnical {

    enum IndexesPairType {
        AUDUSD = 0,
        EURUSD,
        GBPUSD,
        NZDUSD,
        USDCAD,
        USDCHF,
        USDJPY,
    };

    enum IndexesCurrencyType {
        USD = 0,
        EUR,
        GBP,
        JPY,
        CHF,
        AUD,
        CAD,
        NZD,
    };

    /** \brief
     * "AUDUSD","EURUSD","GBPUSD","NZDUSD","USDCAD","USDCHF","USDJPY"
     * "USD","EUR","GBP","JPY","CHF","AUD","CAD","NZD"
     */
    template<class T = double,
        template <class...> class MA_TYPE = xtechnical::SMA,
        template <class...> class INDIC_TYPE = xtechnical::Stochastic>
    class Indexes : public BaseIndicator<T> {
    private:
		std::array<T, 7>    m_start_prices;
        std::array<DelayLine<T>, 7> m_delay_lines;
        std::array<MA_TYPE<T>, 8>   m_mas;
        std::array<TYPE<T>, 8>      m_indics;

        IndicatorType m_type = IndicatorType::Indexes;

        bool m_save_start_prices = false;
        bool m_init_start_prices = false;
        bool m_reverse = false; // Реверс значений индикатора
        bool m_use_indics = false;

        bool calc_indexes(const std::array<T, 7> &start_prices, const std::array<T, 7> &prices, const PriceType type) {
            std::array<T, 8> currency_indexes;
            currency_indexes[0] = 1.0;
            for (size_t i = 0; i < 4; ++i) {
                currency_indexes[0] *= (prices[i] / start_prices[i]);
            }
            for (size_t i = 4; i < 7; ++i) {
                currency_indexes[0] *= (start_prices[i] / prices[i]);
            }
            currency_indexes[0] = 100 * std::pow(currency_indexes[0],0.125);
            currency_indexes[1] = currency_indexes[0] * prices[1] / start_prices[1];
            currency_indexes[2] = currency_indexes[0] * prices[2] / start_prices[2];
            currency_indexes[3] = currency_indexes[0] * start_prices[6] / prices[6];
            currency_indexes[4] = currency_indexes[0] * start_prices[5] / prices[5];
            currency_indexes[5] = currency_indexes[0] * prices[0] / start_prices[0];
            currency_indexes[6] = currency_indexes[0] * start_prices[4] / prices[4];
            currency_indexes[7] = currency_indexes[0] * prices[4] / start_prices[4];

            if (m_reverse) {
                for (size_t i = 0; i < currency_indexes.size(); ++i) {
                    currency_indexes[i] = 200 - currency_indexes[i];
                }
            }

            if (m_use_indics) {
                size_t ready_counter = 0;
                for (size_t i = 0; i < currency_indexes.size(); ++i) {
                    m_indics[i].update(currency_indexes[i], type);
                    if (m_indics[i].is_ready()) {
                        currency_indexes[i] = m_indics[i].get();
                        ++ready_counter;
                    }
                }
                if (ready_counter < currency_indexes.size()) {
                    return false;
                }
            }

            if (m_use_mas) {
                size_t ready_counter = 0;
                for (size_t i = 0; i < currency_indexes.size(); ++i) {
                    m_mas[i].update(currency_indexes[i], type);
                    if (m_indics[i].is_ready()) {
                        currency_indexes[i] = m_indics[i].get();
                        ++ready_counter;
                    }
                }
                if (ready_counter < currency_indexes.size()) {
                    return false;
                }
            }

            for (size_t i = 0; i < currency_indexes.size(); ++i) {
                BaseIndicator<T>::output_value[i] = currency_indexes[i];
            }
            return true;
        }

    public:

        Indexes() : BaseIndicator<T>(8)  {};

        /** \brief
         * \param period        Период окна расчета индексов (указать 0, если не используется)
         * \param period_ma     Период сглаживания (указать 0, если не нужно)
         * \param period_indic  Период индикатора (указать 0, если индикатор не нужен)
         * \param is_reverse    Реверс сигналов
         */
        Indexes(const size_t period,
                const size_t period_ma = 0,
                const size_t period_indic = 0,
                const bool is_reverse = false) :
                BaseIndicator<T>(8),
                m_reverse(is_reverse) {
            for (size_t i = 0; i < m_delay_lines.size(); ++i) {
                m_delay_lines[i] = DelayLine<T>(period);
            }
            if (period_ma) {
                m_use_mas = true;
                for (size_t i = 0; i < m_mas.size(); ++i) {
                    m_mas[i] = MA_TYPE<T>(period_ma);
                }
            }
            if (period_indic) {
                m_use_indics = true;
                for (size_t i = 0; i < m_indics.size(); ++i) {
                    m_indics[i] = INDIC_TYPE<T>(period_indic);
                }
            }
        };

        inline void set_start_price(const std::array<T, 7> &prices) {
            m_start_prices = prices;
            m_init_start_prices = true;
        }

        inline void set_start_price() {
            m_save_start_prices = true;
        }

        /** \brief SSA forecast
         * \param prices - "AUDUSD","EURUSD","GBPUSD","NZDUSD","USDCAD","USDCHF","USDJPY"
         * \param type - rank of Hankel matrix
         */
        inline bool update(const std::array<T, 7> &prices, const common::PriceType type = common::PriceType::Close) noexcept {

            for (size_t i = 0; i < prices.size(); ++i) {
                m_delay_lines[i].update(prices[i], type);
            }

            if (!m_delay_lines[0].is_ready()) return false;

            for (size_t i = 0; i < prices.size(); ++i) {
                m_start_prices[i] = m_delay_lines[i].get();
            }

            calc_indexes(prices, m_start_prices);



            if (m_save_start_prices) {
                m_start_prices = prices;
                m_save_start_prices = false;
                m_init_start_prices = true;
            }
            if (m_init_start_prices) {
                calc_indexes(prices, m_start_prices);
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
