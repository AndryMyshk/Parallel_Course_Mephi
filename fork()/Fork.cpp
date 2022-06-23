// Copyright 2021 Myshkin Andrey
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <random>
#include <vector>
#include <stdexcept>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <sys/mman.h>

using namespace std;


constexpr auto MIN = -2147483647;
constexpr auto MAX = 2147483647;

constexpr auto Num_Proccesses = 4;
constexpr auto Start = 0;
constexpr auto End = 150;
constexpr auto Step = 1e-3;
constexpr auto SizeLocal = (End - Start) / Step;

template<typename A>
double Function(A x) {
	return 5 * sin(4 * x) - cos(pow(x, 2)) * sin(x * exp(4));
}

template<typename A>
double FindMax(A start, A end, A step) {
	if (start > end) throw runtime_error("Error: Wrong Grace Segment");
	if (step <= 0 || step > (end - start)) throw runtime_error("Error: Incorrect step's size");

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
	if (start > end) throw runtime_error("Error: Wrong Grace Segment");
	if (step <= 0 || step > (end - start)) throw runtime_error("Error: Incorrect step's size");

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
pair<double, double> FindMinnMax(A start, A end, A step) {
	pair<double, double> tmp;
	// double max, min;

	tmp.second = FindMax(start, end, step);
	tmp.first = FindMin(start, end, step);

	// tmp.first = min;
	// tmp.second = max;

	return tmp;
}

template<typename A>
pair<double, double> FindMinnMaxVector(vector<pair<A, A>> vec) {
	pair<double, double> tmp;
	tmp.first = MAX;
	tmp.second = MIN;

	for (int i = 0; i < Num_Proccesses; i++) {
		if (tmp.first > vec[i].first) tmp.first = vec[i].first;
		if (tmp.second < vec[i].second) tmp.second = vec[i].second;
	}

	return tmp;
}

struct Params {
	pair<double, double> minMax;
	vector<pair<double, double>> mmVec;
	// vector<double> min;
	// vector<double> max;
	vector<double> start;
	double step = Step;
	bool finished[Num_Proccesses];
};


int main() {
	Params* params = static_cast<Params*>(mmap(NULL, sizeof(Params), PROT	_READ | PROT_WRITE,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0));
	
	for (int i = 0; i < Num_Proccesses; i++)
		params->start.push_back(i * SizeLocal);

	for (int i = 0; i < Num_Proccesses; i++)
		params->finished[i] = false;

	if (fork()) {
		////////////////////////////////////////////////////////////////////////////
		while (!(params->finished[0] && params->finished[1] && params->finished[2] && params->finished[3])) {};
		params->minMax = FindMinnMaxVector(params->mmVec);

		cout << " Function's Minimum = " << params->minMax.first << endl;
		cout << " Function's Maximum = " << params->minMax.second << endl;
		munmap(params, sizeof(Params));
		return 0;
	}
	else {
		if (fork() == 0) {
			if (fork() == 0) {
				params->mmVec.push_back(FindMinnMax<double>(params->start[0], params->start[0] + SizeLocal, params->step));
				params->finished[0] = true;
			}
			else {
				params->mmVec.push_back(FindMinnMax<double>(params->start[1], params->start[1] + SizeLocal, params->step));
				params->finished[1] = true;
			}
		}
		else {
			if (fork() == 0) {
				params->mmVec.push_back(FindMinnMax<double>(params->start[2], params->start[2] + SizeLocal, params->step));
				params->finished[2] = true;
			}
			else {
				params->mmVec.push_back(FindMinnMax<double>(params->start[3], params->start[3] + SizeLocal, params->step));
				params->finished[3] = true;
			}
		}
	}
}
