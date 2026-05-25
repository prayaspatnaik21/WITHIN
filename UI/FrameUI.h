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

class FrameUI : public FactoryUI
{
public:
    FrameUI(ThreadSafeQueue<cv::Mat>& buffer);
    ~FrameUI();

    void run();

private:
    void initWindow();
    void initImGui();
    void renderLoop();
    void cleanup();

    void createTexture(int width , int height);

    void fetchFrame(cv::Mat& localFrame);
    void updateGLTexture(const cv::Mat& frame, bool& ready);

    void updateTexture(const cv::Mat& frame);
    void startImGuiFrame();
    void drawUI(const cv::Mat& frame, float fps);
    void renderImGui();

    void captureLoop();

private:
    std::thread frameThread;
    std::atomic<bool> running;

    cv::Mat frame;
    std::mutex frameMutex;

    GLFWwindow* window;
    GLuint textureID;

    ThreadSafeQueue<cv::Mat>& buffer;
};