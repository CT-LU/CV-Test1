#include <cuda_runtime.h>
#include <helper_cuda.h>       // helper functions for CUDA error checking and initialization
#include <cuda.h>
#include "gmmWithCuda.h"


#define FRAME_SIZE		(FRAME_WIDTH*FRAME_HEIGHT*FRAME_CHANNELS)

//GMM parameter
#define ALPHA 0.00005
#define DEF_COVARIANCE  8.0
#define MAX_COVARIANCE  11.0
#define COVARIANCE_THRESHOLD (1.5*1.5)
#define DEF_WEIGHT 0.00005
#define MAX_GMM_COMPONENTS	3

typedef struct __align__(32)
{
	float3 pixel_mean[FRAME_WIDTH*FRAME_HEIGHT];
	float covariance[FRAME_WIDTH*FRAME_HEIGHT];
	float weight[FRAME_WIDTH*FRAME_HEIGHT];
} gaussian_model;

__device__ 
float3 operator+(const float3 &a, const float3 &b) {

	return make_float3(a.x + b.x, a.y + b.y, a.z + b.z);
}

__device__ 
float3 operator-(const float3 &a, const float3 &b) {

	return make_float3(a.x - b.x, a.y - b.y, a.z - b.z);
}

__device__ 
float3 operator*(const float3 &a, const float3 &b) {

	return make_float3(a.x*b.x, a.y*b.y, a.z*b.z);
}

__device__ 
float3 operator*(const float a, const float3 &b) {

	return make_float3(a*b.x, a*b.y, a*b.z);
}

/*
 * frame is from camera, this function is to initialize the gaussian models
 * It only be invoked once at first
 */
__global__ void
initializeGmm(uchar3* frame, gaussian_model* components)
{
	int index = blockDim.x * blockIdx.x + threadIdx.x;

	components[0].pixel_mean[index] = make_float3(frame[index].x, frame[index].y, frame[index].z);
	components[0].covariance[index] = DEF_COVARIANCE;
	components[0].weight[index] = 1.0;

#pragma unroll
	for (int i = 1; i < MAX_GMM_COMPONENTS; i++) {
		components[i].weight[index] = 0.0;
	}
}

/*
 * frame is from camera, gmm_frame is output filtered by gmm, components always stay on gpu's global memory
 * each invoking performGmm will update the components, return a new gmm_frame
 */
__global__ void
performGmm(const uchar3* frame, unsigned char* gmm_frame, gaussian_model* components)
{
	const int index = blockDim.x * blockIdx.x + threadIdx.x;
	//GMM processing parameter
	bool isMatch = false;
	float sum_of_weight = 0.0;
	float sum_of_square_diff = 0.0;
	float covariance_runtime = 0.0;
	float3 pixel_value;
	float3 pixel_mean;
	float3 pixel_diff;	

	//reset node runtime point
	gaussian_model* current_component = NULL;

	//get BGR value from each pixel

	pixel_value = make_float3(frame[index].x, frame[index].y, frame[index].z);
	
	//Macthing current pixel for GMM
#pragma unroll
	for(int k = 0; k < MAX_GMM_COMPONENTS; k++) {
		
		current_component = &components[k]; // component from 0 to max
		
		if (current_component->weight[index] == 0) {
			continue;
		}

		if(!isMatch) {
			//Handle matching for each Gaussian Component
			pixel_mean = current_component->pixel_mean[index];
			
			//get diff
			pixel_diff = pixel_value - pixel_mean;
			
			//get covariance for current gaussian model
			covariance_runtime = current_component->covariance[index];

			//get sum of square diff for BGR
			float3 tmp = pixel_diff*pixel_diff;
			sum_of_square_diff = tmp.x + tmp.y + tmp.z;
			
			//judge match or unmatch for current gaussian component
			if( sum_of_square_diff <= (COVARIANCE_THRESHOLD*covariance_runtime*covariance_runtime) )
			{
				//Match current Gaussian component
				//Update weight
				current_component->weight[index] = (1-ALPHA)*(current_component->weight[index]) + ALPHA;
				//Update Gaussian Component
				//Update mean
				pixel_mean = pixel_mean + ALPHA*pixel_diff;
				current_component->pixel_mean[index] = pixel_mean;

				//get new diff
				pixel_diff = pixel_value - pixel_mean;
				
				//update new sum of square_diff	
				float3 tmp = pixel_diff*pixel_diff;
				sum_of_square_diff = tmp.x + tmp.y + tmp.z;

				//Update covariance let Rho = ALPHA
				if (covariance_runtime < MAX_COVARIANCE) {
					covariance_runtime = covariance_runtime + ALPHA*(sum_of_square_diff - covariance_runtime);
					current_component->covariance[index] = covariance_runtime;
				}

				//Set match flag
				isMatch = true;

			}		
		}

		if (!isMatch) {
			//UnMatch current Gaussian component
			current_component->weight[index] = (1-ALPHA)*(current_component->weight[index]);
		}

		//get sum of weight
		sum_of_weight += current_component->weight[index];

	} // the end of the k components

	//if there is no match in GMM, delete the least weight gaussian component
	if(!isMatch) {
		
		current_component = &components[0]; 
		int min_component = 0;
		float min_weight = current_component->weight[index];

#pragma unroll
		for (int gg = 1; gg < MAX_GMM_COMPONENTS; gg++) {
			current_component = &components[gg];
			if (current_component->weight[index] < min_weight) {
				min_weight = current_component->weight[index];
				min_component = gg;
			}
		}					
		
		current_component = &components[min_component];
		sum_of_weight -= current_component->weight[index];
		current_component->pixel_mean[index] = pixel_value;
		current_component->covariance[index] = DEF_COVARIANCE;
		current_component->weight[index] = DEF_WEIGHT;

		//record the new sum of weight
		sum_of_weight += current_component->weight[index];
	}
	//normalize the sum of weight to 1, if sum of weight < 0.9 or sum of weight > 1.2
	//do normalization
	if(sum_of_weight < 0.9 || sum_of_weight > 1.2)
	{
		//Normalize the weight for each Gaussian component
#pragma unroll
		for (int gg = 0; gg < MAX_GMM_COMPONENTS; gg++) {
			current_component = &components[gg];
			current_component->weight[index] /= sum_of_weight;
		}
	}
	//judge foreground or background for current pixel and set the result to the gmm_frame
	if(!isMatch){
		//UnMatch any one of background GMM
		gmm_frame[index] = 255;
	} else {
		gmm_frame[index] = 0;
	}
}

