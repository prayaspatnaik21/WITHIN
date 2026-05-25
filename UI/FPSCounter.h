///////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <opencv2/opencv.hpp>

///////////////////////////////////////////////////////////////////////////////////////////

class FPSCounter
{
    public:
        void tick()
        {
            double now = cv::getTickCount();

            if(lastTime != 0)
            {
                double dt = (now - lastTime) / cv::getTickFrequency();
                float currentFPS = 1.0f / dt;

                fps = 0.9f * fps + 0.1f * currentFPS;
            }

            lastTime = now;
        }

        float get() const {return fps;}

    private:
        double lastTime = 0;
        float fps = 0.0f;
};

///////////////////////////////////////////////////////////////////////////////////////////