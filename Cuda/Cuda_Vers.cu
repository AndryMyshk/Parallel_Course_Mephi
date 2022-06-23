// Copyright 2021 Andrey Myshkin
// Cuda Version -----

//#ifdef __INTELLISENSE___
//// in here put whatever is your favorite flavor of intellisense workarounds
//#endif
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "thrust\reduce.h"
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

// #include <cuda.h>
#include <cuda_runtime_api.h>
#include <device_launch_parameters.h>
#include <device_functions.h>


__shared__ float sharedBufMin[2048];
__shared__ float sharedBufMax[2048];

__device__ float Function(float x) {
	return (5 * sin(4 * x) - cos(x * x) * sin(x * exp(4)));
}

//__device__ void Function_New(float x, float &value) {
//	value = (5 * sin(4 * x) - cos(x * x) * sin(x * exp(4)));
//	return;
//}

__global__ void resultFunction(float* buffer, float start, float step, int size) {
	int idx = blockIdx.x;
	int dimx = blockDim.x;
	int bias = dimx * idx + threadIdx.x;
	if (bias > size - 1) return;

	float x = start + bias * step;
	buffer[bias] = Function(x);
	return;
}

__global__ void FindMinnMax(float* buffer, float* bufOutPutMin, float* bufOutPutMax) {
	extern __shared__ float sharedBufMin[];
	extern __shared__ float sharedBufMax[];

	unsigned int tid = threadIdx.x;
	unsigned int idx = blockIdx.x;
	unsigned int dimx = blockDim.x;
	unsigned int cur = idx * dimx + threadIdx.x;

	sharedBufMin[tid] = buffer[cur];
	sharedBufMax[tid] = buffer[cur];
	__syncthreads();

	for (unsigned int i = 1; i < dimx; i *= 2) {
		int num = 2 * i * tid;
		if (num < dimx) {
			__syncthreads();
			if (threadIdx.x == 0) acquire_semaphore(&sem);
            __syncthreads();
			// Start critical section			
			if (sharedBufMin[tid] > sharedBufMin[tid + i]) sharedBufMin[tid] = sharedBufMin[tid + i];
			if (sharedBufMax[tid] < sharedBufMax[tid + i]) sharedBufMax[tid] = sharedBufMax[tid + i];
			// End critical section
			__threadfence();
			__syncthreads();
            if (threadIdx.x == 0) release_semaphore(&sem);
			__syncthreads();
		}
		__syncthreads();
	}
	if (tid == 0) {
		bufOutPutMin[idx] = sharedBufMin[0];
		bufOutPutMax[idx] = sharedBufMax[0];
	}
	return;
}


int main() {
	float start = 0;
	float end = 128;  // 2^7
	float step = 0.0625;  // 2^(-4)
	float sizeBlock = 256;
	cudaError_t status;

	int size = (end - start) / step;  // 2048
	// int sizeGrid = ceil(size / sizeBlock);  // 8

	// float min, max;
	float* min = (float*)malloc(sizeof(float) * size);
	float* max = (float*)malloc(sizeof(float) * size);

	float* bufferValuesGPU = nullptr;
	// bufferValues = (float*)malloc(sizeof(float) * size);
	status = cudaMalloc((void**)&bufferValuesGPU, sizeof(float) * size);
	if (status != cudaSuccess) printf("Error! Incorrect allocate memory \"bufferValuesGPU\"..");

	int sizeGrid = ceil(size / sizeBlock);

	resultFunction<<<sizeGrid, sizeBlock>>>(bufferValuesGPU, start, step, size);  // заполнение массива значениями в заданных точках
	cudaDeviceSynchronize();

	float* bufferValuesMin = nullptr;
	float* bufferValuesMax = nullptr;
	status = cudaMalloc((void**)&bufferValuesMin, sizeof(float) * size);
	if (status != cudaSuccess) printf("Error! Incorrect allocate memory \"bufferValuesMin\" ..");
	status = cudaMalloc((void**)&bufferValuesMax, sizeof(float) * size);
	if (status != cudaSuccess) printf("Error! Incorrect allocate memory \"bufferValuesMax\" ..");

	FindMinnMax<<<sizeGrid, sizeBlock>>>(bufferValuesGPU, bufferValuesMin, bufferValuesMax);  // нахождение минимума и максимума

	cudaEvent_t syncEvent;
	
	cudaEventCreate(&syncEvent);
	cudaEventRecord(syncEvent, 0);
	cudaEventSynchronize(syncEvent);

	status = cudaMemcpy(min, bufferValuesMin, sizeof(float) * size, cudaMemcpyDeviceToHost);
	if (status != cudaSuccess) printf("Error! Incorrect cudaMemcpy \"bufferValuesMin\" ..");
	status = cudaMemcpy(max, bufferValuesMax, sizeof(float) * size, cudaMemcpyDeviceToHost);
	if (status != cudaSuccess) printf("Error! Incorrect cudaMemcpy \"bufferValuesMax\" ..");

	float minGl = min[0];
	float maxGl = max[0];
	for (int i = 1; i < sizeGrid; i++) {
		if (minGl > min[i]) minGl = min[i];
		if (maxGl < max[i]) maxGl = max[i];		
	}

    printf("Function's Minimum = %lf", minGl);
	printf("Function's Maximum = &lf", maxGl);
	// printf("Function's Minimum = %lf", min[0]);
	// printf("Function's Maximum = &lf", max[0]);

	cudaEventDestroy(syncEvent);

	cudaFree(bufferValuesGPU);
	cudaFree(bufferValuesMin);
	cudaFree(bufferValuesMax);

	if (min) { free(min); min = nullptr; }
	if (max) { free(min); min = nullptr; }

	return 0;
}
