////////////////////////////////////////////////////////////////////////////////////

#include "LinearContrastStretchingGPU.h"
#include "ColorConversionCPU.h"

////////////////////////////////////////////////////////////////////////////////////

__global__ void linearContrastStretchingKernel(const unsigned char* gpu_input , unsigned char* gpu_output ,
                                                int width ,int height ,
                                                float minVal , float maxVal)
{
    /*
    Pout = (pin - c)/(d - c)) * (b-a) + a

        a and b are the desired min and max values
        c and d are the input min and max values
    */

    int row_id = blockIdx.y * blockDim.y + threadIdx.y;
    int col_id = blockIdx.x * blockDim.x + threadIdx.x;

    if(row_id >= height || col_id >= width)
        return;
    int idx = row_id * width + col_id;

    if(maxVal > minVal)
    {
        gpu_output[idx] = static_cast<char>((static_cast<float>(gpu_input[idx] - minVal) / (maxVal - minVal)) * (255));
    }
    else
    {
        gpu_output[idx] = gpu_input[idx];
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat linearContrastStretchingGPU(const cv::Mat& in)
{
    /*
        1. allocation of memory in gpu for the input and output.
        2. kernel definition for the algorithm
        3. transfer output from gpu
    */

    if(in.empty()) return {};

    int width = in.cols;
    int height = in.rows;

    int channels = in.channels();
    cv::Mat colorConverted;
    // using cpu version of grey scale conversion
    if(channels == 3)
    {
        colorConverted = grayScaleConversion_cpu(in);
    }

    size_t inputSize = width * height * sizeof(unsigned char);
    size_t outputSize = width * height * sizeof(unsigned char);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    double minVal , maxVal;
    cv::Point minLoc , maxLoc;

    cv::minMaxLoc(colorConverted , &minVal , &maxVal , &minLoc , &maxLoc);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////

    // allocate GPU memory
    unsigned char* gpu_input , *gpu_output;

    cudaMalloc(&gpu_input , inputSize);
    cudaMalloc(&gpu_output , outputSize);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////

    // copy cpu to gpu

    cudaMemcpy(gpu_input , colorConverted.data , inputSize , cudaMemcpyHostToDevice);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Launch Kernel
    dim3 block(16 , 16);
    dim3 grid((width + block.x - 1) / block.x , (height + block.y - 1)/block.y); // write the general formula

    linearContrastStretchingKernel<<<grid , block>>>(gpu_input , gpu_output , width , height , minVal , maxVal);
    cudaDeviceSynchronize();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////

    cv::Mat out(height , width , CV_8UC1);

    // copy output
    cudaMemcpy(out.data , gpu_output , outputSize , cudaMemcpyDeviceToHost);

    // Free Memory in GPU
    cudaFree(gpu_input);
    cudaFree(gpu_output);
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////

    return out;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////