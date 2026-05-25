#include "BinaryImageFactory.h"

class Thresholding : public BinaryImageFactory
{
    public:
        void process(cv::Mat& image, cv::Mat& out) override;
};