//////////////////////////////////////////////////////////////////////////////////

#include "Thresholding.h"

//////////////////////////////////////////////////////////////////////////////////
void Thresholding :: process(cv::Mat& in, cv::Mat& out)
{
    int rows = in.rows;
    int cols = in.cols;

    out.create(rows , cols , CV_8UC1);

    for(int row_id = 0 ; row_id < rows ; row_id++)
    {
        cv::Vec3b* rowPtr = in.ptr<cv::Vec3b>(row_id);

        uchar* outRow = out.ptr<uchar>(row_id);

        for(int col_id = 0 ; col_id < cols ; col_id++)
        {
            cv::Vec3b& pixel = rowPtr[col_id];

            const float blue = pixel[0];
            const float green = pixel[1];
            const float red = pixel[2];
            
            const float luminance = 0.212 * red + 0.7125 * green + 0.0722 * blue;

            outRow[col_id] = (luminance < 100.0f) ? 0 : 255;
        }
    }
}