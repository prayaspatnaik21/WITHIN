#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

/// @return 
int xioctl(int fd, unsigned long request, void* argument)
{
    int result;

    do
    {
        result = ioctl(fd, request, argument);
    }
    while (result == -1 && errno == EINTR);

    return result;
}

std::string fourccToString(__u32 format)
{
    std::string result(4, ' ');

    result[0] = static_cast<char>(format & 0xFF);
    result[1] = static_cast<char>((format >> 8) & 0xFF);
    result[2] = static_cast<char>((format >> 16) & 0xFF);
    result[3] = static_cast<char>((format >> 24) & 0xFF);

    return result;
}

double fractionToFPS(const v4l2_fract& interval)
{
    if (interval.numerator == 0)
    {
        return 0.0;
    }

    return static_cast<double>(interval.denominator) /
           static_cast<double>(interval.numerator);
}

void printFrameIntervals(
    int fd,
    __u32 pixelFormat,
    __u32 width,
    __u32 height)
{
    v4l2_frmivalenum interval{};

    interval.pixel_format = pixelFormat;
    interval.width = width;
    interval.height = height;

    std::cout << "      Frame intervals:\n";

    bool foundInterval = false;

    for (interval.index = 0; ; ++interval.index)
    {
        if (xioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &interval) == -1)
        {
            if (errno != EINVAL)
            {
                std::cerr
                    << "VIDIOC_ENUM_FRAMEINTERVALS failed: "
                    << std::strerror(errno)
                    << '\n';
            }

            break;
        }

        foundInterval = true;

        if (interval.type == V4L2_FRMIVAL_TYPE_DISCRETE)
        {
            std::cout
                << "        "
                << interval.discrete.numerator
                << "/"
                << interval.discrete.denominator
                << " seconds per frame"
                << " = "
                << std::fixed
                << std::setprecision(3)
                << fractionToFPS(interval.discrete)
                << " FPS\n";
        }
        else if (interval.type == V4L2_FRMIVAL_TYPE_CONTINUOUS)
        {
            std::cout
                << "        Continuous interval range\n"
                << "          Minimum: "
                << interval.stepwise.min.numerator
                << "/"
                << interval.stepwise.min.denominator
                << " seconds\n"
                << "          Maximum: "
                << interval.stepwise.max.numerator
                << "/"
                << interval.stepwise.max.denominator
                << " seconds\n";
        }
        else if (interval.type == V4L2_FRMIVAL_TYPE_STEPWISE)
        {
            std::cout
                << "        Stepwise interval range\n"
                << "          Minimum: "
                << interval.stepwise.min.numerator
                << "/"
                << interval.stepwise.min.denominator
                << " seconds\n"
                << "          Maximum: "
                << interval.stepwise.max.numerator
                << "/"
                << interval.stepwise.max.denominator
                << " seconds\n"
                << "          Step: "
                << interval.stepwise.step.numerator
                << "/"
                << interval.stepwise.step.denominator
                << " seconds\n";
        }
    }

    if (!foundInterval)
    {
        std::cout << "        No frame intervals reported\n";
    }
}

void printFrameSizes(int fd, __u32 pixelFormat)
{
    v4l2_frmsizeenum frameSize{};

    frameSize.pixel_format = pixelFormat;

    bool foundSize = false;

    for (frameSize.index = 0; ; ++frameSize.index)
    {
        if (xioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frameSize) == -1)
        {
            if (errno != EINVAL)
            {
                std::cerr
                    << "VIDIOC_ENUM_FRAMESIZES failed: "
                    << std::strerror(errno)
                    << '\n';
            }

            break;
        }

        foundSize = true;

        if (frameSize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
        {
            const __u32 width = frameSize.discrete.width;
            const __u32 height = frameSize.discrete.height;

            std::cout
                << "    Resolution: "
                << width
                << "x"
                << height
                << '\n';

            printFrameIntervals(
                fd,
                pixelFormat,
                width,
                height
            );
        }
        else if (frameSize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS)
        {
            std::cout
                << "    Continuous resolution range\n"
                << "      Minimum: "
                << frameSize.stepwise.min_width
                << "x"
                << frameSize.stepwise.min_height
                << '\n'
                << "      Maximum: "
                << frameSize.stepwise.max_width
                << "x"
                << frameSize.stepwise.max_height
                << '\n';
        }
        else if (frameSize.type == V4L2_FRMSIZE_TYPE_STEPWISE)
        {
            std::cout
                << "    Stepwise resolution range\n"
                << "      Minimum: "
                << frameSize.stepwise.min_width
                << "x"
                << frameSize.stepwise.min_height
                << '\n'
                << "      Maximum: "
                << frameSize.stepwise.max_width
                << "x"
                << frameSize.stepwise.max_height
                << '\n'
                << "      Width step: "
                << frameSize.stepwise.step_width
                << '\n'
                << "      Height step: "
                << frameSize.stepwise.step_height
                << '\n';
        }
    }

    if (!foundSize)
    {
        std::cout << "    No frame sizes reported\n";
    }
}

