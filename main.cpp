#include "benchmark.h"

#include <Windows.h>
#include <thread>

int x;
int y;
int z;

void Test()
{
	z = x + y;
}

int main()
{
	BenchmarkAuto(&Test).PrintNS();
}