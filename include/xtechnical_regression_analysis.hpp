#ifndef XTECHNICAL_REGRESSION_ANALYSIS_HPP_INCLUDED
#define XTECHNICAL_REGRESSION_ANALYSIS_HPP_INCLUDED

namespace xtechnical_regression_analysis {

    enum LeastSquaresMethodType {
        LSM_LINE = 0,               /**< Y = A1*X + A0 */
        LSM_PARABOLA = 1,           /**< Y = A2*X^2 + A1*X + A0 */
    };

    /** \brief Метод наименьших квадратов
     * Определение коэффициентов линейной аппроксимации по МНК
     * С помощью данной функции можно найти коэффициенты для функций
     * Y = A1*X + A0 или Y = A2*X^2 + A1*X + A0
     * \param coeff массив коэффициентов (2 либо 3 коэффициента)
     * \param point двумерный массив точек
     * \param type тип линии
     */
    template <typename T1, typename T2>
    void calc_least_squares_method(T2& coeff, const T1& point, const uint32_t &type) {
        if(type == LSM_LINE) {
            double sx = 0, sy = 0, sx2 = 0, sxy = 0;
            size_t size_point = point.size();
            for(size_t i = 0; i < size_point; ++i) {
                sx += point[i].x;
                sy += point[i].y;
                sx2 += point[i].x * point[i].x;
                sxy += point[i].x * point[i].y;
            }
            //coeff.resize(2);
            coeff[1] = ((double)size_point * sxy - (sx * sy)) / ((double)size_point * sx2 - sx * sx);
            coeff[0] = (sy - coeff[0] * sx) / (double)size_point;
        } else
        if(type == LSM_PARABOLA) {
            double sx = 0, sy = 0, sx2 = 0, sx3 = 0, sx4 = 0, sxy = 0, sx2y = 0;
            size_t size_point = point.size();
            for(size_t i = 0; i < size_point; ++i) {
                sx += point[i].x;
                sy += point[i].y;
                double m2 = point[i].x * point[i].x;
                double m3 = m2 * point[i].x;
                sx2 += m2;
                sx3 += m3;
                sx4 += m3 * point[i].x;
                double mxy = point[i].x * point[i].y;
                sxy += mxy;
                sx2y += point[i].x * mxy;
            }
            double sxsx2 = sx*sx2;
            double sxsx4 = sx*sx4;
            double sx2sx2 = sx2*sx2;
            double sx2sx3 = sx2*sx3;
            double sxsx3 = sx*sx3;
            double nsx3 = (double)size_point*sx3;
            /* найдем определитель матрицы
             * n   sx  sx2
             * sx  sx2 sx3
             * sx2 sx3 sx4
             */
            double A = (double)size_point * (sx2 * sx4 - sx3 * sx3) - sx* (sx * sx4 - sx2 * sx3) + sx2 * (sx * sx3 - sx2 * sx2);
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
            coeff[1] = A * ((sx2sx3 - sxsx4) * sy + ((double)size_point*sx4 - sx2sx2) * sxy + (sxsx2 - nsx3) * sx2y);
            coeff[2] = A * ((sxsx3 - sx2sx2) * sy + (sxsx2 - nsx3) * sxy + ((double)size_point*sx2 - sx*sx) * sx2y);
        }
    }

    /** \brief Метод наименьших квадратов
     * Определение коэффициентов линейной аппроксимации по МНК
     * С помощью данной функции можно найти коэффициенты для функций
     * Y = A1*X + A0 или Y = A2*X^2 + A1*X + A0
     * \param coeff массив коэффициентов (2 либо 3 коэффициента)
     * \param array_point двумерный массив точек
     * \param array_size размер массива
     * \param type тип линии
     */
    template <typename T1, typename T2>
    void calc_least_squares_method(T2& coeff, const T1& array_point, const size_t array_size, const uint32_t &type) {
        if(type == LSM_LINE) {
            double sx = 0, sy = 0, sx2 = 0, sxy = 0;
            for(size_t i = 0; i < array_size; ++i) {
                sx += array_point[i][0];
                sy += array_point[i][1];
                sx2 += array_point[i][0] * array_point[i][0];
                sxy += array_point[i][0] * array_point[i][1];
            }
            //coeff.resize(2);
            coeff[1] = ((double)array_size * sxy - (sx * sy)) / ((double)array_size * sx2 - sx * sx);
            coeff[0] = (sy - coeff[0] * sx) / (double)array_size;
        } else
        if(type == LSM_PARABOLA) {
            double sx = 0, sy = 0, sx2 = 0, sx3 = 0, sx4 = 0, sxy = 0, sx2y = 0;
            for(size_t i = 0; i < array_size; ++i) {
                sx += array_point[i][0];
                sy += array_point[i][1];
                double m2 = array_point[i][0] * array_point[i][0];
                double m3 = m2 * array_point[i][0];
                sx2 += m2;
                sx3 += m3;
                sx4 += m3 * array_point[i][0];
                double mxy = array_point[i][0] * array_point[i][1];
                sxy += mxy;
                sx2y += array_point[i][0] * mxy;
            }
            double sxsx2 = sx*sx2;
            double sxsx4 = sx*sx4;
            double sx2sx2 = sx2*sx2;
            double sx2sx3 = sx2*sx3;
            double sxsx3 = sx*sx3;
            double nsx3 = (double)array_size*sx3;
            /* найдем определитель матрицы
             * n   sx  sx2
             * sx  sx2 sx3
             * sx2 sx3 sx4
             */
            double A = (double)array_size * (sx2 * sx4 - sx3 * sx3) - sx* (sx * sx4 - sx2 * sx3) + sx2 * (sx * sx3 - sx2 * sx2);
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
            coeff[1] = A * ((sx2sx3 - sxsx4) * sy + ((double)array_size*sx4 - sx2sx2) * sxy + (sxsx2 - nsx3) * sx2y);
            coeff[2] = A * ((sxsx3 - sx2sx2) * sy + (sxsx2 - nsx3) * sxy + ((double)array_size*sx2 - sx*sx) * sx2y);
        }
    }

    template <typename T1, typename T2>
    T2 calc_line(const T1& coeff, const T2 &x, const uint32_t &type) {
        if(type == LSM_LINE) {
            return coeff[1] * x + coeff[0];
        } else
        if(type == LSM_PARABOLA) {
            return coeff[2] * x * x + coeff[1] * x + coeff[0];
        }
        return 0;
    }
}

#endif // XTECHNICAL_REGRESSION_ANALYSIS_HPP_INCLUDED
