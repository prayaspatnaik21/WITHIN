//////////////////////////////////////////////////////////////////////////////////

#include "FrameUI.h"
#include "FPSCounter.h"

//////////////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////////////

FrameUI::FrameUI()
    : running(true)
    , window(nullptr)
    , textureID(0)
    , currentChannels(0)
{
    std::cout << "Frame UI created\n";
}

//////////////////////////////////////////////////////////////////////////////////

FrameUI::~FrameUI()
{
    std::cout << "Shutting down...\n";
    running.store(false);
}

//////////////////////////////////////////////////////////////////////////////////
// Public API
//////////////////////////////////////////////////////////////////////////////////

void FrameUI::run()
{
    initWindow();
    if (!window) return;

    initImGui();

    std::cout << "UI started\n";

    renderLoop();
    cleanup();
}

//////////////////////////////////////////////////////////////////////////////////
// External Input
//////////////////////////////////////////////////////////////////////////////////

void FrameUI::pushFrame(cv::Mat newFrame)
{
    std::lock_guard<std::mutex> lock(frameMutex);
    frame = std::move(newFrame);
}

//////////////////////////////////////////////////////////////////////////////////
// Initialization
//////////////////////////////////////////////////////////////////////////////////

void FrameUI::initWindow()
{
    if (!glfwInit())
    {
        std::cerr << "GLFW init failed\n";
        return;
    }

    window = glfwCreateWindow(1280, 720, "Camera Viewer", NULL, NULL);

    if (!window)
    {
        std::cerr << "Window creation failed\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
}

//////////////////////////////////////////////////////////////////////////////////

void FrameUI::initImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

//////////////////////////////////////////////////////////////////////////////////
// Render Loop
//////////////////////////////////////////////////////////////////////////////////

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
        drawUI(localFrame, fps.get());
        renderImGui();
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Frame Handling
//////////////////////////////////////////////////////////////////////////////////

void FrameUI::fetchFrame(cv::Mat& localFrame)
{
    std::lock_guard<std::mutex> lock(frameMutex);

    if (!frame.empty())
        localFrame = std::move(frame);
}

//////////////////////////////////////////////////////////////////////////////////

void FrameUI::updateGLTexture(const cv::Mat& frame, bool& ready)
{
    if (frame.empty()) return;

    int channels = frame.channels();

    // Recreate texture if format changes
    if (!ready || currentChannels != channels)
    {
        if (textureID != 0)
        {
            glDeleteTextures(1, &textureID);
            textureID = 0;
        }

        createTexture(frame.cols, frame.rows, channels);
        currentChannels = channels;
        ready = true;
    }

    updateTexture(frame);
}

//////////////////////////////////////////////////////////////////////////////////
// ImGui Helpers
//////////////////////////////////////////////////////////////////////////////////

void FrameUI::startImGuiFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

//////////////////////////////////////////////////////////////////////////////////

void FrameUI::drawUI(const cv::Mat& frame, float fps)
{
    ImGui::Begin("Camera Debug");

    ImGui::Text("FPS: %.2f", fps);

    if (!frame.empty())
    {
        ImGui::Text("Resolution: %d x %d",
                    frame.cols, frame.rows);

        ImGui::Image(
            (void*)(intptr_t)textureID,
            ImVec2(640, 480)
        );
    }
    else
    {
        ImGui::Text("Waiting for frame...");
    }

    ImGui::End();
}

//////////////////////////////////////////////////////////////////////////////////

void FrameUI::renderImGui()
{
    ImGui::Render();

    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

//////////////////////////////////////////////////////////////////////////////////
// OpenGL Texture
//////////////////////////////////////////////////////////////////////////////////

void FrameUI::createTexture(int w, int h, int channels)
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (channels == 1)
    {
        // Grayscale
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                     w, h, 0, GL_RED,
                     GL_UNSIGNED_BYTE, nullptr);

        // Make grayscale display correctly
        GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    }
    else if (channels == 3)
    {
        // BGR (OpenCV default)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                     w, h, 0, GL_BGR,
                     GL_UNSIGNED_BYTE, nullptr);
    }
    else
    {
        std::cerr << "Unsupported channel count: " << channels << std::endl;
    }
}

//////////////////////////////////////////////////////////////////////////////////

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
    else
    {
        std::cerr << "Unsupported format in updateTexture\n";
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Cleanup
//////////////////////////////////////////////////////////////////////////////////

void FrameUI::cleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (textureID != 0)
    {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }

    if (window)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
}

//////////////////////////////////////////////////////////////////////////////////