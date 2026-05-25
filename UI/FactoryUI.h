class FactoryUI
{
    public:
        virtual void createTexture(int width , int height) = 0;
        virtual void updateTexture(const cv::Mat& frame) = 0;
        virtual void run() = 0;
};