/*
 * for allocating device memory
 */
uchar3* d_frame = NULL;	
unsigned char* d_gmm_frame = NULL;
gaussian_model* d_components;

#define THREADS 256 

/*
 * cpu invoke gpu kernel to initialize gmm models 
 */
extern "C"
void gpu_initialize_gmm(const unsigned char* frame)
{
	/*
	 * for kernel grid and thread num 
	 */
	int threadsPerBlock = THREADS;
	int blocksPerGrid = (FRAME_WIDTH*FRAME_HEIGHT) / threadsPerBlock;

	cudaError_t err = cudaSuccess;
	
	err = cudaMalloc((void **)&d_gmm_frame, FRAME_WIDTH*FRAME_HEIGHT);

	if (err != cudaSuccess)
	{
		fprintf(stderr, "Failed to allocate device gmm frame (error code %s)!\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	err = cudaMalloc((void **)&d_frame, FRAME_SIZE);

	if (err != cudaSuccess)
	{
		fprintf(stderr, "Failed to allocate device frame (error code %s)!\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	err = cudaMemcpy(d_frame, frame, FRAME_SIZE, cudaMemcpyHostToDevice);

	if (err != cudaSuccess)
	{
		fprintf(stderr, "Failed to copy frame from host to device while initializing (error code %s)!\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
	
	if (cudaMalloc((void **)&d_components, sizeof(gaussian_model)*MAX_GMM_COMPONENTS) != cudaSuccess)
	{
		fprintf(stderr, "Failed to allocate components (error code %s)!\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	printf("CUDA kernel launch with %d blocks of %d threads\n", blocksPerGrid, threadsPerBlock);
	initializeGmm<<<blocksPerGrid, threadsPerBlock>>>(d_frame, d_components);
	err = cudaGetLastError();

	if (err != cudaSuccess)
	{
		fprintf(stderr, "Failed to launch initializeGmm kernel (error code %s)!\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
}

/*
 * cpu invoke gpu kernel to perform CUDA-GMM and get output frame filtered by CUDA-GMM
 */
extern "C"
void gpu_perform_gmm(const unsigned char* frame, unsigned char* gmm_frame)
{
	/*
	 * for kernel grid and thread num 
	 */
	int threadsPerBlock = THREADS;
	int blocksPerGrid = (FRAME_WIDTH*FRAME_HEIGHT) / threadsPerBlock;

	cudaError_t err = cudaSuccess;
	err = cudaMemcpy(d_frame, frame, FRAME_SIZE, cudaMemcpyHostToDevice);

	if (err != cudaSuccess)
	{
		fprintf(stderr, "Failed to copy frame from host to device (error code %s)!\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
	
	performGmm<<<blocksPerGrid, threadsPerBlock>>>(d_frame, d_gmm_frame, d_components);
	err = cudaGetLastError();

	if (err != cudaSuccess)
	{
		fprintf(stderr, "Failed to launch performGmm kernel (error code %s)!\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
	
	err = cudaMemcpy(gmm_frame, d_gmm_frame, FRAME_WIDTH*FRAME_HEIGHT, cudaMemcpyDeviceToHost);

	if (err != cudaSuccess)
	{
		fprintf(stderr, "Failed to copy gmm frame from device to host (error code %s)!\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}

}

extern "C"
void gpu_free_gmm()
{
	cudaError_t err = cudaFree(d_frame);

	if (err != cudaSuccess)
	{
		fprintf(stderr, "Failed to free device vector B (error code %s)!\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
	
	err = cudaFree(d_gmm_frame);

	if (err != cudaSuccess)
	{
		fprintf(stderr, "Failed to free device vector B (error code %s)!\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	cudaDeviceReset();
}


