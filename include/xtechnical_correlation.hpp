/*
* xtechnical_analysis - Technical analysis C++ library
*
* Copyright (c) 2018 Elektro Yar. Email: git.electroyar@gmail.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#ifndef XTECHNICAL_CORRELATION_HPP_INCLUDED
#define XTECHNICAL_CORRELATION_HPP_INCLUDED

#include "xtechnical_common.hpp"

#include <vector>
#include <algorithm>
#include <numeric>
#include <limits>
#include <cmath>

namespace xtechnical_correlation {
    using namespace xtechnical_common;

    /** \brief Коэффициент корреляции Пирсона
     * Коэффициент корреляции Пирсона характеризует существование линейной зависимости между двумя величинами
     * \param x первая выборка данных
     * \param y вторая выборка данных
     * \param rxy коэффициент корреляции Пирсона (от -1 до +1)
     * \return вернет 0 в случае успеха
     */
    template<class T1, class T2, class T3>
    int calculate_pearson_correlation_coefficient(std::vector<T1>& x, std::vector<T2> &y, T3 &rxy)
    {
        if(x.size() != y.size() || x.size() == 0) {
            return INVALID_PARAMETER;
        }
        T1 xm = std::accumulate(x.begin(), x.end(), T1(0));
        T2 ym = std::accumulate(y.begin(), y.end(), T2(0));
        xm /= (T1)x.size();
        ym /= (T2)y.size();
        T3 sum = 0, sumx2 = 0, sumy2 = 0;
        for(size_t i = 0; i < x.size(); ++i) {
            T1 dx = x[i] - xm;
            T2 dy = y[i] - ym;
            sum += dx * dy;
            sumx2 += dx * dx;
            sumy2 += dy * dy;
        }
        if(sumx2 == 0 || sumy2 == 0) {
            return INVALID_PARAMETER;
        }
        rxy = sum / std::sqrt(sumx2 * sumy2);
        return OK;
    }

    /** \brief Ранжирование для корреляции Спирмена
     * \param x вектор данных
     * \param xp ранги
     */
    template<typename T1, typename T2>
    void calculate_spearmen_ranking(std::vector<T1>& x, std::vector<T2> &xp) {
        std::vector<T1> temp = x;
        xp.resize(x.size());
        std::sort(temp.begin(), temp.end());
        temp.erase(std::unique(temp.begin(), temp.end()), temp.end());

        for(size_t i = 0; i < xp.size(); ++i) {
            auto it = std::lower_bound(temp.begin(), temp.end(), x[i]);
            xp[i] = std::distance(temp.begin(), it) + 1;
        }
    }

    /** \brief Посчитать количество повторяющихся рангов
     * \param xp вектор рангов
     * \return количество одинаковых рангов
     */
    template<typename T1>
    int calculate_repetitions_rank(std::vector<T1>& xp) {
        int num_repetitions = 0;
        //std::vector<T1> temp;
        for(size_t i = 0; i < xp.size(); ++i) {
            for(size_t j = i + 1; j < xp.size(); ++j) {
                if(xp[i] == xp[j]) {
                //auto it = std::lower_bound(temp.begin(), temp.end(), xp[i]);
                //if(it == temp.end()) {
                    num_repetitions++;
                        //temp.push_back(xp[i]);
                //}
                }
            }
        }
        return num_repetitions;
    }

    /** \brief Переформирование рангов
     * Факторам, имеющим одинаковое значение, присваивается новый ранг,
     * равный средней арифметической номеров мест, занимаемых ими в упорядоченном ряду
     * Почитать про переформирование можно например тут:
     * http://www.teasib.ru/ewels-116-3.html
     * \param xp вектор рангов, который будет переформирован
     */
    template<typename T1>
    void calculate_reshaping_ranks(std::vector<T1>& xp) {
        std::vector<T1> temp = xp;      // тут будет храниться упорядоченный ряд
        std::sort(temp.begin(), temp.end()); // создадим упорядоченный ряд
        std::vector<T1> sum_ranks(xp.size(), 0);
        std::vector<int> num_sum_ranks(xp.size(), 0);
        for(size_t i = 0; i < xp.size(); ++i) {
            auto it = std::lower_bound(temp.begin(), temp.end(), xp[i]);
            int indx = std::distance(temp.begin(), it);
            while(indx < temp.size() && temp[indx] == xp[i]) {
                sum_ranks[i] += (indx + 1);
                num_sum_ranks[i]++;
                indx++;
            }
        }
        // найдем новые ранги
        for(size_t i = 0; i < xp.size(); ++i) {
            xp[i] = sum_ranks[i] / (T1) num_sum_ranks[i];
        }
    }

    /** \brief Контрольная сумма для корреляции Спирмена
     * \param size количество выборок
     * \return контрольная сумма
     */
    template<class T1>
    T1 calculate_spearman_check_sum(const int size) {
        return (T1)(size * size * size - size) / (T1)12.0;
    }

    /** \brief Коэффициент корреляции Спирмена
     * Коэффициент корреляции Спирмена - мера линейной связи между случайными величинами.
     * Корреляция Спирмена является ранговой, то есть для оценки силы связи используются не численные значения, а соответствующие им ранги.
     * Коэффициент инвариантен по отношению к любому монотонному преобразованию шкалы измерения.
     * Ссылка на материал про коэффициент Спирмена
     * https://math.semestr.ru/corel/spirmen.php
     * \param x первая выборка данных
     * \param y вторая выборка данных
     * \param p коэффициент корреляции Спирмена (от -1 до +1)
     * \return вернет 0 в случае успеха
     */
    template<class T1, class T2, class T3>
    int calculate_spearman_rank_correlation_coefficient(std::vector<T1>& x, std::vector<T2> &y, T3 &p) {
        if(x.size() != y.size() || x.size() == 0) {
            return INVALID_PARAMETER;
        }
        // найдем ранги элементов
        std::vector<T3> rx(x.size());
        std::vector<T3> ry(y.size());
        calculate_spearmen_ranking(x, rx);
        calculate_spearmen_ranking(y, ry);

        int rep_x = calculate_repetitions_rank(rx);
        int rep_y = calculate_repetitions_rank(ry);

        T3 d1 = 0.0, d2 = 0.0;
        if(rep_x > 0) {
            d1 = calculate_spearman_check_sum<T3>(rep_x);
            T3 sum = std::accumulate(rx.begin(), rx.end(), T3(0));
            T3 calc_sum = (((T3)rx.size() + 1.0) * (T3)rx.size()) / 2.0;
            if(sum != calc_sum) {
                calculate_reshaping_ranks(rx);
            }
        }
        if(rep_y > 0) {
            d2 = calculate_spearman_check_sum<T3>(rep_y);
            T3 sum = std::accumulate(ry.begin(), ry.end(), T3(0));
            T3 calc_sum = (((T3)ry.size() + 1.0) * (T3)ry.size()) / 2.0;
            if(sum != calc_sum) {
                calculate_reshaping_ranks(ry);
            }
        }

        T3 sum = 0;
        for(size_t i = 0; i < x.size(); ++i) {
            T3 diff = rx[i] - ry[i];
            sum += diff * diff;
        }

        T3 n = x.size();
        p = 1.0 - ((6.0 *  sum + d1 + d2)/(n * n * n - n));
        return OK;
    }

    /** \brief Найти число степеней свободы
     * \param размер 1 выборки
     * \param размер 2 выборки
     * \return число степеней свободы
     */
    int calculate_number_degrees_freedom(const int n1, const int n2) {
        return n1 + n2 - 2;
    }

    /** \brief Проверить значимость коэффициента корреляции по t-критерию Стьюдента
     * \param p коэффициент ранговой корреляции Спирмена
     * \param size размер выборки
     * \return значимость коэффициента ранговой корреляции Спирмена
     */
    template<class T1, class T2>
    T1 calculate_significance_correlation_coefficient_t_criterion(const T1 p, const T2 size) {
        return p * std::sqrt(size - 2) / std::sqrt(1.0 - p * p);
    }

