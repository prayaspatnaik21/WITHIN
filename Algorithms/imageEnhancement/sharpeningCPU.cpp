////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "sharpeningCPU.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat sharpeningMultiChannelImage(const cv::Mat& in , int height , int width , float ksharp ,  int v)
{
    std::vector<cv::Mat> channels;
    cv::split(in , channels);

    cv::Mat blue = channels[0];
    cv::Mat green = channels[1];
    cv::Mat red = channels[2];

    cv::Mat sharpenedBlue = sharpeningSingleChannelImage(blue , height , width , ksharp , v);
    cv::Mat sharpenedGreen = sharpeningSingleChannelImage(green , height , width , ksharp , v);
    cv::Mat sharpenedRed = sharpeningSingleChannelImage(red , height , width , ksharp , v);

    channels = {sharpenedBlue , sharpenedGreen , sharpenedRed};
    cv::Mat out;
    cv::merge(channels , out);
    return out;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat sharpeningSingleChannelImage(const cv::Mat& in , int height , int width , float ksharp , int v)
{
    // I am taking vertical and horizontal neighbors in the equation 
    // Starting I will assume few constant and check the output
    
    cv::Mat out(height , width , CV_8UC1);

    for(int row_id = 0 ; row_id < height ; row_id++)
    {
        for(int col_id = 0 ; col_id < width ; col_id++)
        {
            int pixelValue = in.at<uchar>(row_id , col_id);

            int forward_row_id = row_id + v;
            int backward_row_id = row_id - v;

            int forward_col_id = col_id + v;
            int backward_col_id = col_id - v;

            int verticalNeSum = 0;

            if(forward_row_id < height && backward_row_id >= 0)
                verticalNeSum = static_cast<int>(in.at<uchar>(forward_row_id , col_id)) + static_cast<int>(in.at<uchar>(backward_row_id , col_id));
            
            int horizontalNeSum = 0;

            if(forward_col_id < width && backward_col_id >= 0)
                horizontalNeSum = static_cast<int>(in.at<uchar>(row_id , forward_col_id)) + static_cast<int>(in.at<uchar>(row_id , backward_col_id));
            // can give weight to the ne sum , 
            
            out.at<uchar>(row_id , col_id) = static_cast<uchar>((pixelValue - (0.5 * ksharp * ((verticalNeSum + horizontalNeSum)/2))) / ( 1 - ksharp));
        }
    }
    return out;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat sharpeningCPU(cv::Mat& in)
{
    /*
        Imatest Algorithm on sharpening

        1. Lsharp(x) = (L(x) - 0.5 Ksharp(L(x-v) + L(x+v)))  / (1 - Ksharp)
            1. L(x) - input pixel level
            2. Lsharp(x) - sharpened pixel level.
            3. Ksharp - sharpening constant
            4. V - shift used for sharpening 
                1. V = Rs/dscan
                2. R - sharpening radius (the number of pixels between original image
                                        and shifted replicas) in pixels.
                3. dscan - scan rate in pixels per distance
    */

    int height = in.rows;
    int width = in.cols;

    int channels = in.channels();
    float ksharp = 0.5;
    int v = 2;
    if(channels == 3)
        return sharpeningMultiChannelImage(in , height , width ,  ksharp ,   v);
    else
        return sharpeningSingleChannelImage(in , height, width , ksharp , v);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////