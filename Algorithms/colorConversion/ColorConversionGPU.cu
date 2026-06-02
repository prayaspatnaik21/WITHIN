//////////////////////////////////////////////////////////////////

#include "ColorConversion.h"

//////////////////////////////////////////////////////////////////

__global__ void rgb_to_gray_kernel(const unsigned char* in , unsigned char* out , int width , int height , int channels)
{
    int row_id = blockIdx.y * blockDim.y + threadIdx.y;
    int col_id = blockIdx.x * blockDim.x + threadIdx.x;

    if(row_id >= height || col_id >= width)
        return;

        int idx = (row_id * width + col_id) * channels;

        unsigned char b = in[idx];
        unsigned char g = in[idx + 1];
        unsigned char r = in[idx + 2];

        out[row_id * width + col_id] = (unsigned char)(0.114f * b + 0.587f * g + 0.299f * r);

}
//////////////////////////////////////////////////////////////////
cv::Mat grayScaleConversion_gpu(cv::Mat& in)
{
    int width = in.cols;
    int height = in.rows;

    int channels  = in.channels();

    size_t inputSize = width * height * channels * sizeof(unsigned char);
    size_t outputSize = width * height * sizeof(unsigned char);

    unsigned char* d_input , *d_output;

    // Allocate GPU Memory
    cudaMalloc(&d_input , inputSize);
    cudaMalloc(&d_output , outputSize);

    // copy to GPU
    cudaMemcpy(d_input , in.data , inputSize , cudaMemcpyHostToDevice);

    dim3 block(16 , 16);
    dim3 grid((width + block.x - 1) / block.x , 
              (height + block.y - 1) / block.y);
    // Run Kernel
    rgb_to_gray_kernel<<<grid , block>>>(
        d_input , d_output , width , height , channels
    );

    cudaDeviceSynchronize();

    cv::Mat out(height, width, CV_8UC1);

    // copy output
    cudaMemcpy(out.data , d_output , outputSize , cudaMemcpyDeviceToHost);

    // Free GPU
    cudaFree(d_input);
    cudaFree(d_output);

    return out;
}

//////////////////////////////////////////////////////////////////