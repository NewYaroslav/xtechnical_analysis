#ifndef XTECHNICAL_COMPARE_HPP_INCLUDED
#define XTECHNICAL_COMPARE_HPP_INCLUDED

namespace xtechnical {

    /** \brief Сравнение двух переменных с плавающей точкой
     * \param x Первая переменная
     * \param y Вторая переменная
     * \return Вернет true, если две переменные ранвы
     */
    template<class T>
    bool combined_tolerance_compare(const T x, const T y) {
        double maxXYOne = std::max( { 1.0, std::fabs(x) , std::fabs(y) } ) ;
        return std::fabs(x - y) <= std::numeric_limits<T>::epsilon() * maxXYOne;
    }
}

#endif // XTECHNICAL_COMPARE_HPP_INCLUDED
