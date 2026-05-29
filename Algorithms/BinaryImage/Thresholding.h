#pragma once 

#include "BinaryImageFactory.h"

class Thresholding : public BinaryImageFactory
{
    public:
        cv::Mat process(cv::Mat& image) override;
};