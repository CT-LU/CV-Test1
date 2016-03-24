#include <cuda_runtime.h>
#include <helper_cuda.h>
#include <cuda.h>

texture<short, 2, cudaReadModeElementType> imageTex;
short *d_image; // original image
short *d_phase;
short *d_confidence;
size_t src_pitch;
size_t phase_pitch;
size_t conf_pitch;

__global__
void d_splitImage(int phase_pitch, int conf_pitch,
	          short* d_confidence, short* d_phase)
{
	int x = blockIdx.x*blockDim.x + threadIdx.x;                                                                     
	int y = blockIdx.y*blockDim.y + threadIdx.y;
	
	if(blockIdx.x & 1) {
		d_phase[y*phase_pitch + x - 8 - ((blockIdx.x>>1)<<3)] = tex2D(imageTex, x, y);
	} else {
		d_confidence[y*conf_pitch + x - ((blockIdx.x>>1)<<3)] = tex2D(imageTex, x, y);
	}	
#if 0 // cpu simulate gpu	
	for(int bidx = 0; bidx < matImageDual.cols/8; bidx++) {
		for(int bidy = 0; bidy < matImageDual.rows/16; bidy++) {

			for(int tidx = 0; tidx < 8; tidx++) {
				for(int tidy = 0; tidy < 16; tidy++) {

					int x = bidx*8 + tidx;
					int y = bidy*16 + tidy;

					if(bidx % 2) {
						phase.at<short>(y, x - 8 - (bidx/2)*8) = matImageDual.at<short>(y, x);
					} else {
						confidence.at<short>(y, x - (bidx/2)*8) = matImageDual.at<short>(y, x);
					}	
				}
			}

		}
	}	
#endif
}

int iDivUp(int a, int b)
{
	return (a % b != 0) ? (a / b + 1) : (a / b);
}

extern "C"
void splitImage(int width, int height, short* confidence, short* phase)
{
	dim3 blockSize(8, 16);
	dim3 gridSize(iDivUp(width, blockSize.x), iDivUp(height, blockSize.y));
	
	d_splitImage<<<gridSize, blockSize>>>(phase_pitch/sizeof(short), conf_pitch/sizeof(short),
		       			      d_confidence, d_phase);

	// check if kernel execution generated an error
        getLastCudaError("Error: d_splitImage Kernel execution FAILED");

	// copy result back from global memory to array
	checkCudaErrors(cudaMemcpy2D(confidence, sizeof(short)*width/2, d_confidence, conf_pitch,
                                     sizeof(short)*width/2, height, cudaMemcpyDeviceToHost));
	checkCudaErrors(cudaMemcpy2D(phase, sizeof(short)*width/2, d_phase, phase_pitch,
                                     sizeof(short)*width/2, height, cudaMemcpyDeviceToHost));
}	

extern "C"
void initTexture(int width, int height, short* h_image)
{
	checkCudaErrors(cudaMemcpy2D(d_image, src_pitch, h_image, sizeof(short)*width,
				sizeof(short)*width, height, cudaMemcpyHostToDevice));

	// Bind the array to the texture    
	cudaChannelFormatDesc desc = cudaCreateChannelDesc<short>();
	imageTex.addressMode[0] = cudaAddressModeClamp;
	imageTex.addressMode[1] = cudaAddressModeClamp;
	imageTex.filterMode     = cudaFilterModePoint;
	imageTex.normalized     = false;

	checkCudaErrors(cudaBindTexture2D(0, imageTex, d_image, desc, width, height, src_pitch)); 
}

extern "C"
void allocateMem(int width, int height)
{
	// copy image data to array
	checkCudaErrors(cudaMallocPitch((void **)&d_phase, &phase_pitch, (width/2)*sizeof(short), height));
	checkCudaErrors(cudaMallocPitch((void **)&d_confidence, &conf_pitch, (width/2)*sizeof(short), height));
	checkCudaErrors(cudaMallocPitch((void **)&d_image, &src_pitch, sizeof(short)*width, height));
}

extern "C"
void freeDevice()
{
	checkCudaErrors(cudaFree(d_image));
	checkCudaErrors(cudaFree(d_phase));
	checkCudaErrors(cudaFree(d_confidence));
}
