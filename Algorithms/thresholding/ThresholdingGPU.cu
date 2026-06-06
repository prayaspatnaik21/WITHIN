//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ThresholdingGPU.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
__global__ void thresholdKernelSinglechannel(const unsigned char* in , unsigned char* out , int width , int height , unsigned char threshold)
{
    int row_id = blockIdx.y * blockDim.y + threadIdx.y;
    int col_id = blockIdx.x * blockDim.x + threadIdx.x;

    if(col_id >= width || row_id >= height)
        return;

    int idx = row_id * width + col_id;

    out[idx] = (in[idx] > threshold) ? 255 : 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//kernel

__global__ void thresholdKernelMultichannel(const unsigned char* in , unsigned char* out,
                                int width , int height , unsigned char threshold)
{
    
    int row_id = blockIdx.y * blockDim.y + threadIdx.y;
    int col_id = blockIdx.x * blockDim.x + threadIdx.x;

    if(col_id >= width || row_id >= height)
        return;

    int idx = row_id * width + col_id;
    int bgrIdx = idx * 3;
    
    unsigned char b = in[bgrIdx + 0];
    unsigned char g = in[bgrIdx + 1];
    unsigned char r = in[bgrIdx + 2];

    float luminance = 0.114f * b + 0.587f * g + 0.299f * r;
    out[idx] = (luminance > threshold) ? 255 : 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat threshold_gpu(cv::Mat& in)
{
    if(in.empty())
        return {};

    int width = in.cols;
    int height = in.rows;
    int channels = in.channels();

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

    ///////////////////////////////////////////////////////////////////////

    if(channels == 3)    
    {
        thresholdKernelMultichannel<<<grid , block>>>(
            gpu_input,
            gpu_output,
            width,
            height,
            threshold
        );
    }
    else
    {
        thresholdKernelSinglechannel<<<grid , block>>>(
            gpu_input,
            gpu_output,
            width,
            height,
            threshold
        );
    }


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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////