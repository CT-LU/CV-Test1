#include <cuda_runtime.h>
#include <helper_cuda.h>
#include <cuda.h>

__constant__ float const_gaussian[64];   //gaussian array in device side

texture<short, 2, cudaReadModeElementType> image_tex;
texture<float, 2, cudaReadModeElementType> xmap_tex;
texture<float, 2, cudaReadModeElementType> ymap_tex;
texture<float, 2, cudaReadModeElementType> p1_tex;
texture<float, 2, cudaReadModeElementType> p2_tex;

short *d_image; // original image
short *d_phase;
short *d_confidence;
short *d_tmp;
float *d_xmap;
float *d_ymap;
unsigned char* d_mask;
float *d_p1;
float *d_p2;

size_t src_pitch;
size_t phase_pitch;
size_t conf_pitch;
size_t xmap_pitch;
size_t ymap_pitch;
size_t tmp_pitch;
size_t mask_pitch;
size_t p1_pitch;
size_t p2_pitch;

#define iDivUp(a, b)(a % b != 0) ? (a / b + 1) : (a / b)
//-------------------------------------------------------------------------------

/*
 * Euclidean Distance (x, y, d) = exp((|x - y|/d)^2 / 2)
 */
__device__ float euclideanDistance(float4 a, float4 b, float d)
{

    float mod = (b.x - a.x) * (b.x - a.x) +
                (b.y - a.y) * (b.y - a.y) +
                (b.z - a.z) * (b.z - a.z);

    return __expf(-mod / (2.f * d * d));
}

/*
 *
 */
#if 0
__global__ void
d_bilateralFilter(short* dst, int dst_pitch,
                   float e_d,  int radius)
{
    int x = blockIdx.x*blockDim.x + threadIdx.x;
    int y = blockIdx.y*blockDim.y + threadIdx.y;

    float sum = 0.0f;
    float factor;
    float4 t = {0.f, 0.f, 0.f, 0.f};
    float4 center_pix = tex2D(rgbaTex, x, y);

    for (int i = -radius; i <= radius; i++)
    {
        for (int j = -radius; j <= radius; j++)
        {
            float4 current_pix = tex2D(rgbaTex, x + j, y + i);
            factor = const_gaussian[i + radius]*const_gaussian[j + radius]* 
                     euclideanDistance(current_pix, center_pix, e_d);            

            t += factor*current_pix;
            sum += factor;
        }
    }

    dst[y*dst_pitch + x] = rgbaFloatToInt(t/sum);
}
#endif
/*
 *
 */
__global__
void d_convert2Distance(unsigned char* mask, int mask_pitch,
	       	short* phase, int phase_pitch,
		float* P1, int P1_pitch,
		float* P2, int P2_pitch) {

	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;
					
	float pixel = phase[y*phase_pitch + x];

	pixel = (pixel - P2[y*P2_pitch + x])*P1[y*P1_pitch + x];

	short pixel_short = (short)(pixel*0.842 + 277.8459);
	
	if(mask[y*mask_pitch + x]) {
		pixel_short = 0;
	}		

	phase[y*phase_pitch + x] = pixel_short;
}

/*
 *
 */
__global__ 
void d_remap(int width, int height,
		short* dst, int dst_pitch,
		short* src, int src_pitch)
{
        const int x = blockIdx.x*blockDim.x + threadIdx.x;
        const int y = blockIdx.y*blockDim.y + threadIdx.y;
        
	float tmp_newx = tex2D(xmap_tex, x, y);							
	float tmp_newy = tex2D(ymap_tex, x, y);							

	int newx = (int)floor(tmp_newx + 0.5);
	int newy = (int)floor(tmp_newy + 0.5);

	if(newx < width && newy < height) {
		dst[y*dst_pitch + x] = src[newy*src_pitch + newx];
	} else {
		dst[y*dst_pitch + x] = src[y*src_pitch + x];
	}
}

/*
 *
 */
__global__
void d_splitImage(int phase_pitch, int conf_pitch,
	          short* d_confidence, short* d_phase)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;                                                                     
	const int y = blockIdx.y*blockDim.y + threadIdx.y;
	
	if(blockIdx.x & 1) {
		d_phase[y*phase_pitch + x - 8 - ((blockIdx.x>>1)<<3)] = tex2D(image_tex, x, y);
	} else {
		d_confidence[y*conf_pitch + x - ((blockIdx.x>>1)<<3)] = tex2D(image_tex, x, y);
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

/*
 *
 */
__global__
void d_makeMask(short* src, int src_pitch,
		unsigned char* mask, int mask_pitch) 
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;                                                                     
	const int y = blockIdx.y*blockDim.y + threadIdx.y;
	
	short pixel = src[y*src_pitch + x];

	if(pixel == 4095 || pixel <= 1)
		mask[y*mask_pitch + x] = 1;				
	else
		mask[y*mask_pitch + x] = 0;				
}

