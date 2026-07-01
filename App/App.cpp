///////////////////////////////////////////////////////////////////////////////////////////////

#include "App.h"

///////////////////////////////////////////////////////////////////////////////////////////////

App :: App()
    :buffer(std::make_unique<ThreadSafeQueue<cv::Mat>>(60)),
    imageProcessor(std::make_shared<ImageProcessor>()),
    cameraPtr(std::make_shared<Camera>(std::move(buffer))),
    frameUiPtr(std::make_shared<FrameUI>(imageProcessor)),
    killed(false)
    {
        imageProcessThread = std::thread(&App::processFrames , this);
        std::cout << "App ctor" << std::endl;
    }

///////////////////////////////////////////////////////////////////////////////////////////////

App :: ~App()
{
    std::cout << "App dtor" << std::endl;
    killed.store(true);

    if(imageProcessThread.joinable())
        imageProcessThread.join();
}

///////////////////////////////////////////////////////////////////////////////////////////////

void App :: processFrames()
{
    std::cout << "Processing Frames" << std::endl;
    
    while(!killed)
    {
        auto frame = cameraPtr->getFrame();
        ProcessedFrame processedFrame = imageProcessor->processWithMetadata(frame);

        if(!processedFrame.output.empty())
        { 
            frameUiPtr->pushFrame(std::move(processedFrame));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////

void App :: run()
{
    frameUiPtr->run();
}

///////////////////////////////////////////////////////////////////////////////////////////////
