#ifndef XTECHNICAL_PERIOD_STATS_HPP_INCLUDED
#define XTECHNICAL_PERIOD_STATS_HPP_INCLUDED

#include "../xtechnical_common.hpp"

namespace xtechnical {

    /** \brief Статистика за период
     */
    template<class T>
    class PeriodStatsV1 {
    private:
        // время / значение
        std::map<uint64_t, T> data;
        uint64_t start_time = 0;
        uint64_t last_time = 0;
        uint64_t life_time = 0;

        inline void remove(const uint64_t time, const uint64_t lifetime) noexcept {
            // удаляем все что меньше нашей метки времени
            const uint64_t end_life_time = time - lifetime; // время последних данных
            auto it = data.upper_bound(end_life_time);
            if (it != data.end()) data.erase(data.begin(), it);
        }

    public:

        PeriodStatsV1() {};

        /** \brief Конструктор
         * \param user_life_time Время хранения данных
         */
        PeriodStatsV1(const uint64_t user_life_time) :
            life_time(user_life_time) {
        }

        /** \brief Добавить значение
         * \param value     Значение
         * \param time      Время значения
         */
        inline void add(const int value, const uint64_t time) noexcept {
            data[time] = value;
            // удаляем устаревшие данные
            remove(time, life_time);
            last_time = time;
            if (start_time == 0) start_time = time;
        }

        /** \brief Проверить наличие данных
         */
        inline bool empty() noexcept {
            return data.empty();
        }

        /** \brief Получить максимальное значение
         */
        inline T get_max_value() noexcept {
            T max_values = std::numeric_limits<T>::lowest();
            for (auto &item : data) {
                max_values = std::max(max_values, item.second);
            }
            return max_values;
        }

        /** \brief Получить минимальное
         */
        inline T get_min_value() noexcept {
            T min_value = std::numeric_limits<T>::max();
            for (auto &item : data) {
                min_value = std::min(min_value, item.second);
            }
            return min_value;
        }

        /** \brief Получить значение с максимальным весом
         */
        inline T get_max_weight() noexcept {
            std::multiset<T> counter;
            int max_counter = 0;
            T value = 0;
            for (auto &item : data) {
                counter.insert(item.second);
                const int c = counter.count(item.second);
                if (c >= max_counter) {
                    max_counter = c;
                    value = item.second;
                }
            }
            return value;
        }

        /** \brief Получить центр масс
         */
        inline T get_center_mass() noexcept {
            T sum = 0;
            for (auto &item : data) {
                sum += item.second;
            }
            return sum / data.size();
        }

        /** \brief Проверить заполненность данными
         * \return Вернет true, если данные зполнены
         */
        inline bool init() noexcept {
            const uint64_t diff = (last_time - start_time);
            return (diff >= life_time);
        }

        inline void clear() noexcept {
            data.clear();
            start_time = 0;
        }
    };

    /** \brief Статистика за период
     */
    class PeriodStatsV2 {
    private:
        // значение / время / win / loss
        std::map<int, std::map<uint64_t, std::pair<int, int>>> data;
        uint64_t start_time = 0;
        uint64_t last_time = 0;
        uint64_t life_time = 0;

        inline void remove(const uint64_t time, const uint64_t lifetime) noexcept {
            // удаляем все что меньше нашей метки времени
            const uint64_t end_life_time = time - lifetime; // время последних данных
            for (auto &item : data) {
                auto it = item.second.upper_bound(end_life_time);
                if (it != item.second.end()) item.second.erase(item.second.begin(), it);
            }
            // удаляем пустые данные
            auto it = data.begin();
            while(it != data.end()) {
                if (it->second.empty()) it = data.erase(it);
                else it++;
            }
        }

    public:

        PeriodStatsV2() {};

        /** \brief Конструктор
         * \param user_life_time Время хранения данных
         */
        PeriodStatsV2(const uint64_t user_life_time) :
            life_time(user_life_time) {
        }

        /** \brief Добавить значение
         * \param value     Значение
         * \param time      Время значения
         * \param result    Результат прогноза (1 - удачный, -1 - неудачный)
         */
        inline void add(const int value, const uint64_t time, const int result) noexcept {
            // добавляем статистику
            auto it = data.find(value);
            if (it == data.end()) {
                // статистики по значению нету, добавляем
                if (result > 0) data[value][time] = std::pair<int, int>(result, 0);
                else if (result < 0) data[value][time] = std::pair<int, int>(0, -result);
            } else {
                // если значение есть, ищем время
                auto it2 = it->second.find(time);
                if (it2 == it->second.end()) {
                    // если время не найдено, добавляем
                    if (result > 0) data[value][time] = std::pair<int, int>(result, 0);
                    else if (result < 0) data[value][time] = std::pair<int, int>(0, -result);
                } else {
                    // если время найдено, учитываем статистику
                    if (result > 0) it2->second.first += result;
                    else if (result < 0) it2->second.second += -result;
                }
            }
            // удаляем устаревшие данные
            remove(time, life_time);
            last_time = time;
            if (start_time == 0) start_time = time;
        }

