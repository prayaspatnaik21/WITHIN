cmake_minimum_required(VERSION 3.18)

project(camera_understanding LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 14)

# Make sure CMake uses your nvcc
set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)

find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(cam
    main.cpp

    App/App.cpp
    src/utils.cpp

    UI/FrameUI.cpp
    Camera/Camera.cpp

    Algorithms/BinaryImage/ImageProcessor.cpp
    Algorithms/BinaryImage/Thresholding.cpp
    Algorithms/BinaryImage/ThresholdingGPU.cu

    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp

    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(cam PRIVATE
    imgui
    imgui/backends
    include
    UI
    Core
    Camera
    App
    Algorithms/BinaryImage
)

target_link_libraries(cam
    ${OpenCV_LIBS}
    glfw
    OpenGL::GL
    dl
)