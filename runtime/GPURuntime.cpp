/////////////////////////////////////////////////////////////////////////////////

#include "GPURuntime.h"

/////////////////////////////////////////////////////////////////////////////////

cv::Mat GPURuntime :: execute(AlgoType algo , cv::Mat& frame)
{
    switch(algo)
    {
        case AlgoType :: Threshold:
            return threshold_gpu(frame);
        case AlgoType :: GreyScaleConversion:
            return grayScaleConversion_gpu(frame);
        case AlgoType :: LinearContrastStretch:
            return linearContrastStretchingGPU(frame);
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