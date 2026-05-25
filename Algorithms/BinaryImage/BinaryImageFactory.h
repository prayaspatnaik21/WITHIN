#include <opencv2/opencv.hpp>

class BinaryImageFactory
{
    public:
        virtual void process(cv::Mat& image, cv::Mat& out) = 0;
        virtual ~BinaryImageFactory() = default;
};