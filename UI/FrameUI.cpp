#include "FrameUI.h"
#include "FPSCounter.h"

FrameUI::FrameUI(std::shared_ptr<ImageProcessor> processor)
    : running(true),
      window(nullptr),
      textureID(0),
      currentChannels(0),
      processor(processor),
      useGPU(false),
      thresholdEnabled(false)
{
    std::cout << "Frame UI created\n";
}

FrameUI::~FrameUI()
{
    running.store(false);
}

void FrameUI::run()
{
    initWindow();
    if (!window) return;

    initImGui();
    renderLoop();
    cleanup();
}

void FrameUI::pushFrame(cv::Mat newFrame)
{
    std::lock_guard<std::mutex> lock(frameMutex);
    frame = std::move(newFrame);
}

void FrameUI::initWindow()
{
    if (!glfwInit()) return;

    window = glfwCreateWindow(1280, 720, "Camera Viewer", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
}

void FrameUI::initImGui()
{
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void FrameUI::renderLoop()
{
    cv::Mat localFrame;
    bool texReady = false;
    FPSCounter fps;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        fetchFrame(localFrame);
        updateGLTexture(localFrame, texReady);

        fps.tick();

        startImGuiFrame();
        drawMainLayout(localFrame, fps.get());
        renderImGui();
    }
}

//////////////////////////////////////////////////////////
// 🔥 MAIN LAYOUT (LEFT = FRAME, RIGHT = CONTROL)
//////////////////////////////////////////////////////////
void FrameUI::drawMainLayout(const cv::Mat& frame, float fps)
{
    ImGui::Begin("Main Window", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse);

    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImGui::GetIO().DisplaySize);

    ImGui::Columns(2, nullptr, false);

    drawFramePanel(frame, fps);
    ImGui::NextColumn();
    drawControlPanel();

    ImGui::Columns(1);
    ImGui::End();
}

//////////////////////////////////////////////////////////
// 🎥 LEFT PANEL (FRAME)
//////////////////////////////////////////////////////////
void FrameUI::drawFramePanel(const cv::Mat& frame, float fps)
{
    ImGui::Text("Camera Feed");
    ImGui::Separator();

    ImGui::Text("FPS: %.2f", fps);

    if (!frame.empty())
    {
        ImGui::Text("Resolution: %d x %d", frame.cols, frame.rows);

        ImGui::Image(
            (void*)(intptr_t)textureID,
            ImVec2(800, 600)
        );
    }
    else
    {
        ImGui::Text("Waiting for frame...");
    }
}

//////////////////////////////////////////////////////////
// 🎛️ RIGHT PANEL (CONTROL)
//////////////////////////////////////////////////////////
void FrameUI::drawControlPanel()
{
    ImGui::Text("Control Panel");
    ImGui::Separator();

    // ======================
    // GPU TOGGLE BUTTON
    // ======================
    if (drawToggleButton("GPU Runtime", useGPU))
    {
        useGPU = !useGPU;

        if (useGPU)
            processor->setRunTime(std::make_shared<GPURuntime>());
        else
            processor->setRunTime(std::make_shared<CPURuntime>());
    }

    ImGui::Spacing();

    // ======================
    // THRESHOLD BUTTON
    // ======================
    if (drawToggleButton("Threshold", thresholdEnabled))
    {
        thresholdEnabled = !thresholdEnabled;

        if (thresholdEnabled)
            processor->addAlgorithm(AlgoType::Threshold);
        else
            processor->removeAlgorithm(AlgoType::Threshold);
    }
}

//////////////////////////////////////////////////////////
// 🎨 TOGGLE BUTTON (RED / GREEN)
//////////////////////////////////////////////////////////
bool FrameUI::drawToggleButton(const char* label, bool state)
{
    ImVec4 color = state
        ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f)  // Green
        : ImVec4(0.8f, 0.2f, 0.2f, 1.0f); // Red

    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);

    bool clicked = ImGui::Button(label, ImVec2(200, 50));

    ImGui::PopStyleColor(3);

    return clicked;
}

//////////////////////////////////////////////////////////
// TEXTURE + FRAME (UNCHANGED)
//////////////////////////////////////////////////////////
void FrameUI::updateGLTexture(const cv::Mat& frame, bool& ready)
{
    if (frame.empty()) return;

    int channels = frame.channels();

    if (!ready || currentChannels != channels)
    {
        if (textureID != 0)
            glDeleteTextures(1, &textureID);

        createTexture(frame.cols, frame.rows, channels);
        currentChannels = channels;
        ready = true;
    }

    updateTexture(frame);
}

void FrameUI::updateTexture(const cv::Mat& frame)
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (frame.channels() == 1)
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                        frame.cols, frame.rows,
                        GL_RED, GL_UNSIGNED_BYTE,
                        frame.data);
    }
    else if (frame.channels() == 3)
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                        frame.cols, frame.rows,
                        GL_BGR, GL_UNSIGNED_BYTE,
                        frame.data);
    }
}

void FrameUI::createTexture(int w, int h, int channels)
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (channels == 1)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                     w, h, 0, GL_RED,
                     GL_UNSIGNED_BYTE, nullptr);

        GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    }
    else if (channels == 3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                     w, h, 0, GL_BGR,
                     GL_UNSIGNED_BYTE, nullptr);
    }
}

//////////////////////////////////////////////////////////
// FRAME FETCH
//////////////////////////////////////////////////////////
void FrameUI::fetchFrame(cv::Mat& localFrame)
{
    std::lock_guard<std::mutex> lock(frameMutex);

    if (!frame.empty())
        localFrame = frame.clone();
}

//////////////////////////////////////////////////////////
// IMGUI
//////////////////////////////////////////////////////////
void FrameUI::startImGuiFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void FrameUI::renderImGui()
{
    ImGui::Render();
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

//////////////////////////////////////////////////////////
// CLEANUP
//////////////////////////////////////////////////////////
void FrameUI::cleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (window)
        glfwDestroyWindow(window);

    glfwTerminate();
}