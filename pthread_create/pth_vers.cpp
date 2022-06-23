// Copyright 2021 Myshkin Andrey
#include <iostream>
#include <cmath> 
#include <utility>
#include <pthread.h>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

constexpr auto MIN = -2147483647;
constexpr auto MAX = 2147483647;

constexpr auto numThreads = 4;

template<typename A>
struct Params {
	A start;
	A end;
	A step;
	A min;
	A max;
};

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

void* threadFunction(void* thread_data) {
	Params<double>* params = (Params<double>*)thread_data;
	params->min = FindMin(params->start, params->end, params->step);
	params->max = FindMax(params->start, params->end, params->step);

	return NULL;
}


int main() {
	// default: start = 0, end = 150, step =  1e-3;
	double start = 0;
	double end = 150;
	double step = 1e-3;
	double min, max;
	double sizeLocal = (end - start) / numThreads;

	pthread_t* threads = (pthread_t*)malloc(numThreads * sizeof(pthread_t));

	Params<double>* params = (Params<double>*)malloc(numThreads * sizeof(Params<double>));

	for (int i = 0; i < numThreads; i++) {
		params[i].start = i * sizeLocal;
		params[i].end = (i + 1) * sizeLocal;
		params[i].step = step;
		params[i].min = MAX;
		params[i].max = MIN;

		pthread_create(&(threads[i]), NULL, threadFunction, &params[i]);
	}

	for (int i = 0; i < numThreads; i++)
		pthread_join(threads[i], NULL);

	for (int i = 0; i < numThreads; i++) {
		if (i == 0) {
			min = params[i].min;
			max = params[i].max;
		}
		if (params[i].min < min) min = params[i].min;
		if (params[i].max > max) max = params[i].max;
	}
	free(threads);
	free(params);

	std::cout << " Function's Maximum = " << min << std::endl;
	std::cout << " Function's Minimum = " << max << std::endl;

	return 0;
}

