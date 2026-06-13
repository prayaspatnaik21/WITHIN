/////////////////////////////////////////////////////////////////////////////////////////////////////

#include "sharpeningGPU.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void sharpenSingleChannelImageKernel(const unsigned char* device_input ,unsigned char*  device_output , int width , int height , float kSharp , int V)
{
    int row_id = blockIdx.y * blockDim.y + threadIdx.y;
    int col_id = blockIdx.x * blockDim.x + threadIdx.x;

    if(row_id < height && col_id < width)
    {
        int pixelValue = static_cast<int>(device_input[row_id * width + col_id]);

        int forward_row_id = row_id + V;
        int backward_row_id = row_id - V;

        int forward_col_id = col_id + V;
        int backward_col_id = col_id - V;

        int horizontalNeSum = 0 , verticalNeSum = 0;

        if(forward_col_id < width && backward_col_id >= 0)
            horizontalNeSum = static_cast<int>(device_input[row_id * width + forward_col_id] + device_input[row_id * width + backward_col_id]);
        
        if(forward_row_id < height && backward_row_id >= 0)
            verticalNeSum = static_cast<int>(device_input[forward_row_id * width + col_id] + device_input[backward_row_id * width + col_id]);
        
        device_output[row_id * width + col_id] = static_cast<unsigned char>((pixelValue - (0.5 * kSharp * ((verticalNeSum + horizontalNeSum)/2))) / ( 1 - kSharp));
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void sharpenMultiChannelImageKernel(const unsigned char* device_input , unsigned char* device_output , int width , int height , float kSharp , int V)
{
    int row_id = blockIdx.y * blockDim.y + threadIdx.y;
    int col_id = blockIdx.x * blockDim.x + threadIdx.x;

    if(row_id < height && col_id < width)
    {
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        int idx = (row_id * width + col_id) * 3;
        int pixelValue_b = static_cast<int>(device_input[idx]);
        int pixelValue_g = static_cast<int>(device_input[idx + 1]);
        int pixelValue_r = static_cast<int>(device_input[idx + 2]);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        int forward_row_id = row_id + V;
        int backward_row_id = row_id - V;

        int forward_col_id = col_id + V;
        int backward_col_id = col_id - V;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        int horizontalNeSum_b = 0 , verticalNeSum_b = 0;
        int horizontalNeSum_g = 0 , verticalNeSum_g = 0;
        int horizontalNeSum_r = 0 , verticalNeSum_r = 0;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        if(forward_col_id < width && backward_col_id >= 0)
        {
            int idx_forward = (row_id * width + forward_col_id) * 3;
            int idx_backward = (row_id * width + backward_col_id) * 3;

            horizontalNeSum_b = static_cast<int>(device_input[idx_forward] + device_input[idx_backward]);
            horizontalNeSum_g = static_cast<int>(device_input[idx_forward + 1] + device_input[idx_backward + 1]);
            horizontalNeSum_r = static_cast<int>(device_input[idx_forward + 2] + device_input[idx_backward + 2]);
        }

        if(forward_row_id < height && backward_row_id >= 0)
        {
            int idx_forward = (forward_row_id * width + col_id) * 3;
            int idx_backward = (backward_row_id * width + col_id) * 3;

            verticalNeSum_b = static_cast<int>(device_input[idx_forward] + device_input[idx_backward]);
            verticalNeSum_g = static_cast<int>(device_input[idx_forward + 1] + device_input[idx_backward + 1]);
            verticalNeSum_r = static_cast<int>(device_input[idx_forward + 2] + device_input[idx_backward + 2]);
        }

        idx = (row_id * width + col_id) * 3;
        device_output[idx] = static_cast<unsigned char>((pixelValue_b - (0.5 * kSharp * ((verticalNeSum_b + horizontalNeSum_b)/2))) / ( 1 - kSharp));
        device_output[idx + 1] = static_cast<unsigned char>((pixelValue_g - (0.5 * kSharp * ((verticalNeSum_g + horizontalNeSum_g)/2))) / ( 1 - kSharp));
        device_output[idx + 2] = static_cast<unsigned char>((pixelValue_r - (0.5 * kSharp * ((verticalNeSum_r + horizontalNeSum_r) / 2))) / (1 - kSharp));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
cv::Mat sharpeningGPU(cv::Mat& in)
{
    /*
        1. allocation of memory in gpu for the input and output
        2. kernel definitions for both single channel and multi channel
        3. transfer output from gpu
    */

    int height = in.rows;
    int width = in.cols;
    int channels = in.channels();

    size_t inputSize = height * width * channels * sizeof(unsigned char);
    size_t outputSize = height * width * channels * sizeof(unsigned char);

    ////////////////////////////////////////////////////////////////////////////////////////////////

    unsigned char* device_input , *device_output;
    cudaMalloc(&device_input , inputSize);
    cudaMalloc(&device_output , outputSize);

    ////////////////////////////////////////////////////////////////////////////////////////////////

    // copy memory to gpu
    cudaMemcpy(device_input , in.data , inputSize , cudaMemcpyHostToDevice);

    ////////////////////////////////////////////////////////////////////////////////////////////////

    // Launch kernel
    dim3 block(16 , 16);
    dim3 grid((width + block.x - 1) / block.x , (height + block.y - 1) / block.y);
    float kSharp = 0.5;
    int V = 2;
    if(channels == 3)
    {
        sharpenMultiChannelImageKernel<<<grid , block>>>(device_input , device_output , width , height , kSharp , V); 
    }
    else
    {
        sharpenSingleChannelImageKernel<<<grid , block>>>(device_input , device_output , width , height , kSharp , V);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    cv::Mat out;

    out = (channels == 3) ? cv::Mat(height , width , CV_8UC3) : cv::Mat(height , width , CV_8UC1);

    //////////////////////////////////////////////////////////////////////////////////////////////////

    cudaMemcpy(out.data , device_output , outputSize , cudaMemcpyDeviceToHost);

    cudaFree(device_input);
    cudaFree(device_output);

    //////////////////////////////////////////////////////////////////////////////////////////////////

    return out;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////