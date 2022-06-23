// Copyright 2021 Myshkin Andrey
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <cstring>
#include <cstdlib>


template<typename typeT>
typeT Function(typeT x) {
	return (typeT)(5 * sin(4 * (double)x) - cos(pow((double)x, 2)) * sin((double)x * exp(4)));
}

template<typename typeT>
typeT FindMax(typeT start, typeT end, typeT step) {
	if (start > end) throw std::runtime_error("Error: Wrong Grace Segment");
	if (step <= 0 || step > (end - start)) throw std::runtime_error("Error: Incorrect step's size");

	typeT max = Function(start);
	typeT x = start + step;
	while (x <= end) {
		typeT value = Function(x);
		if (max < value) max = value;
		x += step;
	}

	return max;
}

template<typename typeT>
typeT FindMin(typeT start, typeT end, typeT step) {
	if (start > end) throw std::runtime_error("Error: Wrong Grace Segment");
	if (step <= 0 || step > (end - start)) throw std::runtime_error("Error: Incorrect step's size");

	typeT min = Function(start);
	typeT x = start + step;
	while (x <= end) {
		typeT value = Function(x);
		if (min > value) min = value;
		x += step;
	}

	return min;
}

template<typename typeT>
typeT FindMinArray(typeT *buffer, int size) {
	if (buffer == nullptr) throw std::runtime_error("Error: Incorrect input");

	typeT min = Function(buffer[0]);
	for (int i = 0; i < size; i++) {
		if (min > buffer[i]) min = buffer[i];
	}

	return min;
}

template<typename typeT>
typeT FindMaxArray(typeT *buffer, int size) {
	if (buffer == nullptr) throw std::runtime_error("Error: Incorrect input");

	typeT max = Function(buffer[0]);
	for (int i = 0; i < size; i++) {
		if (max < buffer[i]) max = buffer[i];
	}

	return max;
}

template<typename typeT>
std::pair<typeT, typeT> FindMinnMax(typeT start, typeT end, typeT step) {
	std::pair<double, double> tmp;
	tmp.first = FindMin(start, end, step);
	tmp.second = FindMax(start, end, step);

	return tmp;
}

constexpr auto num_threads = 4;

using namespace boost::interprocess;

int main(int argc, char *argv[])
{
	// default: start = 0, end = 150, step = 1e-3;
	double start = 0;
	double end = 150;
	double step = 1e-3;
	double min, max;

	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (argc == 1) {
		// main process
		struct shm_remove
		{
			shm_remove() { shared_memory_object::remove("MySharedMemory"); }
			~shm_remove() { shared_memory_object::remove("MySharedMemory"); }
		} remover;

		shared_memory_object shm (create_only, "MySharedMemory", read_write);

		shm.truncate(sizeof(double) * num_threads * 2);

		mapped_region region(shm, read_write);

		std::memset(region.get_address(), -1, region.get_size());
		size_t sizeReg = region.get_size();
		printf("sizeReg = %d\n", sizeReg);
		//////////////////
		double* mem = (double*)region.get_address();
		printf("_____________________________________Main Process, pointer on shared memory = %p\n", mem);

		for (int i = 0; i < num_threads; i++) {
			double sizeLocal = (end - start) / num_threads;
			char cmd[4096];
			sprintf_s(cmd, "%s %d %lf %lf %lf", argv[0], i, (i * sizeLocal), (i + 1) * sizeLocal, step);
			printf("[%d]  cmd = \"%s\"\n", i, cmd);
			if(!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) printf("Error\n");
			// ResumeThread(pi.hProcess);
		}

		for (int i = 0; i < num_threads; i++)
			WaitForSingleObject(pi.hProcess, INFINITE);
		int* mem_int = (int*)region.get_address();
		bool fexist = false;
		while (!fexist)
		{
			fexist = true;
			for (int i = 0; i < num_threads * 4; i++) {
				if (mem_int[i] == -1) { fexist = false; break; }
			}
			Sleep(1);
		}
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		for (int i = 0; i < num_threads; i++) {
			printf("Min = %lf\n", mem[i]);
			printf("Max = %lf\n", mem[i + num_threads]);
		}

		min = FindMinArray(mem, num_threads);
		max = FindMaxArray(mem + num_threads, num_threads);

		printf("Function's Minimum = %lf\n", min);
		printf("Function's Maximum = %lf\n", max);

		printf("________________________________\n");
		printf("Success!\n");
		printf("________________________________\n");
		shm_remove();
	}
	if (argc > 1) {
		// child process
		shared_memory_object shm(open_only, "MySharedMemory", read_write);

		mapped_region region(shm, read_write);

		int num_thread = atoi(argv[1]);
		double start, end, step;
		// printf("child [%d] - \n", num_thread);
		start = atof(argv[2]);
		end = atof(argv[3]);
		step = atof(argv[4]);
		std::pair<double, double> result = FindMinnMax(start, end, step);
		printf("--------------------------------------------------------------------------------------------\n");
		printf("Start = %lf, End = %lf, Step = %lf\n", start, end, step);
		printf("Local Minimum = %lf,  Local Maximum = %lf\n", result.first, result.second);

		double* mem = (double*)region.get_address();
		printf("_____________________________________Child Process = %d, pointer on shared memory = %p\n", num_thread, mem);
		// mem[num_thread] = (double)num_thread;
		mem[num_thread] = result.first;  // 0, 1, 2, 3
		mem[num_thread + num_threads] = result.second;  // 4, 5, 6, 7
	}

	return 0;
}
