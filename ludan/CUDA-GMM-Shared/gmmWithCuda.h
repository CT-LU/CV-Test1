#ifndef __GMMWITHCUDA_H_
#define __GMMWITHCUDA_H_

/*
 * cpu invoke gpu kernel to initialize gmm models 
 */
extern "C"
void gpu_initialize_gmm(const unsigned char* frame);

/*
 * cpu invoke gpu kernel to perform CUDA-GMM and get output frame filtered by CUDA-GMM
 */
extern "C"
void gpu_perform_gmm(const unsigned char* frame, unsigned char* gmm_frame);

extern "C"
void gpu_free_gmm();

#endif
