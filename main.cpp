#include <opencv2/opencv.hpp>
#include "FrameUI.h"
#include <memory>

// ----------------------------
// MAIN
// ----------------------------
int main()
{
    auto framePtr = std::make_shared<FrameUI>();
    framePtr->run();
    return 0;
}