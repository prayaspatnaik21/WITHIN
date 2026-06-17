////////////////////////////////////////////////////////////////////////////////////

#include "blurImageGPU.h"

////////////////////////////////////////////////////////////////////////////////////

__global__ void blur_kernel_single_channel(const unsigned char* in , unsigned char* out , int width , int height)
{
    int row_id = blockIdx.y * blockDim.y + threadIdx.y;
    int col_id = blockIdx.x * blockDim.x + threadIdx.x;

    if(row_id < height && col_id < width)
    {
        int pixel = 0;
        int count = 0;

        for(int kernel_row_id = -1 ; kernel_row_id <= 1 ; kernel_row_id++)
        {
            for(int kernel_col_id = -1 ; kernel_col_id <= 1 ; kernel_col_id++)
            {
                int curr_row_id = row_id + kernel_row_id;
                int curr_col_id = col_id + kernel_col_id;

                if(curr_row_id >= 0 && curr_row_id < height &&
                    curr_col_id >= 0 && curr_col_id < width)
                    {
                        pixel += in[curr_row_id * width + curr_col_id];
                        count++;
                    }
            }
        }

        out[row_id * width + col_id] = static_cast<unsigned char>(pixel / count);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void blur_kernel_multi_channel(const unsigned char* in , unsigned char* out , int width , int height)
{
    int row_id = blockIdx.y * blockDim.y + threadIdx.y;
    int col_id = blockIdx.x * blockDim.x + threadIdx.x;

    if(row_id < height && col_id < width)
    {
        int pixel_b = 0;
        int pixel_g = 0;
        int pixel_r = 0;

        int count = 0;

        for(int kernel_row_id = -1 ; kernel_row_id <= 1 ; kernel_row_id++)
        {
            for(int kernel_col_id = -1 ; kernel_col_id <= 1 ; kernel_col_id++)
            {
                int curr_row_id = row_id + kernel_row_id;
                int curr_col_id = col_id + kernel_col_id;

                if(curr_row_id >= 0 && curr_row_id < height &&
                    curr_col_id >= 0 && curr_col_id < width)
                    {
                        int idx = (curr_row_id * width + curr_col_id) * 3;
                        pixel_b += in[idx];
                        pixel_g += in[idx + 1];
                        pixel_r += in[idx + 2];
                        count++;
                    }
            }
        }

        int out_idx = (row_id * width + col_id) * 3;
        out[out_idx ] = static_cast<unsigned char>(pixel_b/count);
        out[out_idx + 1] = static_cast<unsigned char>(pixel_g / count);
        out[out_idx + 2] = static_cast<unsigned char>(pixel_r / count);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat blurImageGPU(const cv::Mat& in)
{
    /*
        1. send in and output to the gpu
        2. write kernel to do blurring
            1. per pixel check the ne and obtain the average
        3. Get the output from the GPU
    */

    int width = in.cols;
    int height = in.rows;

    int channels = in.channels();


    ////////////////////////////////////////////////////////////////////////////////

    size_t inputSize = width * height * channels * sizeof(unsigned char);
    size_t outputSize = width * height * channels * sizeof(unsigned char);

    unsigned char* device_input , *device_output;

    // Allocate GPU Memory
    cudaMalloc(&device_input , inputSize);
    cudaMalloc(&device_output , outputSize);

    // copy to GPU
    cudaMemcpy(device_input , in.data , inputSize , cudaMemcpyHostToDevice);

    dim3 block(16 , 16);
    dim3 grid((width + block.x - 1) / block.x,
            (height + block.y - 1) / block.y);
    
    if(channels == 1)
    {
        blur_kernel_single_channel<<<grid , block>>>(device_input , device_output , width , height);
    }
    else
    {
        blur_kernel_multi_channel<<<grid , block>>>(device_input , device_output , width , height);
    }

    cudaDeviceSynchronize();


    cv::Mat out;

    if(channels == 1)
    {
        out = cv::Mat(height , width , CV_8UC1);
    }
    else
    {
        out = cv::Mat(height , width , CV_8UC3);
    }

    cudaMemcpy(out.data , device_output , outputSize , cudaMemcpyDeviceToHost);

    cudaFree(device_input);
    cudaFree(device_output);

    return out;

}

////////////////////////////////////////////////////////////////////////////////////