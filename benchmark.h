#pragma once
#include <thread>
#include <chrono>
#include <iostream>
#include <Windows.h>

class Timer
{
    __int64 start;

public:
    void mark()
    {
        start = _Query_perf_counter();
    }
    double check()
    {
        const static double freq = 1e9 / _Query_perf_frequency();

        __int64 stop = stop = _Query_perf_counter();

        return (stop - start) * freq;
    }
};

class ProgressBar
{
    Timer timer;
    int _max;

public:
    char** lineStart;
    int current = 0;

    ProgressBar(int _max) : _max(_max) { }

    void run(int width, int sleep)
    {
        timer.mark();

        float progress = current / float(_max);;

        while (current < _max)
        {
            std::cout << "\r[";

            int pos = width * progress;

            for (int i = 0; i < pos; ++i)
            {
                std::cout << '=';
            }
            for (; pos < width; pos++)
            {
                std::cout << ' ';
            }

            double est = timer.check() * (1 - progress) / progress * 1e-9;

            std::cout << "] " << int(progress * 100.0) << "% EST: " << est << "s\r";
            //std::cout.flush();

            Sleep(sleep);

            progress = current / float(_max);
        }

        std::cout << "\r[== Done ==]   ";
    }
};

struct Stats
{
    double avg;
    double dev;

    void PrintNS(bool fill = true)
    {
        Print(" ns", 1e0, fill);
    }
    void PrintUS(bool fill = true)
    {
        Print(" us", 1e-3, fill);
    }
    void PrintMS(bool fill = true)
    {
        Print(" ms", 1e-6, fill);
    }    
    void PrintS(bool fill = true)
    {
        Print(" s", 1e-9, fill);
    }

    void PrintAuto(bool fill = true)
    {
        if (avg > 1e9) PrintS(fill);
        else if (avg > 1e6) PrintMS(fill);
        else if (avg > 1e3) PrintUS(fill);
        else PrintNS(fill);
    }

private:
    void Print(const char* unit, double x, bool fill)
    {
        std::cout << "Avg: " << avg * x << "  +-" << dev * x << unit << "                                                " << std::endl;
    }
};

template<typename T> double Run(int resolution)
{
    Timer t;
    T* tests = new T[resolution];

    t.mark();

    for (int j = 0; j < resolution; j++)
        tests[j]();

    double result = t.check();

    delete[] tests;

    return result;
}
template<typename T> double Sample(int sampleCount, int resolution, int& progress)
{
    double result = DBL_MAX;
    
    for (int i = 0; i < sampleCount; i++)
    {
        double x = Run<T>(resolution);

        if (result > x)
            result = x;

        progress++;
    }

    return result;
}
template<typename T> double Regress(int pointCount, int sampleCount, int resolution, int& progress)
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
            y += Sample<T>(sampleCount, resolution, progress) / resolution;

        _x += x;
        _y += y;

        xx += x * x;
        xy += x * y;
    }

    return (double)(pointCount * xy - _x * _y) / (pointCount * xx - _x * _x);
}

// use this method to find resolution automatically
template<typename T> int FindResolution(int sampleCount)
{
    double result = 0;

    int i = 1;

    const static int min_res = 2000;

    int dummy;

    while (result < min_res)
    {
        result = Sample<T>(sampleCount, i, dummy);

        if (result)
        {
            i *= 1.1 * min_res / result;

            if (i == 0)  return 1;
        }
        else
        {
            i *= 2;
        }
    }

    return i;
}

// num - numberf of interation, pointCount - number of points in regression, sampleCount - number of samples, resolution - number of iteration in one test (this number cannot be too small)
template<typename T> Stats Benchmark(int num, int pointCount, int sampleCount, int resolution)
{
    std::cout << "Benchmark begin with resolution " << resolution  << std::endl;

    Stats result = { 0, 0 };
    ProgressBar bar = ProgressBar(num * sampleCount * pointCount * (pointCount + 1) / 2);
    double* records = new double[num];

    std::thread t([&] { bar.run(50, 200); });

    for (int i = 0; i < num; i++)
    {
        records[i] = Regress<T>(pointCount, sampleCount, resolution, bar.current);

        result.avg += records[i];
    }

    t.join();

    result.avg /= num;

    for (int i = 0; i < num; i++)
    {
        result.dev += (result.avg - records[i]) * (result.avg - records[i]);
    }

    result.dev = std::sqrt(result.dev / num);

    delete[] records;

    result.PrintAuto();

    return result;
}
template<typename T> Stats Benchmark()
{
    int num = 20;
    int pointCount = 40;
    int sampleCount = 20;
    int resolution = FindResolution<T>(sampleCount);

    return Benchmark<T>(num, pointCount, sampleCount, resolution);
}