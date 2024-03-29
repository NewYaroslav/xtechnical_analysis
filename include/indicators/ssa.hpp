#ifndef XTECHNICAL_SSA_HPP_INCLUDED
#define XTECHNICAL_SSA_HPP_INCLUDED

#include "../xtechnical_common.hpp"
#include <vector>
#include <Eigen/Dense>

namespace xtechnical {

    template<class T = double>
    class SSA {
    public:

        enum class SSAMode {
            RestoredSeriesAddition,
            OriginalSeriesAddition,
            OriginalSeriesForecast,
        };

        enum class MetricType {
            None,
            MAE,
            MSE,
            RSquared,
        };

    private:

        template<class VectorType = Eigen::VectorXd>
        class CircularBuffer {
        public:

            CircularBuffer() {};

            CircularBuffer(const size_t size) {
                m_buffer = VectorType::Zero(size);
                m_head = 0;
                m_count = 0;
                m_intrabar_head = 0;
                m_intrabar_count = 0;
                m_intrabar = false;
            }

            template<class InputType = double>
            inline void update(const InputType value, const common::PriceType type = common::PriceType::Close) noexcept {
                if (type == common::PriceType::IntraBar) {
                    m_intrabar = true;
                    m_intrabar_buffer = m_buffer;
                    m_intrabar_head = m_head;
                    m_intrabar_count = m_count;
                    m_intrabar_buffer(m_intrabar_head) = value;
                    m_intrabar_head = (m_intrabar_head + 1) % m_buffer.size();
                    if (m_intrabar_count < m_intrabar_buffer.size()) {
                        ++m_intrabar_count;
                    }
                } else {
                    m_intrabar = false;
                    m_buffer(m_head) = value;
                    m_head = (m_head + 1) % m_buffer.size();
                    if (m_count < m_buffer.size()) {
                        ++m_count;
                    }
                }
            }

            inline bool full() const noexcept {
                if (m_intrabar) return m_intrabar_count == m_intrabar_buffer.size();
                return m_count == m_buffer.size();
            }

            inline VectorType get_vec() const noexcept {
                VectorType vec(m_buffer.size());
                if (m_intrabar) {
                    const size_t bs = m_intrabar_buffer.size();
                    vec.segment(0, bs - m_intrabar_head) = m_intrabar_buffer.segment(m_intrabar_head, bs - m_intrabar_head);
                    if (m_intrabar_head > 0) {
                        vec.segment(bs - m_intrabar_head, m_intrabar_head) = m_intrabar_buffer.segment(0, m_intrabar_head);
                    }
                } else {
                    vec.segment(0, m_buffer.size() - m_head) = m_buffer.segment(m_head, m_buffer.size() - m_head);
                    if (m_head > 0) {
                        vec.segment(m_buffer.size() - m_head, m_head) = m_buffer.segment(0, m_head);
                    }
                }
                return std::move(vec);
            }

            inline void clear() noexcept {
                m_buffer = VectorType::Zero(m_buffer.size());
                m_head = 0;
                m_count = 0;
                m_intrabar_head = 0;
                m_intrabar_count = 0;
                m_intrabar = false;
            }

            inline const size_t size() noexcept {
                return m_buffer.size();
            }

        private:
            VectorType m_buffer;
            VectorType m_intrabar_buffer;
            int m_head;
            int m_intrabar_head;
            int m_count;
            int m_intrabar_count;
            bool m_intrabar = false;
        }; //

        template<class MatrixType = Eigen::MatrixXd, class VectorType = Eigen::VectorXd>
        inline static MatrixType hankel(const VectorType &c, const size_t r) noexcept {
            const size_t n = c.size() - r + 1;
            MatrixType H(n, r);
            H.col(0) = c.head(n);
            for (size_t col = 1; col < r; ++col) {
                const size_t offset = n - col;
                H.block(0, col, offset, 1) = c.segment(col, offset);
                H.block(offset, col, col, 1) = c.segment(offset + col, col);
            }
            return std::move(H);
        }

