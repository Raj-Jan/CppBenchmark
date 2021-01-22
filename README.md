# CppBenchmark

Usage:
1. Copy file CppBenchmark.h to your project.
2. #include it and invoke Benchmark or BenchmarkAuto functions.
3. (optional) Invoke print method on result.

Methods uses console to print progression, but not result of benchmark.

Algorithm used:
1. Run code in loop and measure its execution time (loops are necessary for function with low execution time).
2. Measure execution of few samples and choose one with lowest execution time (this step removes big spikes in execution times, and increases accuracy by a lot)
3. Prepare "plot" where on x axis are incrising number of samples and on y axis times of execution.
4. Estimate average time of execution using linear regresion.
5. Average estimated times from few masurements.
6. Calculate standard deviation.
