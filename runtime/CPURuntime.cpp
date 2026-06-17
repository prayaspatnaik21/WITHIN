/////////////////////////////////////////////////////////////////////////////////

#include "CPURuntime.h"

/////////////////////////////////////////////////////////////////////////////////

cv::Mat CPURuntime :: execute(AlgoType algo ,const cv::Mat& frame)
{
    switch(algo)
    {
        case AlgoType :: Threshold:
            return threshold_cpu(frame);
        case AlgoType :: GreyScaleConversion:
            return grayScaleConversion_cpu(frame);
        case AlgoType :: LinearContrastStretch:
            return linearContrastStretchingCPU(frame);
        case AlgoType :: HistogramEqualization:
            return histogramEqualizationCPU(frame);
        case AlgoType :: blur:
            return blurImageCPU(frame);
        case AlgoType :: sharpening:
            return sharpeningCPU(frame);
        default:
            cv::Mat result = std::move(frame);
            return result;
    }
    return frame;
}

/////////////////////////////////////////////////////////////////////////////////