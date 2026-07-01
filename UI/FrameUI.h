#pragma once

#include <atomic>
#include <mutex>
#include <iostream>
#include <memory>
#include <string>

#include <opencv2/opencv.hpp>

// OpenGL / GLFW / ImGui
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ImageProcessor.h"
#include "CPURuntime.h"
#include "GPURuntime.h"
#include "helper.h"
#include "ImageLogger.h"
#include "ProcessedFrame.h"

class FrameUI
{
public:
    FrameUI(std::shared_ptr<ImageProcessor> processor);
    ~FrameUI();

    void run();
    void pushFrame(ProcessedFrame newFrame);

private:
    std::atomic<bool> running;
    GLFWwindow* window;

    GLuint textureID;
    int currentChannels;

    ProcessedFrame frame;
    std::mutex frameMutex;

    std::shared_ptr<ImageProcessor> processor;
    ImageLogger imageLogger;

    // UI state
    bool useGPU;
    bool thresholdEnabled;
    bool grayScaleConversionEnabled;
    bool linearContrastStretchingEnabled;
    bool histogramEqualizationEnabled;
    bool blurEnabled;
    bool sharpeningEnabled;
    std::string lastSaveStatus;
    
    // Init
    void initWindow();
    void initImGui();

    // Loop
    void renderLoop();

    // Frame
    void fetchFrame(ProcessedFrame& localFrame);
    void updateGLTexture(const cv::Mat& frame, bool& ready);

    // UI
    void startImGuiFrame();
    void drawMainLayout(const ProcessedFrame& frame, float fps);
    void drawFramePanel(const cv::Mat& frame, float fps);
    void drawControlPanel();
    bool drawToggleButton(const char* label, bool state);
    void saveCurrentComparison();

    void renderImGui();

    // OpenGL
    void createTexture(int w, int h, int channels);
    void updateTexture(const cv::Mat& frame);

    // Cleanup
    void cleanup();
};
