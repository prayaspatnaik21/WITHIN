#include "FrameUI.h"

FrameUI :: FrameUI()
    :running(true)
    ,window(nullptr)
    ,textureID(0)
{
    frameThread = std::thread(&FrameUI::captureLoop , this);
};

//////////////////////////////////////////////////////////////////////////////////

FrameUI::~FrameUI()
{
    std::cout << "Shutting down...\n";

    running.store(false);

    if (frameThread.joinable())
        frameThread.join();

}

//////////////////////////////////////////////////////////////////////////////////

void FrameUI::captureLoop()
{
    cv::Mat temp;

    while (running)
    {
        cap.read(temp);
        if (temp.empty())
            continue;

        std::lock_guard<std::mutex> lock(frameMutex);
        frame = temp.clone();   // safe copy
    }
}

//////////////////////////////////////////////////////////////////////////////////

void FrameUI::createTexture(int w, int h)
{
    if (textureID != 0)
        return;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        w,
        h,
        0,
        GL_BGR,
        GL_UNSIGNED_BYTE,
        nullptr
    );
}

//////////////////////////////////////////////////////////////////////////////////

void FrameUI::updateTexture(const cv::Mat& frame)
{
    glBindTexture(GL_TEXTURE_2D, textureID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0, 0,
        frame.cols,
        frame.rows,
        GL_BGR,
        GL_UNSIGNED_BYTE,
        frame.data
    );
}

//////////////////////////////////////////////////////////////////////////////////

void FrameUI::run()
{
    // ----------------------------
    // GLFW
    // ----------------------------
    if (!glfwInit())
    {
        std::cerr << "GLFW init failed\n";
        return;
    }

    window = glfwCreateWindow(1280, 720, "Camera Viewer", NULL, NULL);
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1); // vsync

    // ----------------------------
    // IMGUI
    // ----------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // ----------------------------
    // LOOP
    // ----------------------------
    cv::Mat localFrame;
    bool texReady = false;

    int frameCount = 0;
    double t0 = cv::getTickCount();
    float fps = 0.0f;

    std::cout << "UI started\n";

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // -------- copy frame safely --------
        {
            std::lock_guard<std::mutex> lock(frameMutex);
            if (!frame.empty())
                localFrame = frame.clone();
        }

        // -------- texture update --------
        if (!localFrame.empty())
        {
            if (!texReady)
            {
                createTexture(localFrame.cols, localFrame.rows);
                texReady = true;
            }

            updateTexture(localFrame);
        }

        // -------- FPS --------
        frameCount++;
        double now = cv::getTickCount();
        double elapsed = (now - t0) / cv::getTickFrequency();

        if (elapsed >= 1.0)
        {
            fps = frameCount / elapsed;
            frameCount = 0;
            t0 = now;
        }

        // -------- IMGUI --------
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Camera Debug");

        ImGui::Text("FPS: %.2f", fps);

        if (!localFrame.empty())
        {
            ImGui::Text("Resolution: %d x %d", localFrame.cols, localFrame.rows);

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

        // -------- RENDER --------
        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // ----------------------------
    // CLEANUP
    // ----------------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

//////////////////////////////////////////////////////////////////////////////////