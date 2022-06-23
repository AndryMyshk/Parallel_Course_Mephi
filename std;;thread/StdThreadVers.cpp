// Copyright 2021 Myshkin Andrey
#include <iostream>
#include <cmath> 
#include <utility>
#include <thread>
#include <vector>
#include <mutex>

constexpr auto MIN = -2147483647;
constexpr auto MAX = 2147483647;

std::mutex g_lock;

void GetNumThread() {
	std::thread::id this_id = std::this_thread::get_id();

	g_lock.lock();
	std::cout << "Thread number is "<< this_id << std::endl;
	g_lock.unlock();
}

template<typename A>
double Function(A x) {
	return 5 * sin(4 * x) - cos(pow(x, 2)) * sin(x * exp(4));
}

template<typename A>
std::pair<double, double> FindMinnMax(A start, A end, A step, double &min, double &max) {
	if (start > end) throw std::runtime_error("Error: Wrong Grace Segment");
	if (step <= 0 || step > (end - start)) throw std::runtime_error("Error: Incorrect step's size");

	std::pair<double, double> tmp;

	GetNumThread();
	min = MAX;
	max = MIN;
	A curr = start;
	while (curr <= end) {
		if (min > Function(curr + step))
			min = Function(curr + step);
		curr = curr + step;
		if (max < Function(curr + step))
			max = Function(curr + step);
		curr = curr + step;
	}

	tmp.first = min;
	tmp.second = max;

	return tmp;
}

std::pair<double, double> FindPairOfvalues(int NumThreads, std::vector<std::pair<double, double>> v_min_max) {
	if (NumThreads <= 0) throw std::runtime_error("Error: Incorrect Num of threads");
	double min, max;

	for (int i = 0; i < NumThreads - 1; i++) {
		if (v_min_max[i].first < v_min_max[i + 1].first) min = v_min_max[i].first;
		else min = v_min_max[i + 1].first;
		if (v_min_max[i].second > v_min_max[i + 1].second) max = v_min_max[i].second;
		else max = v_min_max[i + 1].second;
	}

	std::pair<double, double> mm(min, max);
	return mm;
}

int main()

{
	// default: start = 0, end = 150, step =  1e-3;
	double start = 0;
	double end = 150;
	double step = 1e-3;
	double min = MAX;
	double max = MAX;
	std::pair<double, double> mm;
	const int NumThreads = 4;  // default

	double localSize = (end - start) / NumThreads;
	std::vector<double> min_v;
	std::vector<std::pair<double, double>> v_min_max;

	std::thread threads[NumThreads];

	for (int i = 0; i < NumThreads; ++i) {
		double first = start + i * localSize;
		threads[i] = std::thread(FindMinnMax<double>, first, first + localSize, step, std::ref(min), std::ref(max));
		threads[i].join();
		std::pair<double, double> m_m(min, max);
		v_min_max.push_back(m_m);
		std::cout << "Minimum = " << v_min_max[i].first << std::endl;
		std::cout << "Maximum = " << v_min_max[i].second << std::endl;
	}

	printf("\n");
	for (int i = 0; i < NumThreads; i++) {
		printf("Vector min [%d] = %lf\n", i, v_min_max[i].first);
		printf("Vector max [%d] = %lf\n", i, v_min_max[i].second);
	}

	mm = FindPairOfvalues(NumThreads, v_min_max);

	printf("\n");
	printf("Last min = %lf\n", mm.first);
	printf("Last max = %lf\n", mm.second);
	
	return 0;
}
