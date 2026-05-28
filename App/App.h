///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <opencv2/opencv.hpp>
#include "FrameUI.h"
#include <memory>
#include "ThreadSafeQueue.h"
#include "Camera.h"
#include "ImageProcessor.h"
#include "Thresholding.h"
#include <thread>
#include <atomic> 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class App
{
    public:
        App();
        ~App();
        void run();
    private:
        std::unique_ptr<ThreadSafeQueue<cv::Mat>> buffer;
        std::shared_ptr<ImageProcessor> imageProcessor;
        std::shared_ptr<Camera> cameraPtr;
        std::shared_ptr<FrameUI> framePtr;
        std::thread imageProcessThread;
        std::atomic<bool> killed;
        void processFrames();
};