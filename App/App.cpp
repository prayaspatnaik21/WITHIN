///////////////////////////////////////////////////////////////////////////////////////////////

#include "App.h"

///////////////////////////////////////////////////////////////////////////////////////////////

App :: App()
    :buffer(std::make_unique<ThreadSafeQueue<cv::Mat>>(60)),
    imageProcessor(std::make_shared<ImageProcessor>()),
    cameraPtr(std::make_shared<Camera>(std::move(buffer))),
    framePtr(std::make_shared<FrameUI>()),
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
    auto algorithmPtr = std::make_unique<Thresholding>();
    imageProcessor->setAlgorithm(std::move(algorithmPtr));
    cv::Mat out;
    while(!killed)
    {
        auto frame = cameraPtr->getFrame();
        imageProcessor->run(frame , out);
        if(!out.empty())
        {
            //std::cout << "Push Frame" << std::endl; 
            framePtr->pushFrame(std::move(out));
    
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////

void App :: run()
{
    framePtr->run();
}

///////////////////////////////////////////////////////////////////////////////////////////////