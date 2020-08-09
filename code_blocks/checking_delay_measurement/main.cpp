#include <iostream>
#include "xtechnical_delay_meter.hpp"
#include <random>

#if(0)
class DelayMeter {
private:
    std::mutex buffer_mutex;
    xtechnical::circular_buffer<double> first_buffer;
    xtechnical::circular_buffer<double> second_buffer;
    uint64_t first_start_time = 0;
    uint64_t second_start_time = 0;
    //uint64_t first_last_time = 0;
    //uint64_t second_last_time = 0;

    uint64_t first_diff_time = 0;
    uint64_t second_diff_time = 0;

    uint64_t last_time = 0;
    uint64_t time_step = 0;
    size_t buffer_size = 0;
    size_t window_size = 0;

    /* данные на вывод */
    std::mutex output_mutex;
    int64_t first_offset_window = 0;
    double first_delay = std::numeric_limits<double>::quiet_NaN();
    double first_correlation = 0;

    std::mutex calc_future_mutex;
    std::future<void> calc_future;
    std::atomic<bool> is_ready = ATOMIC_VAR_INIT(false);

    uint64_t time_rounding(const uint64_t timestamp) {
        return timestamp - (timestamp %  time_step);
    }
public:
    DelayMeter() {};

    DelayMeter(const size_t user_buffer_size, const size_t user_window_size, const uint64_t user_time_step) :
        first_buffer(user_buffer_size),
        second_buffer(user_buffer_size),
        time_step(user_time_step),
        buffer_size(user_buffer_size),
        window_size(user_window_size) {
    }

    ~DelayMeter() {
        std::lock_guard<std::mutex> lock(calc_future_mutex);
        if(calc_future.valid()) {
            try {
                calc_future.wait();
                calc_future.get();
            }
            catch(const std::exception &e) {
                std::cerr << "Error: ~DelayMeter(), what: " << e.what() << std::endl;
            }
            catch(...) {
                std::cerr << "Error: ~DelayMeter()" << std::endl;
            }
        }
    };

    /** \brief Обновить состояние индикатора
     *
     * \param price Цена нового тика
     * \param ftimestamp Метка времени
     * \param index Индекс
     */
    void update(const double price, const double ftimestamp, const uint32_t index) {
        const uint64_t timestamp = time_rounding(ftimestamp * 1000.0d);
        if(last_time == 0) last_time = timestamp;
        std::lock_guard<std::mutex> lock(buffer_mutex);
        if(last_time < timestamp) {
            const double first_temp = first_buffer.back();
            const double second_temp = second_buffer.back();
            for(uint64_t t = (last_time + time_step); t <= timestamp; t += time_step) {
                first_buffer.push_back(first_temp);
                second_buffer.push_back(second_temp);
            }
        } else
        if(last_time > timestamp) {
            if(index == 0) {
                for(uint64_t t = last_time; t <= timestamp; t += time_step) {
                    first_buffer.back() = price;
                }
            } else
            if(index == 1) {
                for(uint64_t t = last_time; t <= timestamp; t += time_step) {
                    second_buffer.back() = price;
                }
            }
        }
        last_time = timestamp;
        if(index == 0) {
            first_buffer.back() = price;
            if(first_start_time == 0) first_start_time = timestamp;
        } else
        if(index == 1) {
            second_buffer.back() = price;
            if(second_start_time == 0) second_start_time = timestamp;
        }
        first_diff_time = timestamp - first_start_time;
        second_diff_time = timestamp - second_start_time;
    }

    void calc() {
        {
            std::lock_guard<std::mutex> lock(buffer_mutex);
            if(!first_buffer.full()) return;
            if(!second_buffer.full()) return;
            if(first_diff_time < buffer_size) return;
            if(second_diff_time < buffer_size) return;
        }

        /* сначала получаем данные */
        std::vector<double> first_data(buffer_size);
        std::vector<double> second_data(buffer_size);
        {
            std::lock_guard<std::mutex> lock(buffer_mutex);
            for(uint32_t i = 0; i < buffer_size; ++i) {
                first_data[i] = first_buffer[i];
                second_data[i] = second_buffer[i];
            }
        }
        const int32_t start_index = buffer_size - window_size;
        const int32_t max_offset = start_index + 1;

        /* сначала получаем текущее окно данных */
        std::vector<double> first_start_window(first_data.begin() + start_index, first_data.end());
        std::vector<double> second_start_window(second_data.begin() + start_index, second_data.end());
        std::vector<double> first_test_window(window_size);
        std::vector<double> second_test_window(window_size);

        xtechnical_normalization::calculate_min_max(first_start_window, first_start_window, 0);
        xtechnical_normalization::calculate_min_max(second_start_window, second_start_window, 0);

        /* сравниваем первое окно со вторым */
        double first_pearson_correlation = 0;
        int32_t first_offset = 0;
        for(int32_t offset = 0;  offset < max_offset; ++offset) {
            const int32_t index = start_index - offset;
            std::copy(second_data.begin() + index, second_data.begin() + index + window_size, second_test_window.begin());
            /* нормализуем и считаем корреляцию */
            double pearson_correlation = 0;
            xtechnical_normalization::calculate_min_max(second_test_window, second_test_window, 0);
            xtechnical_correlation::calculate_pearson_correlation_coefficient(first_start_window, second_test_window, pearson_correlation);

            if(std::abs(pearson_correlation) >= std::abs(first_pearson_correlation)) {
                first_pearson_correlation = pearson_correlation;
                first_offset = offset;
            }
        }

        /* сравниваем второе окно с первым */
        double second_pearson_correlation = 0;
        int32_t second_offset = 0;
        for(int32_t offset = 0;  offset < max_offset; ++offset) {
            const int32_t index = start_index - offset;
            std::copy(first_data.begin() + index, first_data.begin() + index + window_size, first_test_window.begin());
            /* нормализуем и считаем корреляцию */
            double pearson_correlation = 0;
            xtechnical_normalization::calculate_min_max(first_test_window, first_test_window, 0);
            xtechnical_correlation::calculate_pearson_correlation_coefficient(second_start_window, first_test_window, pearson_correlation);

            if(std::abs(pearson_correlation) >= std::abs(second_pearson_correlation)) {
                second_pearson_correlation = pearson_correlation;
                second_offset = offset;
            }
        }
        if(std::abs(second_pearson_correlation) > std::abs(first_pearson_correlation)) {
            std::lock_guard<std::mutex> lock(output_mutex);
            first_offset_window = -second_offset;
            first_correlation = second_pearson_correlation;
            first_delay = (double)first_offset_window * (double)time_step / 1000.0d;
        } else {
            std::lock_guard<std::mutex> lock(output_mutex);
            first_offset_window = first_offset;
            first_correlation = first_pearson_correlation;
            first_delay = (double)first_offset_window * (double)time_step / 1000.0d;
        }
        is_ready = true;
    }

