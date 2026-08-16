#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "rawFrameView.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <linux/videodev2.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*


    Class V4L2 Camera Class
    =======================

    1. CameraConfiguration class - Describes what we request from the driver
    2. The requested and actual formats are kept separately because VIDIOC_S_FMT can modify a request.
    3. initialize()
        1. openDevice()
        2. QueryCapabilities()
        3. configureFormat()
        4. requestAndMapBuffers()

        5. After initialization , device is open , the format is accepted and buffer memory has been allocated by V4L2.
        6. Every buffer has been mapped into the process
        7. Streaming has not started yet.
    4.start()
        1. queueAllBuffer and VIDIOC_STREAMON
        2. The camera needs queued emoty buffers before streaming begins
    5. waitForFrame()
        1. The is the per frame acquisition function.
        2. Internally it will poll -> VIDIOC_DQBUF _. find mapped buffers and construct RawFrameView
    6. MappedBuffer records the result of mmap() function.
    7. The destructor will eventually execute - stop streaming -> unmap buffers -> close device   
    
    8. This declares the camera abstraction
        1. It own the V4L2 file descriptor
        2. The memory mappings
        3. Capture state
        4. Buffer ownership bookkeeping
        5. The negotiated camera format.
    9. Camera configuration represents what the application request.

*/
namespace within::capture
{
    struct CameraConfiguration
    {    

        std::string devicePath = "/dev/video0";

        std::uint32_t width = 4032;
        std::uint32_t height = 3040;

        std::uint32_t pixelFormat = V4L2_PIX_FMT_SRGGB10;

        std::uint32_t requestedBufferCount = 4;

        BayerPattern bayerPattern = BayerPattern::RGGB;
        RawStorage storage = RawStorage::TegraTR16;
    };

    class V4L2Camera
    {
        public:
            explicit V4L2Camera(CameraConfiguration configuration);
            ~V4L2Camera();

            // The camera own OS resources , copying it would be unsafe
            V4L2Camera(const V4L2Camera&) = delete;
            V4L2Camera& operator=(const V4L2Camera&) = delete;

            // Move disabled
            V4L2Camera(V4L2Camera&&) = delete;
            V4L2Camera& operator=(V4L2Camera&&) = delete;

            // Opens the device . check its capabilities , sets the raw format
            // requests v4l2 buffer , and map those buffers into this process

            void initialize();

            // Queues all mapped buffers and starts camera streaming
            void start();

            // Waits for a completed camera frame
            // Returns std::nullopt when the timeout expires without receiving a frame
            // when a frame is returned, the application temporarily owns its V4L2 buffer and must eventually 
            // requeueBuffer().
            std::optional<RawFrameView> waitForFrame(int timeoutMilliseconds);

            // Gives a previously dequeues buffer back to the camera driver
            // The raw frame view pointing into this buffer must not be used afterward
            void requeueBuffer(std::uint32_t bufferIndex);

            // Stops Streaming. Calling this when streaming is already stopped is safe.
            void stop() noexcept;

            bool isInitialized() const noexcept;
            bool isStreaming() const noexcept;

            std::uint32_t width() const noexcept;
            std::uint32_t height() const noexcept;
            std::uint32_t bytesPerLine() const noexcept;
            std::uint32_t imageSize() const noexcept;

        private:
            struct MappedBuffer
            {
                void* address = nullptr;
                std::size_t length = 0;
            };

            void openDevice();
            void queryCapabilities();
            void configureFormat();
            void requestAndMapBuffers();
            void queueAllBuffers();

            void unmapBuffers() noexcept;
            void closeDevice() noexcept;

            CameraConfiguration configuration_;

            int fileDescriptor_=-1;

            std::vector<MappedBuffer> mappedBuffers_;

            std::vector<std::uint8_t> bufferDequeued_;

            // These contain the actual values accepted and returned by the driver
            // They may differ from the original requested configuration.

            std::uint32_t actualWidth_ = 0;
            std::uint32_t actualHeight_ = 0;
            std::uint32_t actualPixelFormat_ = 0;
            std::uint32_t bytesPerLine_ = 0;
            std::uint32_t imageSize_ = 0;

            bool initialized_ = false;
            bool streaming_ = false;
    };
}