//--------------------------------------------------------------------------------------------------

/*
 *
 */
extern "C"
void convert2Distance(int width, int height, short* phase)
{
	dim3 blockSize(8, 16);
	dim3 gridSize(iDivUp(width, blockSize.x), iDivUp(height, blockSize.y));
	d_convert2Distance<<<gridSize, blockSize>>>(d_mask, mask_pitch/sizeof(unsigned char),
	       	 					d_tmp, tmp_pitch/sizeof(short),
							d_p1, p1_pitch/sizeof(float),
							d_p2, p2_pitch/sizeof(float)); 
	// copy result back from global memory to array
	checkCudaErrors(cudaMemcpy2D(phase, sizeof(short)*width, d_tmp, tmp_pitch,
                                     sizeof(short)*width, height, cudaMemcpyDeviceToHost));
}

/*
 *
 */
extern "C"
void splitImage(int width, int height, short* confidence, short* phase)
{
	dim3 blockSize(8, 16);
	dim3 gridSize(iDivUp(width, blockSize.x), iDivUp(height, blockSize.y));
	d_splitImage<<<gridSize, blockSize>>>(phase_pitch/sizeof(short), conf_pitch/sizeof(short),
		       			      d_confidence, d_phase);

	// copy result back from global memory to array
	checkCudaErrors(cudaMemcpy2D(confidence, sizeof(short)*width/2, d_confidence, conf_pitch,
                                     sizeof(short)*width/2, height, cudaMemcpyDeviceToHost));
	checkCudaErrors(cudaMemcpy2D(phase, sizeof(short)*width/2, d_phase, phase_pitch,
                                     sizeof(short)*width/2, height, cudaMemcpyDeviceToHost));
}	

/*
 *
 */
extern "C"
void makeMask(int width, int height, unsigned char* dst)
{
	dim3 blockSize(8, 16);
	dim3 gridSize(iDivUp(width, blockSize.x), iDivUp(height, blockSize.y));
	d_makeMask<<<gridSize, blockSize>>>(d_tmp, tmp_pitch/sizeof(short),						
						d_mask, mask_pitch/sizeof(unsigned char));

	// copy result back from global memory to array
	checkCudaErrors(cudaMemcpy2D(dst, sizeof(unsigned char)*width, d_mask, mask_pitch,
                                     sizeof(unsigned char)*width, height, cudaMemcpyDeviceToHost));
}	

/*
 *
 */
extern "C"
void remapImage(int width, int height, short* dst)
{
	dim3 blockSize(8, 16);
	dim3 gridSize(iDivUp(width, blockSize.x), iDivUp(height, blockSize.y));
	d_remap<<<gridSize, blockSize>>>(width, height,
						d_tmp, tmp_pitch/sizeof(short),						
						d_phase, phase_pitch/sizeof(short));

	// copy result back from global memory to array
	checkCudaErrors(cudaMemcpy2D(dst, sizeof(short)*width, d_tmp, tmp_pitch,
                                     sizeof(short)*width, height, cudaMemcpyDeviceToHost));
}	

