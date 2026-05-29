//////////////////////////////////////////////////////////////////////////////////

#include "ThresholdingGPU.h"

//////////////////////////////////////////////////////////////////////////////////

//kernel

__global__ void thresholdKernel(const unsigned char* in , unsigned char* out,
                                int width , int height , unsigned char threshold)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if(x >= width || y >= height)
        return;

    int idx = y * width + x;
    int bgrIdx = idx * 3;
    
    unsigned char b = in[bgrIdx + 0];
    unsigned char g = in[bgrIdx + 1];
    unsigned char r = in[bgrIdx + 2];

    float luminance = 0.114f * b + 0.587f * g + 0.299f * r;
    out[idx] = (luminance > threshold) ? 255 : 0;
}

//////////////////////////////////////////////////////////////////////////////////

cv::Mat ThresholdingGPU:: process(cv::Mat& in)
{
    if(in.empty())
        return {};

    int width = in.cols;
    int height = in.rows;

    int imageSize = width * height * 3;

    ///////////////////////////////////////////////////////////////////////
    // allocate GPU Memory
    unsigned char* gpu_input , *gpu_output;

    cudaMalloc(&gpu_input , imageSize);
    cudaMalloc(&gpu_output , width * height);

    ///////////////////////////////////////////////////////////////////////
    // Copy CPU -> GPU
    cudaMemcpy(gpu_input , in.data , imageSize , cudaMemcpyHostToDevice);

    ///////////////////////////////////////////////////////////////////////
    // Launch Kernel
    dim3 block(16 , 16);
    dim3 grid((width + 15) / 16 , (height + 15)/16);
    unsigned int threshold = 100;

    thresholdKernel<<<grid , block>>>(
        gpu_input,
        gpu_output,
        width,
        height,
        threshold
    );

    cudaDeviceSynchronize();

    ///////////////////////////////////////////////////////////////////////
    // Copy GPU -> CPU
    cv::Mat output(height , width , CV_8UC1);
    cudaMemcpy(output.data , gpu_output , height * width , cudaMemcpyDeviceToHost);

    ///////////////////////////////////////////////////////////////////////
    // Free GPU memory
    cudaFree(gpu_input);
    cudaFree(gpu_output);

    return output;
}