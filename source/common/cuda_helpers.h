#pragma once


#include <cuda_runtime.h>

#define CUDA_CHECK(x) 															   \
do																				   \
{																				   \
	cudaError_t err = x;														   \
	if (err != cudaSuccess)														   \
	{																			   \
		std::cout << "CUDA error: " << cudaGetErrorString(err) 					   \
			<< " at " << __FILE__ << ":" << __LINE__ << std::endl;				   \
	}																			   \
} while (0)