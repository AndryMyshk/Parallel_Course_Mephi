// Copyright 2021 Myshkin Andrey
#include <iostream>
#include <cmath>
#include<Windows.h>

using namespace std;

constexpr auto MIN = -2147483647;
constexpr auto MAX = 2147483647;

template<typename A>
double Function(A x) {
	return 5 * sin(4 * x) - cos(pow(x, 2)) * sin(x * exp(4));
}

template<typename A>
double FindMax(A start, A end, A step) {
	if (start > end) throw std::runtime_error("Error: Wrong Grace Segment");
	if (step <= 0 || step > (end - start)) throw std::runtime_error("Error: Incorrect step's size");

	double max = MIN;
	A curr = start;

	while (curr <= end) {
		if (max < Function(curr + step))
			max = Function(curr + step);
		curr = curr + step;
	}

	return max;
}

template<typename A>
double FindMin(A start, A end, A step) {
	if (start > end) throw std::runtime_error("Error: Wrong Grace Segment");
	if (step <= 0 || step > (end - start)) throw std::runtime_error("Error: Incorrect step's size");

	double min = MAX;
	A curr = start;
	while (curr <= end) {
		if (min > Function(curr + step))
			min = Function(curr + step);
		curr = curr + step;
	}

	return min;
}

double min = MAX;
double max = MIN;

CRITICAL_SECTION cs;

struct thread_params
{
	double start;
	double end;
	double step = 1;
	int threadNum;
};

DWORD __stdcall finding(void *args) {
	thread_params *params = (thread_params *)args;

	EnterCriticalSection(&cs);
	min = FindMin(params->start, params->end, params->step);
	max = FindMax(params->start, params->end, params->step);
	LeaveCriticalSection(&cs);

	return 0;
}

const int N = 4;
HANDLE handles[N];
DWORD thId[N];
thread_params params[N];

int main() {
	double start = 0;
	double end = 150;
	double step = 1e-3;
	double sizeLocal = (end - start) / N;

	InitializeCriticalSection(&cs);

	for (int i = 0; i < N; i++)
	{
		params[i].start = i * sizeLocal;
		params[i].end = (i + 1) * sizeLocal;
		params[i].step = step;
		params[i].threadNum = i;

		handles[i] = CreateThread(NULL, 0, finding, &params[i], CREATE_SUSPENDED, &thId[i]);
		if (!handles[i])
			printf("Error 1: %d\n", GetLastError());
		ResumeThread(handles[i]);
	}

	WaitForMultipleObjects(N, handles, true, INFINITE);
	DeleteCriticalSection(&cs);

	cout << "Function's minimum = " << min << endl;
	cout << "Function's maximum = " << max << endl;
	return 0;
}