        /** \brief one-tick SSA forecast
         * url: http://strijov.com/sources/examples.php
         * \param x - time series, one-dimensional
         * \param K - period
         * \param r - rank of Hankel matrix
         * \param mode - SSA mode
         */
        template<class MatrixType = Eigen::MatrixXd, class VectorType = Eigen::VectorXd>
        inline static void ssa_tick(
                VectorType &x,
                const size_t K,
                const size_t r = 0,
                const SSAMode mode = SSAMode::RestoredSeriesAddition) {
            const size_t N = x.size();
            const size_t L = N - K + 1;
            MatrixType X = hankel<MatrixType, VectorType>(x, K);
            Eigen::JacobiSVD<MatrixType> svd(X, Eigen::ComputeThinU | Eigen::ComputeThinV);

            MatrixType U = svd.matrixU();
            MatrixType V = svd.matrixV();
            VectorType S = svd.singularValues();
            MatrixType Lambda = S.asDiagonal();

            int r1 = 0;
            for (int i = 0; i < S.size(); ++i) {
                if (S(i) > 0 && i > r1) {
                    r1 = i;
                }
            }
            r1 += 1;

            const int rh = ((int)r > r1 || r == 0) ? r1 : r;

            MatrixType pi = U.bottomRows(1).leftCols(rh);
            MatrixType Up = U.topRows(U.rows()-1).leftCols(rh);

            const auto sumsqr = pi.squaredNorm();
            const auto eps = std::numeric_limits<decltype(sumsqr)>::epsilon();

            MatrixType R = Up * pi.transpose();
            if (std::abs(sumsqr - 1.0) > eps) {
                R *= (1.0 / (1.0 - sumsqr));
            }

            switch (mode) {
            case SSAMode::RestoredSeriesAddition: {
                    MatrixType X1 = U.block(0, 0, U.rows(), rh) * Lambda.block(0, 0, rh, rh) * V.block(0, 0, V.rows(), rh).transpose();
                    VectorType ts(N);
                    ts.head(X1.rows()) = X1.col(0);
                    ts.tail(X1.cols() - 1) = X1.row(X1.rows()-1).segment(1, X1.cols()-1);
                    VectorType g = R.transpose() * ts.segment(ts.size() - L + 1, L - 1);
                    ts.conservativeResize(ts.size() + g.size());
                    ts.tail(g.size()) = g;
                    x = ts;
                } break;
            case SSAMode::OriginalSeriesAddition: {
                    MatrixType X1 = U.block(0, 0, U.rows(), rh) * Lambda.block(0, 0, rh, rh) * V.block(0, 0, V.rows(), rh).transpose();
                    VectorType ts(N);
                    ts.head(X1.rows()) = X1.col(0);
                    ts.tail(X1.cols() - 1) = X1.row(X1.rows()-1).segment(1, X1.cols()-1);
                    VectorType g = R.transpose() * ts.segment(ts.size() - L + 1, L - 1);
                    x.conservativeResize(x.size() + g.size());
                    x.tail(g.size()) = g;
                } break;
            case SSAMode::OriginalSeriesForecast: {
                    VectorType g = R.transpose() * x.segment(x.size() - L + 1, L - 1);
                    x.conservativeResize(x.size() + g.size());
                    x.tail(g.size()) = g;
                } break;
            default:
                break;
            };
        }

        /** \brief SSA forecast
         * \param x - time series, one-dimensional
         * \param M - number on the ticks to forecast after the end of the time series x
         * \param K - period
         * \param r - rank of Hankel matrix
         * \param mode - SSA mode
         */
        template<class MatrixType = Eigen::MatrixXd, class VectorType = Eigen::VectorXd>
        inline static VectorType ssa_multi_tick(
                const VectorType &x,
                const size_t M,
                const size_t K,
                const size_t r = 0,
                const SSAMode mode = SSAMode::RestoredSeriesAddition) {
            VectorType x1(x);
            for (size_t i = 0; i < M; ++i) {
                ssa_tick<MatrixType, VectorType>(x1, K, r, mode);
            }
            return std::move(x1);
        }

        template<class VectorType = Eigen::VectorXd>
        inline const T r_squared(const VectorType& data, const VectorType& predictions) {
            const T sum_x2 = (data.array().square()).sum();
            const T sum_y2 = (predictions.array().square()).sum();
            const T sum_xy = (data.array() * predictions.array()).sum();
            const T sum_x = data.array().sum();
            const T sum_y = predictions.array().sum();
            const T ssxx = sum_x2 - ((sum_x * sum_x) / (T)data.size());
            const T ssyy = sum_y2 - ((sum_y * sum_y) / (T)predictions.size());
            const T ssxy = sum_xy - (sum_x * sum_y) / (T)data.size();
            const T r = ssxy / std::sqrt(ssxx * ssyy);
            return r * r;
        }

        template<class VectorType = Eigen::VectorXd>
        inline const T MAE(const VectorType& x, const VectorType& y) {
            if (x.size() != y.size()) return -1;
            const T mae = (x - y).cwiseAbs().sum() / x.size();
            return mae;
        }

        template<class VectorType = Eigen::VectorXd>
        inline const T MSE(const VectorType& x, const VectorType& y) {
            if (x.size() != y.size()) return -1;
            T mse = (x - y).squaredNorm() / x.size();
            return mse;
        }

        CircularBuffer<Eigen::Matrix<T,Eigen::Dynamic,1>> m_buffer;
        std::vector<T> m_reconstructed;
        std::vector<T> m_forecast;
        T m_metric = 0;

    public:

        SSA() {};

        SSA(const size_t window_len) : m_buffer(window_len) {}

        /** \brief SSA forecast
         * \param x - time series, one-dimensional
         * \param M - number on the ticks to forecast after the end of the time series x
         * \param K - period
         * \param r - rank of Hankel matrix
         */
        inline static const Eigen::Matrix<T,Eigen::Dynamic,1> calc_ssa(
                const Eigen::Matrix<T,Eigen::Dynamic,1> &x,
                const size_t M,
                const size_t K,
                const size_t r = 0,
                const SSAMode mode = SSAMode::RestoredSeriesAddition) {
            auto x1 = ssa_multi_tick<
                Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic>,
                Eigen::Matrix<T,Eigen::Dynamic,1>>(x, M, K, r, mode);
            return std::move(x1);
        }

        template<class InputType = double>
        inline bool update(const InputType in, const common::PriceType type = common::PriceType::Close) noexcept {
            m_buffer.update(in, type);
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
