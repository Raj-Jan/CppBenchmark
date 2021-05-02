#pragma once
#include <chrono>
#include <iostream>

struct Stats
{
    double avg;
    double dev;

    void PrintNS()
    {
        Print("ns", 1e0);
    }
    void PrintUS()
    {
        Print("us", 1e-3);
    }
    void PrintMS()
    {
        Print("ms", 1e-6);
    }    
    void PrintS()
    {
        Print("s", 1e-9);
    }

private:

    void Print(const char* unit, double x)
    {
        std::cout << std::endl << "Avg: " << avg * x << " +-" << dev * x << unit << std::endl;
        std::cout << std::endl;
    }
};

template<typename T> double Run(int resolution)
{
    __int64 total = 0;
    __int64 start;
    __int64 stop;

    start = _Query_perf_counter();

    T test = T();

    for (int j = 0; j < resolution; j++)
        test();

    stop = _Query_perf_counter();

    total += stop - start;

    double freq = 1e9 / _Query_perf_frequency();

    return total * freq;
}
template<typename T> double Sample(int sampleCount, int resolution)
{
    double result = DBL_MAX;
    double x = 0;
    
    for (int i = 0; i < sampleCount; i++)
    {
        x = Run<T>(resolution);

        if (result > x)
            result = x;
    }

    return result;
}
template<typename T> double Regress(int pointCount, int sampleCount, int resolution)
{
    double _x = 0;
    double _y = 0;
    double xx = 0;
    double xy = 0;

    for (long i = 1; i <= pointCount; i++)
    {
        auto x = i;
        auto y = 0.0;

        for (int j = 0; j < i; j++)
            y += Sample<T>(sampleCount, resolution) / resolution;

        _x += x;
        _y += y;

        xx += x * x;
        xy += x * y;
    }

    return (double)(pointCount * xy - _x * _y) / (pointCount * xx - _x * _x);
}

// num - numberf of interation, pointCount - number of points in regression, sampleCount - number of samples, resolution - number of iteration in one test (this number cannot be too small)
template<typename T> Stats Benchmark(int num, int pointCount, int sampleCount, int resolution)
{
    std::cout << "Benchmark begin " << "(Resolution: " << resolution << ")" << std::endl ;

    double* records = new double[num];

    Stats result = { 0, 0 };

    for (int i = 0; i < num; i++)
    {
        int x = (int)((double)i / num * 10000);

        if (x % 1000 == 0)
        {
            std::cout << x / 100 << "%" << std::endl;
        }

        records[i] = Regress<T>(pointCount, sampleCount, resolution);

        result.avg += records[i];
    }

    result.avg /= num;

    for (int i = 0; i < num; i++)
    {
        result.dev += (result.avg - records[i]) * (result.avg - records[i]);
    }

    result.dev = std::sqrt(result.dev / num);

    std::cout << "100%" << std::endl << std::endl;

    delete[] records;

    return { result.avg, result.dev };
}

// use this method to find resolution automatically
template<typename T> int FindResolution(int sampleCount)
{
    double result = 0;

    int i = 0;

    while (result < 10000) // 10 000 gives less error; you can use any nuber, but below 1000 gives unresonable big error
    {
        i++;

        result = Sample<T>(sampleCount, i);
    }

    return i;
}

template<typename T> Stats BenchmarkAuto()
{
    int num = 20;
    int pointCount = 20;
    int sampleCount = 10;
    int resolution = FindResolution<T>(sampleCount);

    return Benchmark<T>(num, pointCount, sampleCount, resolution);
}