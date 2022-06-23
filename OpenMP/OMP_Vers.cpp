// Copyright 2021 Myshkin Andrey
#include <omp.h>
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <ctime>
#include <random>
#include <utility>

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

template<typename A>
std::pair<double, double> FindMinnMaxVector(std::vector<std::pair<double, double>> vector, int size) {
	double min = MAX;
	double max = MIN;
	std::pair<double, double> mm;

	for (int i = 0; i < size; i++) {
		if (min > vector[i].first) min = vector[i].first;
		if (max < vector[i].second) max = vector[i].second;
	}

	mm.first = min;
	mm.second = max;
	return mm;
}

template<typename A>
std::pair<double, double> FindMinnMax(A start, A end, A step) {
	std::pair<double, double> tmp;
	double max, min;

	max = FindMax(start, end, step);
	min = FindMin(start, end, step);

	tmp.first = min;
	tmp.second = max;

	return tmp;
}


int main() {
	// default start = 0, end = 150, step = 1e-3
	double start = 0;
	double end = 150;
	double step = 1e-3;
	
	int numThreads = 4;  // default
	double sizeThr;

	std::pair<double, double> mm_local;
	std::pair<double, double> minMax;
	std::vector<std::pair<double, double>> vec;
	omp_set_num_threads(numThreads);

#pragma omp parallel shared(vec) private(mm_local) 
	{
		int tid = omp_get_thread_num();
		sizeThr = end - start / numThreads;

		mm_local = FindMinnMax(sizeThr * tid, sizeThr * (tid + 1), step);
#pragma omp barrier

#pragma omp critical
		vec.push_back(mm_local);
#pragma omp barrier

#pragma omp master
		minMax = FindMinnMaxVector<double>(vec, numThreads);
}

	std::cout << "Function's Minimum = " << minMax.first << std::endl;
	std::cout << "Function's Maximum = " << minMax.second << std::endl;
	return 0;
}
