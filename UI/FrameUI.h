#pragma once

#include <atomic>
#include <mutex>
#include <opencv2/opencv.hpp>

#include <GLFW/glfw3.h>

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class FrameUI
{
public:
    FrameUI();
    ~FrameUI();

    void run();                        // blocking UI loop
    void pushFrame(cv::Mat frame);     // called from main

private:
    // lifecycle
    void initWindow();
    void initImGui();
    void renderLoop();
    void cleanup();

    // rendering
    void fetchFrame(cv::Mat& localFrame);
    void updateGLTexture(const cv::Mat& frame, bool& ready);

    void startImGuiFrame();
    void drawUI(const cv::Mat& frame, float fps);
    void renderImGui();

    // OpenGL
    void createTexture(int w, int h);
    void updateTexture(const cv::Mat& frame);

private:
    std::atomic<bool> running;

    GLFWwindow* window;
    GLuint textureID;

    // shared frame
    cv::Mat frame;
    std::mutex frameMutex;
};