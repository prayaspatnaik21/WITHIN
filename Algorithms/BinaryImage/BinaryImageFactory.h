#pragma once 

#include <opencv2/opencv.hpp>

class BinaryImageFactory
{
    public:
        virtual cv::Mat process(cv::Mat& image) = 0;
        virtual ~BinaryImageFactory() = default;
};