void enumerateFormats(int fd, v4l2_buf_type type)
{
    v4l2_fmtdesc format{};

    format.type = type;

    std::cout << "\nSupported pixel formats\n";
    std::cout << "=======================\n";

    for (format.index = 0; ; ++format.index)
    {
        if (xioctl(fd, VIDIOC_ENUM_FMT, &format) == -1)
        {
            if (errno != EINVAL)
            {
                std::cerr
                    << "VIDIOC_ENUM_FMT failed: "
                    << std::strerror(errno)
                    << '\n';
            }

            break;
        }

        std::cout
            << "\nFormat index: "
            << format.index
            << '\n'
            << "  FourCC: "
            << fourccToString(format.pixelformat)
            << '\n'
            << "  Description: "
            << reinterpret_cast<const char*>(format.description)
            << '\n'
            << "  Flags: 0x"
            << std::hex
            << format.flags
            << std::dec
            << '\n';

        if (format.flags & V4L2_FMT_FLAG_COMPRESSED)
        {
            std::cout << "  Compressed: yes\n";
        }
        else
        {
            std::cout << "  Compressed: no\n";
        }

        if (format.flags & V4L2_FMT_FLAG_EMULATED)
        {
            std::cout << "  Emulated: yes\n";
        }
        else
        {
            std::cout << "  Emulated: no\n";
        }

        printFrameSizes(fd, format.pixelformat);
    }
}

void printCurrentSinglePlanarFormat(
    int fd,
    v4l2_buf_type type)
{
    v4l2_format format{};

    format.type = type;

    if (xioctl(fd, VIDIOC_G_FMT, &format) == -1)
    {
        std::cerr
            << "VIDIOC_G_FMT failed: "
            << std::strerror(errno)
            << '\n';

        return;
    }

    const v4l2_pix_format& pixel = format.fmt.pix;

    std::cout << "\nCurrent single-planar format\n";
    std::cout << "============================\n";

    std::cout
        << "Width:           " << pixel.width << '\n'
        << "Height:          " << pixel.height << '\n'
        << "Pixel format:    "
        << fourccToString(pixel.pixelformat) << '\n'
        << "Field:           " << pixel.field << '\n'
        << "Bytes per line:  " << pixel.bytesperline << '\n'
        << "Image size:      " << pixel.sizeimage << " bytes\n"
        << "Colorspace:      " << pixel.colorspace << '\n'
        << "Quantization:    " << pixel.quantization << '\n'
        << "Transfer func:   " << pixel.xfer_func << '\n'
        << "Y'CbCr encoding: " << pixel.ycbcr_enc << '\n';
}

void printCurrentMultiPlanarFormat(
    int fd,
    v4l2_buf_type type)
{
    v4l2_format format{};

    format.type = type;

    if (xioctl(fd, VIDIOC_G_FMT, &format) == -1)
    {
        std::cerr
            << "VIDIOC_G_FMT failed: "
            << std::strerror(errno)
            << '\n';

        return;
    }

    const v4l2_pix_format_mplane& pixel = format.fmt.pix_mp;

    std::cout << "\nCurrent multi-planar format\n";
    std::cout << "===========================\n";

    std::cout
        << "Width:           " << pixel.width << '\n'
        << "Height:          " << pixel.height << '\n'
        << "Pixel format:    "
        << fourccToString(pixel.pixelformat) << '\n'
        << "Field:           " << pixel.field << '\n'
        << "Number of planes:" << static_cast<int>(pixel.num_planes)
        << '\n'
        << "Colorspace:      " << pixel.colorspace << '\n'
        << "Quantization:    " << pixel.quantization << '\n'
        << "Transfer func:   " << pixel.xfer_func << '\n'
        << "Y'CbCr encoding: " << pixel.ycbcr_enc << '\n';

    for (unsigned int plane = 0;
         plane < pixel.num_planes;
         ++plane)
    {
        std::cout
            << "Plane " << plane << '\n'
            << "  Bytes per line: "
            << pixel.plane_fmt[plane].bytesperline
            << '\n'
            << "  Image size: "
            << pixel.plane_fmt[plane].sizeimage
            << " bytes\n";
    }
}

