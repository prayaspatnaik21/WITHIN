//////////////////////////////////////////////////////////////////////////////////

#include "IRuntime.h"
#include "ThresholdingGPU.h"
#include "ColorConversionGPU.h"
#include "LinearContrastStretchingGPU.h"
#include "blurImageGPU.h"
#include "sharpeningGPU.h"

//////////////////////////////////////////////////////////////////////////////////

class GPURuntime : public IRuntime
{
    public:
        cv::Mat execute(AlgoType algo ,  const cv::Mat& frame) override;
        const char* name() const override { return "GPU"; }
        ~GPURuntime() = default;
};

//////////////////////////////////////////////////////////////////////////////////
