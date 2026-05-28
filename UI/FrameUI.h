#pragma once

//////////////////////////////////////////////////////////////////////////////////

#include <atomic>
#include <mutex>
#include <iostream>

#include <opencv2/opencv.hpp>

// OpenGL / GLFW / ImGui
#include <GLFW/glfw3.h>
// #include <GL/glew.h>   // or GLEW depending on your setup

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

//////////////////////////////////////////////////////////////////////////////////

class FrameUI
{
public:
    // Constructor / Destructor
    FrameUI();
    ~FrameUI();

    // Main entry
    void run();

    // External input (producer thread)
    void pushFrame(cv::Mat newFrame);

private:
    // ----------------------------
    // Internal State
    // ----------------------------
    std::atomic<bool> running;

    GLFWwindow* window;
    GLuint textureID;

    int currentChannels;   // 👈 track grayscale vs color

    // Frame sharing
    cv::Mat frame;
    std::mutex frameMutex;

    // ----------------------------
    // Initialization
    // ----------------------------
    void initWindow();
    void initImGui();

    // ----------------------------
    // Main Loop
    // ----------------------------
    void renderLoop();

    // ----------------------------
    // Frame Handling
    // ----------------------------
    void fetchFrame(cv::Mat& localFrame);
    void updateGLTexture(const cv::Mat& frame, bool& ready);

    // ----------------------------
    // ImGui Helpers
    // ----------------------------
    void startImGuiFrame();
    void drawUI(const cv::Mat& frame, float fps);
    void renderImGui();

    // ----------------------------
    // OpenGL Texture
    // ----------------------------
    void createTexture(int w, int h, int channels); // 👈 updated
    void updateTexture(const cv::Mat& frame);

    // ----------------------------
    // Cleanup
    // ----------------------------
    void cleanup();
};

//////////////////////////////////////////////////////////////////////////////////