        /** \brief Проверить наличие данных
         */
        inline bool empty() noexcept {
            return !data.empty();
        }

        /** \brief Получить максимальное значение
         */
        inline int get_max_value() noexcept {
            int max_values = std::numeric_limits<int>::min();
            for (auto &item : data) {
                max_values = std::max(max_values, item.first);
            }
            return max_values;
        }

        /** \brief Класс для хранения статистики
         */
        class Stats {
        public:
            std::vector<int> 	    values;
            std::vector<uint32_t> 	wins;
            std::vector<uint32_t> 	losses;
            std::vector<uint32_t> 	deals;
            std::vector<double> 	winrates;
            uint32_t    total_deals = 0;
            uint32_t    total_wins = 0;
            uint32_t    total_losses = 0;
            double      total_winrate = 0;
        };

        inline Stats calc() noexcept {
            Stats stats;
            for (auto &item : data) {
                stats.values.push_back(item.first);
                uint32_t wins = 0;
                uint32_t losses = 0;
                for (auto &item2 : item.second) {
                    wins += item2.second.first;
                    losses += item2.second.second;
                }

                stats.wins.push_back(wins);
                stats.losses.push_back(losses);
                const uint32_t d = wins + losses;
                stats.deals.push_back(d);
                const double w = d == 0 ? 0 : (double)wins / (double)d;
                stats.winrates.push_back(w);
                stats.total_wins += wins;
                stats.total_losses += losses;
            }
            stats.total_deals = stats.total_wins + stats.total_losses;
            stats.total_winrate = stats.total_deals == 0 ? 0 : (double)stats.total_wins / (double)stats.total_deals;
            return std::move(stats);
        };

        inline Stats calc_norm(const uint32_t threshold_deals) noexcept {
            Stats stats;
            int start_value = 0;
            uint32_t wins = 0;
            uint32_t losses = 0;
            bool is_init_element = false;
            for (auto &item : data) {

                if (!is_init_element) {
                    is_init_element = true;
                    start_value = item.first;
                }

                for (auto &item2 : item.second) {
                    wins += item2.second.first;
                    losses += item2.second.second;
                    stats.total_wins += item2.second.first;
                    stats.total_losses += item2.second.second;
                }

                const uint32_t d = wins + losses;
                if (d >= threshold_deals) {
                    const double w = d == 0 ? 0 : (double)wins / (double)d;
                    stats.values.push_back(start_value);
                    stats.wins.push_back(wins);
                    stats.losses.push_back(losses);
                    stats.deals.push_back(d);
                    stats.winrates.push_back(w);
                    is_init_element = false;
                    wins = 0;
                    losses = 0;
                }
            }
            if (is_init_element) {
                const uint32_t d = wins + losses;
                const double w = d == 0 ? 0 : (double)wins / (double)d;
                stats.values.push_back(start_value);
                stats.wins.push_back(wins);
                stats.losses.push_back(losses);
                stats.deals.push_back(d);
                stats.winrates.push_back(w);
            }
            stats.total_deals = stats.total_wins + stats.total_losses;
            stats.total_winrate = stats.total_deals == 0 ? 0 : (double)stats.total_wins / (double)stats.total_deals;
            return std::move(stats);
        };

        inline Stats calc_norm_up(const uint32_t threshold_deals) noexcept {
            Stats stats;
            int start_value = 0;
            uint32_t wins = 0;
            uint32_t losses = 0;
            bool is_init_element = false;
            for (auto &item : data) {

                if (!is_init_element) {
                    is_init_element = true;
                    start_value = item.first;
                    stats.values.push_back(start_value);
                    stats.wins.push_back(0);
                    stats.losses.push_back(0);
                    stats.deals.push_back(0);
                    stats.winrates.push_back(0);
                }

                for (auto &item2 : item.second) {
                    wins += item2.second.first;
                    losses += item2.second.second;
                    stats.total_wins += item2.second.first;
                    stats.total_losses += item2.second.second;
                    for (size_t i = 0; i < stats.values.size(); ++i) {
                        stats.wins[i] += item2.second.first;
                        stats.losses[i] += item2.second.second;
                    }
                }

                const uint32_t d = wins + losses;
                if (d >= threshold_deals) {
                    wins = 0;
                    losses = 0;
                    is_init_element = false;
                }
            }
            for (size_t i = 0; i < stats.values.size(); ++i) {
                stats.deals[i] = stats.wins[i] + stats.losses[i];
                stats.winrates[i] = stats.deals[i] == 0 ? 0 : (double)stats.wins[i] / (double)stats.deals[i];
            }
            stats.total_deals = stats.total_wins + stats.total_losses;
            stats.total_winrate = stats.total_deals == 0 ? 0 : (double)stats.total_wins / (double)stats.total_deals;
            return std::move(stats);
        };

        /** \brief Проверить заполненность данными
         * \return Вернет true, если данные зполнены
         */
        inline bool init() noexcept {
            const uint64_t diff = (last_time - start_time);
            return (diff >= life_time);
        }

        inline void clear() noexcept {
            data.clear();
            start_time = 0;
        }
    };

};

#endif // XTECHNICAL_PERIOD_STATS_HPP_INCLUDED
