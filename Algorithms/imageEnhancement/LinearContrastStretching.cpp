////////////////////////////////////////////////////////////////////////////////////

#include "LinearContrastStretching.h"
#include "ColorConversion.h"

////////////////////////////////////////////////////////////////////////////////////

cv::Mat linearContrastStretching(cv::Mat& in)
{
    int rows = in.rows;
    int cols = in.cols;

    int channels = in.channels();
    
    cv::Mat out(rows , cols , CV_8UC1);

    if(channels == 3)
        in = grayScaleConversion_cpu(in);
    /*
    
        Pout = (pin - c)/(d - c)) * (b-a) + a

        a and b are the desired min and max values
        c and d are the input min and max values
    */

    double minVal ,maxVal;
    cv::Point minLoc , maxLoc;

    cv::minMaxLoc(in , &minVal , &maxVal , &minLoc , &maxLoc);

    for(int row_id = 0 ; row_id < rows ; row_id++)
    {
        for(int col_id = 0 ; col_id < cols ; col_id++)
        {
            if(maxVal > minVal)
            {
                uchar pixel = in.at<uchar>(row_id , col_id);
                out.at<uchar>(row_id , col_id) = static_cast<uchar>(((static_cast<double>(pixel) - minVal) / (maxVal - minVal)) * 255.0);
            }
        }
    }
    return out;
}

////////////////////////////////////////////////////////////////////////////////////