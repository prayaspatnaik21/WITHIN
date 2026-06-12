////////////////////////////////////////////////////////////////////////////////////

#include "blurImageCPU.h"

////////////////////////////////////////////////////////////////////////////////////

cv::Mat computeBlurMultiChannel(const cv::Mat& in , int height , int width)
{
    cv::Mat out(height , width , CV_8UC3);

    for(int row_id = 0 ; row_id < height ; row_id++)
    {
        for(int col_id = 0 ; col_id < width ; col_id++)
        {
            
            int sum_b = 0;
            int sum_g = 0;
            int sum_r = 0;
            int count = 0;

            for(int row_id_kernel = -1 ; row_id_kernel <= 1 ; row_id_kernel++)
            {
                for(int col_id_kernel = -1 ; col_id_kernel <= 1 ; col_id_kernel++)
                {

                    if(row_id_kernel == 0 && col_id_kernel == 0)
                        continue;

                    if(row_id + row_id_kernel < 0 || row_id + row_id_kernel >= height ||
                        col_id + col_id_kernel < 0 || col_id + col_id_kernel >= width)
                        continue;

                    cv::Vec3b pixel = in.at<cv::Vec3b>(row_id + row_id_kernel , col_id + col_id_kernel);

                    uchar B = pixel[0];
                    uchar G = pixel[1];
                    uchar R = pixel[2];

                    sum_b += static_cast<int>(B);
                    sum_g += static_cast<int>(G);
                    sum_r += static_cast<int>(R);
                    count++; 
                }
            }
       
            out.at<cv::Vec3b>(row_id , col_id) = cv::Vec3b(static_cast<uchar>(sum_b/count) , static_cast<uchar>(sum_g / count) , static_cast<uchar>(sum_r/count));
        }
    }

    return out;
}

////////////////////////////////////////////////////////////////////////////////////

cv::Mat computeBlurSingleChannel(const cv::Mat& in , int height , int width )
{
    cv::Mat out(height , width , CV_8UC1);

    for(int row_id = 0 ; row_id < height ; row_id++)
    {
        for(int col_id = 0 ; col_id < width ; col_id++)
        {
            
            int pixel = 0;
            int count = 0;
            for(int row_id_kernel = -1 ; row_id_kernel <= 1 ; row_id_kernel++)
            {
                for(int col_id_kernel = -1 ; col_id_kernel <= 1 ; col_id_kernel++)
                {

                    if(row_id_kernel == 0 && col_id_kernel == 0)
                        continue;

                    if(row_id + row_id_kernel < 0 || row_id + row_id_kernel >= height ||
                        col_id + col_id_kernel < 0 || col_id + col_id_kernel >= width)
                        continue;

                    uchar nePixel = in.at<uchar>(row_id + row_id_kernel , col_id + col_id_kernel);

                    pixel += static_cast<int>(nePixel); 
                    count++;
                }
            }

            out.at<uchar>(row_id , col_id) = static_cast<uchar>(pixel/count);
        }
    }

    return out;
}

////////////////////////////////////////////////////////////////////////////////////

cv::Mat blurImageCPU(cv::Mat& in)
{
    ////////////////////////////////////////////////////////////////////////////////

    int height = in.rows;
    int width = in.cols;
    int channels = in.channels();

    ////////////////////////////////////////////////////////////////////////////////

    return (channels == 3) ? computeBlurMultiChannel(in , height , width) :
                             computeBlurSingleChannel(in , height , width);
}

////////////////////////////////////////////////////////////////////////////////////