void printStreamingParameters(
    int fd,
    v4l2_buf_type type)
{
    v4l2_streamparm parameters{};

    parameters.type = type;

    if (xioctl(fd, VIDIOC_G_PARM, &parameters) == -1)
    {
        std::cerr
            << "VIDIOC_G_PARM failed: "
            << std::strerror(errno)
            << '\n';

        return;
    }

    const v4l2_captureparm& capture =
        parameters.parm.capture;

    std::cout << "\nCurrent streaming parameters\n";
    std::cout << "============================\n";

    std::cout
        << "Capability flags: 0x"
        << std::hex
        << capture.capability
        << std::dec
        << '\n';

    if (capture.capability & V4L2_CAP_TIMEPERFRAME)
    {
        std::cout << "Frame-rate selection: supported\n";
    }
    else
    {
        std::cout << "Frame-rate selection: not supported\n";
    }

    std::cout
        << "Time per frame: "
        << capture.timeperframe.numerator
        << "/"
        << capture.timeperframe.denominator
        << " seconds\n"
        << "Current FPS: "
        << std::fixed
        << std::setprecision(3)
        << fractionToFPS(capture.timeperframe)
        << '\n'
        << "Read buffers: "
        << capture.readbuffers
        << '\n';
}

int main(int argc, char** argv)
{
    const char* device =
        argc > 1 ? argv[1] : "/dev/video0";

    const int fd = open(
        device,
        O_RDWR | O_NONBLOCK
    );

    if (fd == -1)
    {
        std::cerr
            << "Failed to open "
            << device
            << ": "
            << std::strerror(errno)
            << '\n';

        return 1;
    }

    v4l2_capability capability{};

    if (xioctl(fd, VIDIOC_QUERYCAP, &capability) == -1)
    {
        std::cerr
            << "VIDIOC_QUERYCAP failed: "
            << std::strerror(errno)
            << '\n';

        close(fd);
        return 1;
    }

    __u32 deviceCapabilities = capability.capabilities;

    if (capability.capabilities & V4L2_CAP_DEVICE_CAPS)
    {
        deviceCapabilities = capability.device_caps;
    }

    std::cout << "Device\n";
    std::cout << "======\n";

    std::cout
        << "Path:    " << device << '\n'
        << "Driver:  "
        << reinterpret_cast<const char*>(capability.driver)
        << '\n'
        << "Card:    "
        << reinterpret_cast<const char*>(capability.card)
        << '\n'
        << "Bus:     "
        << reinterpret_cast<const char*>(capability.bus_info)
        << '\n';

    v4l2_buf_type type;

    if (deviceCapabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
    {
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        std::cout << "API:     Multi-planar capture\n";
    }
    else if (deviceCapabilities & V4L2_CAP_VIDEO_CAPTURE)
    {
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        std::cout << "API:     Single-planar capture\n";
    }
    else
    {
        std::cerr
            << "This node is not a video-capture device\n";

        close(fd);
        return 1;
    }

    if (deviceCapabilities & V4L2_CAP_STREAMING)
    {
        std::cout << "Streaming I/O: supported\n";
    }
    else
    {
        std::cout << "Streaming I/O: not advertised\n";
    }

    enumerateFormats(fd, type);

    if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
    {
        printCurrentMultiPlanarFormat(fd, type);
    }
    else
    {
        printCurrentSinglePlanarFormat(fd, type);
    }

    printStreamingParameters(fd, type);

    close(fd);
    return 0;
}