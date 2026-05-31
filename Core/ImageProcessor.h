////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "helper.h"
#include "IRuntime.h"
#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>

////////////////////////////////////////////////////////////////////////////////////

class ImageProcessor
{
    public:

      ImageProcessor();
      ~ImageProcessor() = default;
      void setRunTime(std::shared_ptr<IRuntime> runtime);
      void addAlgorithm(AlgoType algo);
      void removeAlgorithm(AlgoType algo);
      cv::Mat process(const cv::Mat& in);
    private:
        std::shared_ptr<IRuntime> runTimePtr;
        std::vector<AlgoType> pipeline;
};

////////////////////////////////////////////////////////////////////////////////////