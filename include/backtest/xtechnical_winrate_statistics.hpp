#ifndef XTECHNICAL_WINRATE_STATISTICS_HPP_INCLUDED
#define XTECHNICAL_WINRATE_STATISTICS_HPP_INCLUDED

#include <string>
#include <map>
#include <deque>
#include <functional>

namespace xtechnical {

    /** \brief Статистика винрейта
     */
    template<class T>
    class WinrateStats {
    public:

        /** \brief Класс тика
         */
        class Tick {
        public:
            double bid = 0;         /**< Цена bid */
			double ask = 0;         /**< Цена ask */
            uint64_t timestamp = 0; /**< Метка времени */
			
			Tick() {};
			
			Tick(const double b, const double a, const uint64_t t) :
				bid(b), ask(a), timestamp(t) {
			}
        };

        /** \brief Класс данных ставок
         */
        class Bet {
        public:
            std::string broker;
            std::string symbol;
            int direction = 0;
            uint64_t t1 = 0;
            uint64_t t2 = 0;
            uint64_t last_t = 0;
            double open = 0;
            double close = 0;
            bool init_open = false;
            bool init_close = false;
            T user_data;
        };

    private:
        std::deque<Bet> bets;
        std::map<std::string, std::map<std::string, Tick>> ticks;

    public:

        /** \brief Класс конфигурации
         */
        class Config {
        public:
            uint64_t expiration = 60000;    /**< Экспирация */
            uint64_t delay = 150;           /**< Задержка */
            uint64_t period = 0;            /**< Период в миллисекундах */
            uint64_t between_ticks = 20000; /**< Задержка между тиками */
            std::function<void(const Bet &bet)> on_error = nullptr;
            std::function<void(const Bet &bet)> on_win = nullptr;
            std::function<void(const Bet &bet)> on_loss = nullptr;
        } config;

        uint64_t wins = 0;      /**< Число удачных сделок */
        uint64_t losses = 0;    /**< Число убыточных сделок */

        WinrateStats() {};

        /** \brief Сделать ставку
         * \param broker        Имя брокера
         * \param symbol        Символ
         * \param timestamp     Метка времени
         * \param direction     Направление, 1 - BUY, -1 - SELL
         * \param callback      Функция обратного вызова для передачи структуры ставки
         */
        void place_bet(
                const std::string &broker,
                const std::string &symbol,
                const uint64_t timestamp,
                const int direction,
                std::function<void(Bet &bet)> callback = nullptr) noexcept {
            Bet bet;
            bet.broker = broker;
            bet.symbol = symbol;
            bet.direction = direction;

            const uint64_t t1 = timestamp + config.delay;
            bet.t1 = config.period == 0 ? t1 : (t1 - (t1 % config.period) + config.period);
            bet.t2 = bet.t1 + config.expiration;
            // ищем котировку
            auto it_broker = ticks.find(broker);
            if (it_broker == ticks.end()) return;
            auto it_symbol = it_broker->second.find(symbol);
            if (it_symbol == it_broker->second.end()) return;

            bet.open = (it_symbol->second.ask + it_symbol->second.bid) / 2.0d;
            if (callback != nullptr) callback(bet);
            bets.push_back(bet);
        }

        /** \brief Обновить состояние сделок
         * \param broker        Имя брокера
         * \param symbol        Символ
         * \param tick          Данные тика
         */
        void update(
                const std::string &broker,
                const std::string &symbol,
                const Tick &tick) noexcept {
            ticks[broker][symbol] = tick;
            if (bets.empty()) return;
            size_t index = 0;
            while (index < bets.size()) {
                if (bets[index].broker != broker) {
                    ++index;
                    continue;
                }
                if (bets[index].symbol != symbol) {
                    ++index;
                    continue;
                }

                if (!bets[index].init_open && bets[index].t1 >= tick.timestamp) {
                    bets[index].open = (tick.ask + tick.bid) / 2.0d;
                }
                if (!bets[index].init_open && bets[index].t1 <= tick.timestamp) {
                    bets[index].init_open = true;
                }

                if (!bets[index].init_close && bets[index].t2 >= tick.timestamp) {
                    bets[index].close = (tick.ask + tick.bid) / 2.0d;
                    bets[index].last_t = tick.timestamp;
                }
                if (!bets[index].init_close && bets[index].t2 <= tick.timestamp) {
                    bets[index].init_close = true;
                    if ((bets[index].t2 - bets[index].last_t) > config.between_ticks) {
                        // ошибка сделки, слишком долго не было тика
                        if (config.on_error != nullptr) config.on_error(bets[index]);
                        bets.erase(bets.begin() + index);
                        continue;
                    }
                }

                if (!bets[index].init_close ||
                    !bets[index].init_open) {
                    ++index;
                    continue;
                }

                if (bets[index].direction == 1) {
                    if (bets[index].close > bets[index].open) {
                        ++wins;
                        if (config.on_win != nullptr) config.on_win(bets[index]);
                    } else {
                        ++losses;
                        if (config.on_loss != nullptr) config.on_loss(bets[index]);
                    }
                    bets.erase(bets.begin() + index);
                    continue;
                } else
                if (bets[index].direction == -1) {
                    if (bets[index].close < bets[index].open) {
                        ++wins;
                        if (config.on_win != nullptr) config.on_win(bets[index]);
                    } else {
                        ++losses;
                        if (config.on_loss != nullptr) config.on_loss(bets[index]);
                    }
                    bets.erase(bets.begin() + index);
                    continue;
                }
                ++index;
            } // while
        }
		
		inline void update(
                const std::string &broker,
                const std::string &symbol,
                const double bid,
				const double ask,
				const uint64_t timestamp) noexcept {
			update(broker, symbol, Tick(bid, ask, timestamp));	
		}

        /** \brief Получить винрейт
         * \return Винрейт
         */
        inline double get_winrate() noexcept {
            const double deals = wins + losses;
            const double winrate = deals == 0 ? 0 : (double)wins / (double)deals;
            return winrate;
        }

        /** \brief Получить число сделок
         * \return Число сделок
         */
        inline uint64_t get_deals() noexcept {
            const uint64_t deals = wins + losses;
            return deals;
        }
    };
};

#endif // XTECHNICAL_WINRATE_STATISTICS_HPP_INCLUDED