/*
*
*/
extern "C"
void initMapsTexture(int width, int height, float* h_xmap, float* h_ymap, float* h_p1, float* h_p2)
{
	checkCudaErrors(cudaMemcpy2D(d_xmap, xmap_pitch, h_xmap, sizeof(float)*width,
				sizeof(float)*width, height, cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy2D(d_ymap, ymap_pitch, h_ymap, sizeof(float)*width,
				sizeof(float)*width, height, cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy2D(d_p1, p1_pitch, h_p1, sizeof(float)*width,
				sizeof(float)*width, height, cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy2D(d_p2, p2_pitch, h_p2, sizeof(float)*width,
				sizeof(float)*width, height, cudaMemcpyHostToDevice));

	// Bind the array to the texture    
	cudaChannelFormatDesc desc = cudaCreateChannelDesc<float>();

	xmap_tex.addressMode[0] = cudaAddressModeClamp;
	xmap_tex.addressMode[1] = cudaAddressModeClamp;
	xmap_tex.filterMode     = cudaFilterModePoint;
	xmap_tex.normalized     = false;
	
	ymap_tex.addressMode[0] = cudaAddressModeClamp;
	ymap_tex.addressMode[1] = cudaAddressModeClamp;
	ymap_tex.filterMode     = cudaFilterModePoint;
	ymap_tex.normalized     = false;
	
	p1_tex.addressMode[0] = cudaAddressModeClamp;
	p1_tex.addressMode[1] = cudaAddressModeClamp;
	p1_tex.filterMode     = cudaFilterModePoint;
	p1_tex.normalized     = false;
	
	p2_tex.addressMode[0] = cudaAddressModeClamp;
	p2_tex.addressMode[1] = cudaAddressModeClamp;
	p2_tex.filterMode     = cudaFilterModePoint;
	p2_tex.normalized     = false;
	
	checkCudaErrors(cudaBindTexture2D(0, xmap_tex, d_xmap, desc, width, height, xmap_pitch)); 
	checkCudaErrors(cudaBindTexture2D(0, ymap_tex, d_ymap, desc, width, height, ymap_pitch)); 
	checkCudaErrors(cudaBindTexture2D(0, p1_tex, d_p1, desc, width, height, p1_pitch)); 
	checkCudaErrors(cudaBindTexture2D(0, p2_tex, d_p2, desc, width, height, p2_pitch)); 
}

/*
 *
 */
extern "C"
void initImageTexture(int width, int height, short* h_image)
{
	checkCudaErrors(cudaMemcpy2D(d_image, src_pitch, h_image, sizeof(short)*width,
				sizeof(short)*width, height, cudaMemcpyHostToDevice));

	// Bind the array to the texture    
	cudaChannelFormatDesc desc = cudaCreateChannelDesc<short>();
	image_tex.addressMode[0] = cudaAddressModeClamp;
	image_tex.addressMode[1] = cudaAddressModeClamp;
	image_tex.filterMode     = cudaFilterModePoint;
	image_tex.normalized     = false;

	checkCudaErrors(cudaBindTexture2D(0, image_tex, d_image, desc, width, height, src_pitch)); 
}

/*
 *
 */
extern "C"
void updateGaussian(float delta, int radius)
{
    float gaussian[64];

    for (int i = 0; i < 2*radius + 1; ++i)
    {
        float x = i - radius;
        gaussian[i] = expf(-(x*x) / (2*delta*delta));
    }

    checkCudaErrors(cudaMemcpyToSymbol(const_gaussian, gaussian, sizeof(float)*(2*radius+1)));
}

/*
 *
 */
extern "C"
void allocateMem(int width, int height)
{
	// copy image data to array
	checkCudaErrors(cudaMallocPitch((void **)&d_phase, &phase_pitch, (width/2)*sizeof(short), height));
	checkCudaErrors(cudaMallocPitch((void **)&d_confidence, &conf_pitch, (width/2)*sizeof(short), height));
	checkCudaErrors(cudaMallocPitch((void **)&d_image, &src_pitch, width*sizeof(short), height));
	checkCudaErrors(cudaMallocPitch((void **)&d_xmap, &xmap_pitch, (width/2)*sizeof(float), height));
	checkCudaErrors(cudaMallocPitch((void **)&d_ymap, &ymap_pitch, (width/2)*sizeof(float), height));
	checkCudaErrors(cudaMallocPitch((void **)&d_p1, &p1_pitch, (width/2)*sizeof(float), height));
	checkCudaErrors(cudaMallocPitch((void **)&d_p2, &p2_pitch, (width/2)*sizeof(float), height));
	checkCudaErrors(cudaMallocPitch((void **)&d_tmp, &tmp_pitch, (width/2)*sizeof(float), height));
	checkCudaErrors(cudaMallocPitch((void **)&d_mask, &mask_pitch, (width/2)*sizeof(unsigned char), height));
}

/*
 *
 */
extern "C"
void freeDevice()
{
	checkCudaErrors(cudaFree(d_image));
	checkCudaErrors(cudaFree(d_phase));
	checkCudaErrors(cudaFree(d_confidence));
	checkCudaErrors(cudaFree(d_xmap));
	checkCudaErrors(cudaFree(d_ymap));
	checkCudaErrors(cudaFree(d_p1));
	checkCudaErrors(cudaFree(d_p2));
	checkCudaErrors(cudaFree(d_tmp));
	checkCudaErrors(cudaFree(d_mask));
}