//      Критические точки распределения Стьюдента
    const double table_critical_t_points[] = {
//      Уровень значимости α (двусторонняя критическая область)
//      0.10            0.05            0.02            0.01            0.002           0.001
//      Уровень значимости α (односторонняя критическая область)
//      0.05 	        0.025 	        0.01 	        0.005 	        0.001 	        0.0005
//                                                                                                              Число степеней свободы
        6.31375151,	12.70620474,	31.82051595,	63.65674116,	318.30883899,	636.61924877,	        //	1
        2.91998558,	4.30265273,	6.96455673,	9.92484320,	22.32712477,	31.59905458,	        //	2
        2.35336343,	3.18244631,	4.54070286,	5.84090931,	10.21453185,	12.92397864,	        //	3
        2.13184679,	2.77644511,	3.74694739,	4.60409487,	7.17318222,	8.61030158,		//	4
        2.01504837,	2.57058184,	3.36493000,	4.03214298,	5.89342953,	6.86882663,		//	5
        1.94318028,	2.44691185,	3.14266840,	3.70742802,	5.20762624,	5.95881618,		//	6
        1.89457861,	2.36462425,	2.99795157,	3.49948330,	4.78528963,	5.40788252,		//	7
        1.85954804,	2.30600414,	2.89645945,	3.35538733,	4.50079093,	5.04130543,		//	8
        1.83311293,	2.26215716,	2.82143793,	3.24983554,	4.29680566,	4.78091259,		//	9
        1.81246112,	2.22813885,	2.76376946,	3.16927267,	4.14370049,	4.58689386,		//	10
        1.79588482,	2.20098516,	2.71807918,	3.10580652,	4.02470104,	4.43697934,		//	11
        1.78228756,	2.17881283,	2.68099799,	3.05453959,	3.92963326,	4.31779128,		//	12
        1.77093340,	2.16036866,	2.65030884,	3.01227584,	3.85198239,	4.22083173,		//	13
        1.76131014,	2.14478669,	2.62449407,	2.97684273,	3.78739024,	4.14045411,		//	14
        1.75305036,	2.13144955,	2.60248030,	2.94671288,	3.73283443,	4.07276520,		//	15
        1.74588368,	2.11990530,	2.58348719,	2.92078162,	3.68615479,	4.01499633,		//	16
        1.73960673,	2.10981558,	2.56693398,	2.89823052,	3.64576738,	3.96512627,		//	17
        1.73406361,	2.10092204,	2.55237963,	2.87844047,	3.61048488,	3.92164583,		//	18
        1.72913281,	2.09302405,	2.53948319,	2.86093461,	3.57940015,	3.88340585,		//	19
        1.72471824,	2.08596345,	2.52797700,	2.84533971,	3.55180834,	3.84951627,		//	20
        1.72074290,	2.07961384,	2.51764802,	2.83135956,	3.52715367,	3.81927716,		//	21
        1.71714437,	2.07387307,	2.50832455,	2.81875606,	3.50499203,	3.79213067,		//	22
        1.71387153,	2.06865761,	2.49986674,	2.80733568,	3.48496437,	3.76762680,		//	23
        1.71088208,	2.06389856,	2.49215947,	2.79693950,	3.46677730,	3.74539862,		//	24
        1.70814076,	2.05953855,	2.48510718,	2.78743581,	3.45018873,	3.72514395,		//	25
        1.70561792,	2.05552944,	2.47862982,	2.77871453,	3.43499718,	3.70661174,		//	26
        1.70328845,	2.05183052,	2.47265991,	2.77068296,	3.42103362,	3.68959171,		//	27
        1.70113093,	2.04840714,	2.46714010,	2.76326246,	3.40815518,	3.67390640,		//	28
        1.69912703,	2.04522964,	2.46202136,	2.75638590,	3.39624029,	3.65940502,		//	29
        1.69726089,	2.04227246,	2.45726154,	2.74999565,	3.38518487,	3.64595864,		//	30
        1.68385101,	2.02107539,	2.42325678,	2.70445927,	3.30687771,	3.55096576,		//	40
        1.67590503,	2.00855911,	2.40327192,	2.67779327,	3.26140906,	3.49601288,		//	50
        1.67064886,	2.00029782,	2.39011947,	2.66028303,	3.23170913,	3.46020047,		//	60
        1.66691448,	1.99443711,	2.38080748,	2.64790462,	3.21078906,	3.43501452,		//	70
        1.66412458,	1.99006342,	2.37386827,	2.63869060,	3.19525769,	3.41633746,		//	80
        1.66196108,	1.98667454,	2.36849748,	2.63156517,	3.18327081,	3.40193531,		//	90
        1.66023433,	1.98397152,	2.36421737,	2.62589052,	3.17373949,	3.39049131,		//	100
        1.65882419,	1.98176528,	2.36072634,	2.62126454,	3.16597937,	3.38117908,		//	110
        1.65765090,	1.97993041,	2.35782461,	2.61742115,	3.15953874,	3.37345377,		//	120
        1.65665941,	1.97838041,	2.35537458,	2.61417724,	3.15410747,	3.36694163,		//	130
        1.65581051,	1.97705372,	2.35327841,	2.61140271,	3.14946554,	3.36137771,		//	140
        1.65507550,	1.97590533,	2.35146458,	2.60900257,	3.14545253,	3.35656898,		//	150
        1.65443290,	1.97490156,	2.34987966,	2.60690582,	3.14194875,	3.35237147,		//	160
        1.65386632,	1.97401671,	2.34848289,	2.60505836,	3.13886306,	3.34867562,		//	170
        1.65336301,	1.97323082,	2.34724265,	2.60341823,	3.13612484,	3.34539656,		//	180
        1.65291295,	1.97252818,	2.34613401,	2.60195238,	3.13367853,	3.34246756,		//	190
        1.65250810,	1.97189622,	2.34513708,	2.60063444,	3.13147981,	3.33983541,		//	200
        1.65180929,	1.97080559,	2.34341702,	2.59836093,	3.12768863,	3.33529775,		//	220
        1.65122739,	1.96989764,	2.34198547,	2.59646918,	3.12453569,	3.33152484,		//	240
        1.65073534,	1.96913000,	2.34077546,	2.59487049,	3.12187233,	3.32833840,		//	260
        1.65031382,	1.96847250,	2.33973926,	2.59350165,	3.11959274,	3.32561153,		//	280
        1.64994867,	1.96790301,	2.33884192,	2.59231641,	3.11761955,	3.32325151,		//	300
        1.64867194,	1.96591234,	2.33570641,	2.58817608,	3.11073127,	3.31501522,		//	400
        1.64790685,	1.96471984,	2.33382896,	2.58569784,	3.10661162,	3.31009115,		//	500
        1.64739719,	1.96392562,	2.33257892,	2.58404815,	3.10387072,	3.30681579,		//	600
        1.64703334,	1.96335871,	2.33168682,	2.58287101,	3.10191564,	3.30447983,		//	700
        1.64676056,	1.96293374,	2.33101817,	2.58198882,	3.10045081,	3.30272983,		//	800
        1.64654846,	1.96260333,	2.33049836,	2.58130306,	3.09931237,	3.30136988,		//	900
        1.64637882,	1.96233908,	2.33008267,	2.58075470,	3.09840216,	3.30028265,		//	1000
        1.64561587,	1.96115083,	2.32821384,	2.57828979,	3.09431230,	3.29539814,		//	2000
        1.64536171,	1.96075506,	2.32759153,	2.57746913,	3.09295121,	3.29377288,		//	3000
        1.64523466,	1.96055723,	2.32728050,	2.57705899,	3.09227107,	3.29296080,		//	4000
        1.64515844,	1.96043855,	2.32709392,	2.57681297,	3.09186312,	3.29247372,		//	5000
        1.64510763,	1.96035944,	2.32696955,	2.57664897,	3.09159121,	3.29214908,		//	6000
        1.64507134,	1.96030294,	2.32688072,	2.57653185,	3.09139701,	3.29191723,		//	7000
        1.64504412,	1.96026056,	2.32681410,	2.57644401,	3.09125138,	3.29174336,		//	8000
        1.64502295,	1.96022761,	2.32676229,	2.57637570,	3.09113812,	3.29160814,		//	9000
        1.64500602,	1.96020124,	2.32672084,	2.57632105,	3.09104752,	3.29149997,		//	10000
        1.64492982,	1.96008261,	2.32653434,	2.57607515,	3.09063986,	3.29101328,		//	20000
        1.64490442,	1.96004306,	2.32647218,	2.57599320,	3.09050400,	3.29085108,		//	30000
        1.64489172,	1.96002329,	2.32644110,	2.57595222,	3.09043607,	3.29076999,		//	40000
        1.64488410,	1.96001143,	2.32642246,	2.57592764,	3.09039532,	3.29072134,		//	50000
        1.64487902,	1.96000352,	2.32641003,	2.57591125,	3.09036815,	3.29068890,		//	60000
        1.64487540,	1.95999787,	2.32640115,	2.57589954,	3.09034874,	3.29066573,		//	70000
        1.64487267,	1.95999364,	2.32639449,	2.57589076,	3.09033419,	3.29064836,		//	80000
        1.64487056,	1.95999034,	2.32638931,	2.57588393,	3.09032287,	3.29063484,		//	90000
        1.64486886,	1.95998771,	2.32638517,	2.57587847,	3.09031381,	3.29062403,		//	100000
        1.64485515,	1.95996636,	2.32635160,	2.57583422,	3.09024046,	3.29053646,		//	1000000

    };

    /// Уровени значимости для критических точек распределения Стьюдента
    enum {
        BI_CRITICAl_AREA_0P1 = 0,               ///< Уровень значимости 0.1 (двусторонняя критическая область)
        BI_CRITICAl_AREA_0P05 = 1,              ///< Уровень значимости 0.05 (двусторонняя критическая область)
        BI_CRITICAl_AREA_0P02 = 2,              ///< Уровень значимости 0.02 (двусторонняя критическая область)
        BI_CRITICAl_AREA_0P01 = 3,              ///< Уровень значимости 0.01 (двусторонняя критическая область)
        BI_CRITICAl_AREA_0P002 = 4,             ///< Уровень значимости 0.002 (двусторонняя критическая область)
        BI_CRITICAl_AREA_0P001 = 5,             ///< Уровень значимости 0.001 (двусторонняя критическая область)
        ONE_SIDED_CRITICAl_AREA_0P05 = 0,       ///< Уровень значимости 0.05 (односторонняя критическая область)
        ONE_SIDED_CRITICAl_AREA_0P025 = 1,      ///< Уровень значимости 0.025 (односторонняя критическая область)
        ONE_SIDED_CRITICAl_AREA_0P01 = 2,       ///< Уровень значимости 0.01 (односторонняя критическая область)
        ONE_SIDED_CRITICAl_AREA_0P005 = 3,      ///< Уровень значимости 0.005 (односторонняя критическая область)
        ONE_SIDED_CRITICAl_AREA_0P001 = 4,      ///< Уровень значимости 0.001 (односторонняя критическая область)
        ONE_SIDED_CRITICAl_AREA_0P0005 = 5,     ///< Уровень значимости 0.0005 (односторонняя критическая область)
    };

    /** \brief Получить t-критерий Стьюдента в зависимости от уровня значимости и числа степеней свободы
     * \param significance_level уровень значимости (см. SignificanceLevel, для 0.05 выбрать BI_CRITICAl_AREA_0P05)
     * \param degrees_freedom число степеней свободы
     * \return табличное значение для t-критерия
     */
    double get_critical_t_points(
            const int significance_level,
            const int degrees_freedom) {
        const int PART_FACTOR = 6;
        const int PART_1_SIZE = 30;
        const int PART_2_SIZE = 200;
        const int PART_2_STEP = 10;
        const int PART_3_SIZE = 300;
        const int PART_3_STEP = 20;
        const int PART_4_SIZE = 1000;
        const int PART_4_STEP = 100;
        const int PART_5_SIZE = 10000;
        const int PART_5_STEP = 1000;
        const int PART_6_SIZE = 100000;
        const int PART_6_STEP = 10000;

        if(degrees_freedom >= 1 && degrees_freedom <= PART_1_SIZE) {
            return table_critical_t_points[(degrees_freedom - 1) * PART_FACTOR + significance_level];
        } else
        if(degrees_freedom > PART_1_SIZE && degrees_freedom <= PART_2_SIZE) {
            const int TABLE_OFFSET = PART_FACTOR * (PART_1_SIZE - 1);
            return table_critical_t_points[
                TABLE_OFFSET +
                (((degrees_freedom - PART_1_SIZE - 1)/PART_2_STEP)) * PART_FACTOR +
                significance_level];
        } else
        if(degrees_freedom > PART_2_SIZE && degrees_freedom <= PART_3_SIZE) {
            const int TABLE_OFFSET =
                PART_FACTOR * (PART_1_SIZE - 1) +
                PART_FACTOR * (PART_2_SIZE - PART_1_SIZE) / PART_2_STEP;
            return table_critical_t_points[
                TABLE_OFFSET +
                (((degrees_freedom - PART_2_SIZE - 1)/PART_3_STEP)) * PART_FACTOR +
                significance_level];
        } else
        if(degrees_freedom > PART_3_SIZE && degrees_freedom <= PART_4_SIZE) {
            const int TABLE_OFFSET =
                PART_FACTOR * (PART_1_SIZE - 1) +
                PART_FACTOR * (PART_2_SIZE - PART_1_SIZE) / PART_2_STEP +
                PART_FACTOR * (PART_3_SIZE - PART_2_SIZE) / PART_3_STEP;
            return table_critical_t_points[
                TABLE_OFFSET +
                ((degrees_freedom - PART_3_SIZE - 1)/PART_4_STEP) * PART_FACTOR +
                significance_level];
        } else
        if(degrees_freedom > PART_4_SIZE && degrees_freedom <= PART_5_SIZE) {
            const int TABLE_OFFSET =
                PART_FACTOR * (PART_1_SIZE - 1) +
                PART_FACTOR * (PART_2_SIZE - PART_1_SIZE) / PART_2_STEP +
                PART_FACTOR * (PART_3_SIZE - PART_2_SIZE) / PART_3_STEP +
                PART_FACTOR * (PART_4_SIZE - PART_3_SIZE) / PART_4_STEP;
            return table_critical_t_points[
                TABLE_OFFSET +
                ((degrees_freedom - PART_4_SIZE - 1)/PART_5_STEP) * PART_FACTOR +
                significance_level];
        } else
        if(degrees_freedom > PART_5_SIZE && degrees_freedom <= PART_6_SIZE) {
            const int TABLE_OFFSET =
                PART_FACTOR * (PART_1_SIZE - 1) +
                PART_FACTOR * (PART_2_SIZE - PART_1_SIZE) / PART_2_STEP +
                PART_FACTOR * (PART_3_SIZE - PART_2_SIZE) / PART_3_STEP +
                PART_FACTOR * (PART_4_SIZE - PART_3_SIZE) / PART_4_STEP +
                PART_FACTOR * (PART_5_SIZE - PART_4_SIZE) / PART_5_STEP;
            return table_critical_t_points[
                TABLE_OFFSET +
                ((degrees_freedom - PART_5_SIZE - 1)/PART_6_STEP) * PART_FACTOR +
                significance_level];
        } else
        if(degrees_freedom > PART_6_SIZE) {
            const int TABLE_OFFSET =
                PART_FACTOR * (PART_1_SIZE - 1) +
                PART_FACTOR * (PART_2_SIZE - PART_1_SIZE) / PART_2_STEP +
                PART_FACTOR * (PART_3_SIZE - PART_2_SIZE) / PART_3_STEP +
                PART_FACTOR * (PART_4_SIZE - PART_3_SIZE) / PART_4_STEP +
                PART_FACTOR * (PART_5_SIZE - PART_4_SIZE) / PART_5_STEP +
                PART_FACTOR * (PART_6_SIZE - PART_5_SIZE) / PART_6_STEP;
            return table_critical_t_points[TABLE_OFFSET +significance_level];
        }
        return 0.0;
    }

    /** \brief Оценка коэффициента ранговой корреляции Спирмена.
     * Пример решения можно рассмотреть например тут https://math.semestr.ru/corel/spirmen.php
     * \param p коэффициент ранговой корреляции Спирмена
     * \param significance_level уровень значимости (см. SignificanceLevel, для 0.05 выбрать BI_CRITICAl_AREA_0P05)
     * \param size объем выборки
     * \return вернет true если коэффициент ранговой корреляции статистически значим
     */
    bool check_correlation_coefficient_t_criterion(
            double p,
            const int significance_level,
            const int size) {
        int degrees_freedom = size - 2;
        p = std::abs(p);
        double t_criterion = get_critical_t_points(significance_level, degrees_freedom);
        if((t_criterion * std::sqrt((1.0 - p * p) / (double)degrees_freedom)) < p) {
            return true;
        }
        return false;
    }

}

#endif // CORRELATIONEASY_HPP_INCLUDED
