#include <iostream>
#include "xtechnical_circular_buffer.hpp"
#include <array>

int main() {
    std::cout << "Hello world!" << std::endl;
    std::array<double, 20> test_data = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};

    /* создадим кольцевой буфер размером 8 элементов */
    const size_t buffer_size = 8;
    xtechnical::circular_buffer<double> circular_buffer(buffer_size);

    /* заполняем буфер данными и выводим на экран */
    for(size_t i = 0; i < test_data.size(); ++i) {
        circular_buffer.push_back(test_data[i]);
        std::cout
            << "push " << test_data[i]
            << " front " << circular_buffer.front()
            << " back " << circular_buffer.back()
            << " [0] " << circular_buffer[0]
            << " [" << (buffer_size - 1) << "] " << circular_buffer[buffer_size - 1]
            << " full " << circular_buffer.full()
            << std::endl;
    }

    /* выводим на экран данные буфера */
    for(size_t i = 0; i < buffer_size; ++i) {
        std::cout
            << " [" << i << "] " << circular_buffer[i]
            << std::endl;
    }

    /* флаги буфера */
    std::cout << "empty " << circular_buffer.empty() << std::endl;
    std::cout << "full " << circular_buffer.full() << std::endl;

    /* первый и последний эхлемент буфера */
    std::cout << "front " << circular_buffer.front() << std::endl;
    std::cout << "back " << circular_buffer.back() << std::endl;

    /* проверяем методы тест */
    std::cout << "test(20)" << std::endl;
    circular_buffer.test(20);
    std::cout << "back " << circular_buffer.back() << std::endl;
    std::cout << "test(21)" << std::endl;
    circular_buffer.test(21);
    std::cout << "back " << circular_buffer.back() << std::endl;
    std::cout << "update(20)" << std::endl;
    circular_buffer.update(20);
    std::cout << "back " << circular_buffer.back() << std::endl;

    /* выводим на экран данные буфера */
    for(size_t i = 0; i < buffer_size; ++i) {
        std::cout
            << " [" << i << "] " << circular_buffer[i]
            << std::endl;
    }

    /* получаем значения среднего элемента, суммы и среднего значения буфера */
    std::cout << "middle " << circular_buffer.middle() << std::endl;
    std::cout << "sum " << circular_buffer.sum() << std::endl;
    std::cout << "mean " << circular_buffer.mean() << std::endl;

    /* очищаем и повторно заполняем буфер данными и выводим на экран */
    circular_buffer.clear();
    for(size_t i = 0; i < test_data.size(); ++i) {
        circular_buffer.push_back(test_data[i]);
        std::cout
            << "push " << test_data[i]
            << " front " << circular_buffer.front()
            << " back " << circular_buffer.back()
            << " [0] " << circular_buffer[0]
            << " [" << (buffer_size - 1) << "] " << circular_buffer[buffer_size - 1]
            << " full " << circular_buffer.full()
            << std::endl;
    }

    std::vector<double> temp = circular_buffer.to_vector();
    for(size_t i = 0; i < temp.size(); ++i) {
        std::cout
            << "temp[" << i << "] " << temp[i]
            << std::endl;
    }

    for(size_t i = 0; i < test_data.size(); ++i) {
        std::cout << "***" << std::endl;
        circular_buffer.push_back(test_data[i]);
        std::vector<double> temp = circular_buffer.to_vector();
        for(size_t i = 0; i < temp.size(); ++i) {
            std::cout
                << "temp[" << i << "] " << temp[i]
                << std::endl;
        }
        std::system("pause");
    }
    for(size_t i = 0; i < test_data.size(); ++i) {
        std::cout << "***" << std::endl;
        circular_buffer.push_back(test_data[i]);
        std::vector<double> temp = circular_buffer.to_vector();
        for(size_t i = 0; i < temp.size(); ++i) {
            std::cout
                << "temp[" << i << "] " << temp[i]
                << std::endl;
        }
        std::system("pause");
    }

    return 0;
}
