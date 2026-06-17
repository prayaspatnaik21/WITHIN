////////////////////////////////////////////////////////////////

#include "ColorConversionCPU.h"

////////////////////////////////////////////////////////////////

cv::Mat grayScaleConversion_cpu(const cv::Mat& frame)
{
    int rows = frame.rows;
    int cols = frame.cols;

    cv::Mat gray(rows , cols , CV_8UC1);

    for(int row_id = 0 ; row_id < rows ; row_id++)
    {
        for(int col_id = 0 ; col_id < cols ; col_id++)
        {
            const cv::Vec3b pixel = frame.at<cv::Vec3b>(row_id , col_id);

            const uchar B = pixel[0];
            const uchar G = pixel[1];
            const uchar R = pixel[2];

            gray.at<uchar>(row_id , col_id) = static_cast<uchar>(0.114 * B + 0.587 * G + 0.299 * R);
        }
    }

    return gray;
}

////////////////////////////////////////////////////////////////