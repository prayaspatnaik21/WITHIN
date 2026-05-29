#pragma once 

#include "BinaryImageFactory.h"

class ThresholdingGPU : public BinaryImageFactory
{
    public:
        cv::Mat process(cv::Mat& in) override;
};