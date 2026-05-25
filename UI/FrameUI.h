///////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atomic>
#include <thread>
#include <mutex>

#include <opencv2/opencv.hpp>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include "ThreadSafeQueue.h"
#include "FactoryUI.h"

///////////////////////////////////////////////////////////////////////////////////////////

class FrameUI : public FactoryUI
{
    public:
        FrameUI();
        ~FrameUI();

        void run(); 
    
    private:
        void initWindow();
        void initImGui();
        void renderLoop();
        void cleanup();

        void createTexture(int width , int height);
        void updateTexture(const cv::Mat& frame);
  
        void captureLoop();

        std::thread frameThread;
        std::atomic<bool> running;

        cv::Mat frame;
        std::mutex frameMutex;

        GLFWwindow* window;
        GLuint textureID;
};

///////////////////////////////////////////////////////////////////////////////////////////