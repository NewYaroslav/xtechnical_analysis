#ifndef XTECHNICAL_ORDINARY_LEAST_SQUARES_HPP_INCLUDED
#define XTECHNICAL_ORDINARY_LEAST_SQUARES_HPP_INCLUDED

namespace xtechnical {

    enum class OlsFunctionType {
        LINE = 0,           /**< Y = A1*X + A0 */
        PARABOLA = 1,       /**< Y = A2*X^2 + A1*X + A0 */
    };

    /** \brief Метод наименьших квадратов
     * Определение коэффициентов линейной аппроксимации по МНК
     * С помощью данной функции можно найти коэффициенты для функций
     * Y = A1*X + A0 или Y = A2*X^2 + A1*X + A0
     * \param x         Массив точек X
     * \param y         Массив точек Y
     * \param type_line Тип линии
     * \param coeff     Массив коэффициентов (2 либо 3 коэффициента)
     */
    template <typename T1, typename T2>
    void calc_ols(const T1 &x, const T1 &y, const OlsFunctionType type_line, T2& coeff) {
        if(type_line == OlsFunctionType::LINE) {
            double sx = 0, sy = 0, sx2 = 0, sxy = 0;
            for (size_t i = 0; i < x.size(); ++i) {
                sx += x[i];
                sy += y[i];
                sx2 += x[i] * x[i];
                sxy += x[i] * y[i];
            }
            //coeff.resize(2);
            coeff[1] = ((double)x.size() * sxy - (sx * sy)) / ((double)x.size() * sx2 - sx * sx);
            coeff[0] = (sy - coeff[0] * sx) / (double)x.size();
        } else
        if(type_line == OlsFunctionType::PARABOLA) {
            double sx = 0, sy = 0, sx2 = 0, sx3 = 0, sx4 = 0, sxy = 0, sx2y = 0;
            for (size_t i = 0; i < x.size(); ++i) {
                sx += x[i];
                sy += y[i];
                double m2 = x[i] * x[i];
                double m3 = m2 * x[i];
                sx2 += m2;
                sx3 += m3;
                sx4 += m3 * x[i];
                double mxy = x[i] * y[i];
                sxy += mxy;
                sx2y += x[i] * mxy;
            }
            double sxsx2 = sx*sx2;
            double sxsx4 = sx*sx4;
            double sx2sx2 = sx2*sx2;
            double sx2sx3 = sx2*sx3;
            double sxsx3 = sx*sx3;
            double nsx3 = (double)x.size()*sx3;
            /* найдем определитель матрицы
             * n   sx  sx2
             * sx  sx2 sx3
             * sx2 sx3 sx4
             */
            double A = (double)x.size() * (sx2 * sx4 - sx3 * sx3) - sx* (sx * sx4 - sx2 * sx3) + sx2 * (sx * sx3 - sx2 * sx2);
            A = 1.0/A;
            /* найдем транспонированную матрицу, она будет такой же
             * n   sx  sx2
             * sx  sx2 sx3
             * sx2 sx3 sx4
             * далее найдем определитель для матриц 2 x 2 и применим матрицу кофакторов
             * sx2*sx4-sx3*sx3  sx2*sx3-sx*sx4  sx*sx3-sx2*sx2
             * sx3*sx2-sx*sx4   n*sx4-sx2*sx2   sx*sx2-n*sx3
             * sx*sx3-sx2*sx2   sx*sx2-n*sx3    n*sx2-sx*sx
             * далее каждый элемент надо делить на определитель
             */
            //coeff.resize(3);
            coeff[0] = A * ((sx2*sx4 - sx3*sx3) * sy + (sx2sx3 - sxsx4) * sxy + (sxsx3 - sx2sx2) * sx2y);
            coeff[1] = A * ((sx2sx3 - sxsx4) * sy + ((double)x.size()*sx4 - sx2sx2) * sxy + (sxsx2 - nsx3) * sx2y);
            coeff[2] = A * ((sxsx3 - sx2sx2) * sy + (sxsx2 - nsx3) * sxy + ((double)x.size()*sx2 - sx*sx) * sx2y);
        }
    }

    template <typename T1, typename T2>
    T2 calc_ols_line(const T1& coeff, const T2 x) {
        if(coeff.size() == 2) {
            return coeff[1] * x + coeff[0];
        } else
        if(coeff.size() == 3) {
            return coeff[2] * x * x + coeff[1] * x + coeff[0];
        }
        return 0;
    }

};

#endif // XTECHNICAL_ORDINARY_LEAST_SQUARES_HPP_INCLUDED
