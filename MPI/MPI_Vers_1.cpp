// Copyright 2021 Myshkin Andrey
#include <mpi.h>
#include <Windows.h>
#include <vector>
#include <cstdio>
#include <malloc.h>

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
double FindMinArray(A *buffer, int size) {
	if (buffer == nullptr) throw std::runtime_error("Error: Incorrect input");

	double min = MAX;
	for (int i = 0; i < size; i++) {
		if (min > buffer[i]) min = buffer[i];
	}

	return min;
}

template<typename A>
double FindMaxArray(A *buffer, int size) {
	if (buffer == nullptr) throw std::runtime_error("Error: Incorrect input");

	double max = MIN;
	for (int i = 0; i < size; i++) {
		if (max < buffer[i]) max = buffer[i];
	}

	return max;
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


template<typename A>
std::pair<double, double> getParallelMinnMax(A start, A end, A step) {
	if (start > end) throw std::runtime_error("Error: Wrong Grace Segment");
	if (step <= 0 || step > (end - start)) throw std::runtime_error("Error: Incorrect step's size");

	int size, rank;
	std::pair<double, double> result;

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	A sizeP = (end - start);
	A delta = sizeP / size;

	double *arrayMin = (double*)malloc(sizeof(double)*size);
	double *arrayMax = (double*)malloc(sizeof(double)*size);

	std::pair<double, double> mm_local;

	if (sizeP < size) {
		if (rank == 0) {
			result = FindMinnMax(start, end, step);
			return result;
		}
		else {
			return result;
		}
	}

	if (rank == 0) {
		mm_local = FindMinnMax(start, start + delta, step);
		arrayMin[0] = mm_local.first;
		arrayMax[0] = mm_local.second;
	}
	else {
		mm_local = FindMinnMax(rank * delta, (rank + 1) * delta, step);
	}

	double min, max;
	///////////////////////////////////////////////   MIN
	if (rank != 0) {
		min = mm_local.first;
		MPI_Send(&min, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
	}
	else {
		for (int proc = 1; proc < size; proc++) {
			MPI_Status Status;
			MPI_Recv(&arrayMin[proc], 1, MPI_DOUBLE, proc, 0, MPI_COMM_WORLD, &Status);
		}
		result.first = FindMinArray(arrayMin, size);
	}
	///////////////////////////////////////////////   MAX
	if (rank != 0) {
		max = mm_local.second;
		MPI_Send(&max, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
	}
	else {
		for (int proc = 1; proc < size; proc++) {
			MPI_Status Status;
			MPI_Recv(&arrayMax[proc], 1, MPI_DOUBLE, proc, 1, MPI_COMM_WORLD, &Status);
		}
		result.second = FindMaxArray(arrayMax, size);
	}
	
	if (rank == 0) {
		if (arrayMin) { free(arrayMin); arrayMin = nullptr; }
		if (arrayMax) { free(arrayMax); arrayMax = nullptr; }
	}
	//MPI_Reduce(&mm_local.first, &result.first, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
	//MPI_Reduce(&mm_local.second, &result.second, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	return result;
}


int main(int *argc, char **argv) {
	// default: start = 0, end = 150, step = 1e-3;
	double start = 0;
	double end = 150;
	double step = 1e-3;

	std::pair<double, double> mm;

	int rank;
	MPI_Init(argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	mm = getParallelMinnMax(start, end, step);
	MPI_Finalize();

	if (rank == 0) {
		printf("Function's minimum is %lf\n", mm.first);
		printf("Function's maximum is %lf\n", mm.second);
	}

	return 0;
}
