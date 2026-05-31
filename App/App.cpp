///////////////////////////////////////////////////////////////////////////////////////////////

#include "App.h"

///////////////////////////////////////////////////////////////////////////////////////////////

App :: App()
    :buffer(std::make_unique<ThreadSafeQueue<cv::Mat>>(60)),
    imageProcessor(std::make_shared<ImageProcessor>()),
    cameraPtr(std::make_shared<Camera>(std::move(buffer))),
    frameUiPtr(std::make_shared<FrameUI>()),
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
    
    imageProcessor->addAlgorithm(AlgoType::Threshold);
    
    while(!killed)
    {
        auto frame = cameraPtr->getFrame();
        cv::Mat out = imageProcessor->process(frame);

        if(!out.empty())
        { 
            frameUiPtr->pushFrame(std::move(out));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////

void App :: run()
{
    frameUiPtr->run();
}

///////////////////////////////////////////////////////////////////////////////////////////////