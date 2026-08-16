#pragma once

#include <cstddef>
#include <cstdint>

namespace within::capture
{
    enum class BayerPattern
    {
        RGGB,
        BGGR,
        GRBG,
        GBRG
    };

    enum class RawStorage
    {
        TegraTR16
    };
    
    /*
        1. Design

            1. Description of memory that contain one captured Frame.
            2. External memory format must be decoded explicitly , that's why imageData is a byte pointer.
        
        2. Buffer Cycle
            1. Application queues empty buffer.
            2. Camera fills the buffer
            3. Buffer becomes ready
            4. Application Dequeues Buffer.
            5. Application reads/processes image
            6. Application requeues buffer
            7. Camera Fills it again.
    */

    struct RawFrameView
    {
        /*
            1. Points to the beginning of the memory mapped V4L2 camera buffer.
            2. Does not own the memroy . The pointer remains valid only until the corresponding V4L2 buffer is returned to the driver.
        */

        const std::uint8_t* imageData = nullptr;

        // Number of valid bytes written by the driver to the frame
        std::size_t bytesUsed = 0;

        // Image Dimensions in pixels
        std::uint32_t width = 0;
        std::uint32_t height = 0;

        // Distance in bytes from the beginning of one row to the next
        // Do not assume this is always width * 2. Some drivers add padding to the end of each row
        std::uint32_t bytesPerLine = 0;

        // Identifies the mapped V4L2 buffer containing this frame.
        // The capture component uses this value when returning the buffer to the driver with VIDIOC_QBUF
        std::uint32_t bufferIndex = 0;

        // Frame number supplied by the V4L2 driver
        // This can be used to detect dropped frames
        std::uint32_t sequence = 0;

        BayerPattern bayerPattern = BayerPattern::RGGB;
        RawStorage storage = RawStorage::TegraTR16;
    };
}