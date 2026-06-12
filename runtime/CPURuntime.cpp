/////////////////////////////////////////////////////////////////////////////////

#include "CPURuntime.h"

/////////////////////////////////////////////////////////////////////////////////

cv::Mat CPURuntime :: execute(AlgoType algo ,cv::Mat& frame)
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
        // case AlgoType :: Hdr:
        //     /**/
        // case AlgoType :: edge:
        //     return //;
        default:
            frame;
    }
    return frame;
}

/////////////////////////////////////////////////////////////////////////////////