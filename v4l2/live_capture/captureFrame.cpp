#include "v4l2Camera.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
    void printFirstRaw10Values(const within::capture::RawFrameView& frame, std::size_t requestedCount)
    {
        if(frame.imageData == nullptr)
        {
            throw std::runtime_error("Frame has a null image pointer");
        }

        if(frame.storage != within::capture::RawStorage::TegraTR16)
        {
            throw std::runtime_error("This test expects Tegra T_R16 storage");
        }

        const std::size_t count = std::min<std::size_t>(requestedCount , frame.width);

        if(frame.bytesUsed < count * 2)
        {
            throw std::runtime_error("Frame is too small to print requested pixels");
        }

        const std::uint8_t* firstRow = frame.imageData;
        std::cout << "First " << count << " decoded RAW10 values" << std::endl;

        for(std::size_t column = 0 ; column < count ; ++column)
        {
            const std::size_t byteIndex = column * 2;

            const std::uint16_t storedWord = static_cast<std::uint16_t>(firstRow[byteIndex]) | (static_cast<std::uint16_t>(firstRow[byteIndex + 1]) << 8);
            const std::uint16_t raw10 = static_cast<std::uint16_t>(storedWord >> 6);
            std::cout << std::setw(4) << raw10 << ((column + 1 == count) ? '\n' : ' ');
        }
    }

    void writeRawFrame(const within ::capture::RawFrameView& frame , const std::string& outputPath)
    {
        std::ofstream output(outputPath , std::ios::binary);

        if(!output)
        {
            throw std::runtime_error("Could not create output file : " + outputPath);
        }

        output.write(reinterpret_cast<const char*>(frame.imageData) , static_cast<std::streamsize>(frame.bytesUsed));
        if(!output)
        {
            throw std::runtime_error("Could not write complete RAW frame");
        }
    }
}

int main(int argc, char** argv)
{
    const std::string outputPath = (argc >= 2) ? argv[1] : "/tmp/within_imx477_frame.raw";

    try
    {
        within::capture::CameraConfiguration configuration;

        within::capture::V4L2Camera camera(configuration);

        camera.initialize();

        std::cout << "Camera Initialization " << std::endl;
        std::cout << "Width : " << camera.width() << std::endl;
        std::cout << "Height : " << camera.height() << std::endl;
        std::cout << "Bytes per line : " << camera.bytesPerLine() << std::endl;
        std::cout << "Image Size : " << camera.imageSize() << std::endl;

        camera.start();

        constexpr int warmupFrameCount = 5;
        constexpr int timeoutMilliseconds = 2000;

        for(int frameNumber = 0 ; frameNumber <= warmupFrameCount ; ++frameNumber)
        {
            auto captured = camera.waitForFrame(timeoutMilliseconds);

            if(!captured)
            {
                throw std::runtime_error("Timed out waiting for camera frame");
            }

            const within :: capture :: RawFrameView frame = *captured;

            std::cout << "Dequeued Buffer" << frame.bufferIndex << std::endl;
            std::cout << "Sequence " << frame.sequence << std::endl;
            std::cout << "Bytes used" << frame.bytesUsed << std::endl;

            if(frameNumber < warmupFrameCount)
            {
                camera.requeueBuffer(frame.bufferIndex);
                continue;
            }

            const std::uint64_t minimumFrameBytes = static_cast<std::uint64_t>(frame.bytesPerLine) * frame.height;

            if(frame.bytesUsed < minimumFrameBytes)
            {
                throw std::runtime_error("Captured frame is smaller than stride * height");
            }

            printFirstRaw10Values(frame , 16);
            writeRawFrame(frame , outputPath);
            camera.requeueBuffer(frame.bufferIndex);
        }

        camera.stop();

        std::cout << "Raw Frame written to : " << outputPath << std::endl;
        return 0;
    }
    catch(const std::exception& error)
    {
        std::cerr << "Capture Failed : " << error.what() << std::endl;
        return 1;
    }
}