    void asyn_update(const double price, const double ftimestamp, const uint32_t index) {
        try {
            std::lock_guard<std::mutex> lock(calc_future_mutex);
            if(calc_future.valid()) {
                std::future_status status = calc_future.wait_for(std::chrono::milliseconds(0));
                if(status == std::future_status::ready) {
                    calc_future.get();
                    calc_future = std::async(std::launch::async,[&,price,ftimestamp,index] {
                        update(price, ftimestamp, index);
                        calc();
                    });
                }
            } else {
                calc_future = std::async(std::launch::async,[&,price,ftimestamp,index] {
                    update(price, ftimestamp, index);
                    calc();
                });
            }
        }
        catch(...) {}
    }

    void asyn_calc() {
        try {
            std::lock_guard<std::mutex> lock(calc_future_mutex);
            if(calc_future.valid()) {
                std::future_status status = calc_future.wait_for(std::chrono::milliseconds(0));
                if(status == std::future_status::ready) {
                    calc_future.get();
                    calc_future = std::async(std::launch::async,[&] {
                        calc();
                    });
                }
            } else {
                calc_future = std::async(std::launch::async,[&] {
                    calc();
                });
            }
        }
        catch(...) {}
    }

    /** \brief Получить значение задержки
     * \return Значение задержки
     */
    double get_delay() {
        std::lock_guard<std::mutex> lock(output_mutex);
        return first_delay;
    }

    /** \brief Получить значение корреляции
     * \return Значение корреляции
     */
    double get_pearson_correlation() {
        std::lock_guard<std::mutex> lock(output_mutex);
        return first_correlation;
    }

    /** \brief Проверить наличие данных
     * \return Если данные заполнены, метод вернет true
     */
    bool check_full_data() {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        if(!first_buffer.full()) return false;
        if(!second_buffer.full()) return false;
        if(first_diff_time < buffer_size) return false;
        if(second_diff_time < buffer_size) return false;
        return true;
    }

    /** \brief Проверить возможность чтения результата
     * \return Если результат для чтения уже готов, метод вернет true
     */
    bool check_ready() {
        return is_ready;
    }

    void clear_ready_status() {
        is_ready = false;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        first_buffer.clear();
        second_buffer.clear();
        first_diff_time = 0;
        second_diff_time = 0;
        last_time = 0;
        first_start_time = 0;
        second_start_time = 0;
    }
};
#endif

int main() {
    std::cout << "Hello world!" << std::endl;
    xtechnical::DelayMeter delay_meter(12000, 6000, 50);
    std::vector<double> first_data;
    std::vector<double> second_data;

    std::uniform_real_distribution<double> unif(10,100);
    std::uniform_real_distribution<double> unif2(0.1,10);
    std::default_random_engine re;

    for(size_t i = 0; i < 100000; ++i) {
        double p = unif(re);
        first_data.push_back(p);
        second_data.push_back(p + unif2(re));
    }

    // std::vector<double> second_data(first_data.begin() + 12, first_data.end());

    for(size_t i = 0; i < second_data.size() - 1000; ++i) {
        double ftimestamp = 1000.0d + ((double)i * 0.05d);
        delay_meter.asyn_update(first_data[i], ftimestamp, 0);
        delay_meter.asyn_update(second_data[i + 15], ftimestamp, 1);
        //delay_meter.update(first_data[i], ftimestamp, 0);
        //delay_meter.update(second_data[i + 15], ftimestamp, 1);
        //delay_meter.asyn_calc();
        if(delay_meter.check_full_data()) {
            while(true) {
                if(delay_meter.check_ready())  {
                    std::cout << "delay = " << delay_meter.get_delay() << " corr = " << delay_meter.get_pearson_correlation() << std::endl;
                    delay_meter.clear_ready_status();
                    break;
                } else {
                    std::cout << "wait " << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }

        }
    }

    return 0;
}
