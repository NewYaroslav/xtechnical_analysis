# xtechnical_analysis
C++ header-only библиотека технического анализа для алготрейдинга

### Индикаторы

* RSI с возможностью использоватьлюбые скользящие средние
* BollingerBands
* SMA
* WMA
* EMA
* MMA
* LowPassFilter
* AverageSpeed (не проверен!)
* CurrencyCorrelation (вычисление корреляции)
* DetectorWaveform (экспериментальный индикатор)
* MW (скользящее окно, которое может предоставить много данных, такие как min, max, набор std_dev, means и пр.)

### Нормализация данных

* calculate_min_max
* calculate_zscore
* calculate_difference
* normalize_amplitudes
* calculate_log

### Статистика

* calc_root_mean_square
* calc_mean_value
* calc_harmonic_mean
* calc_geometric_mean
* calc_median
* calc_std_dev_sample
* calc_std_dev_population
* calc_mean_absolute_deviation
* calc_skewness
* calc_standard_error
* calc_sampling_error
* calc_coefficient_variance
* calc_signal_to_noise_ratio
* calc_excess

### Регрессионный анализ

Пока реализован только метод наименьших квадратов

* calc_least_squares_method
* calc_line


