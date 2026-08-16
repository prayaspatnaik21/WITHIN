///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/videodev2.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*

1. https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/vidioc-querycap.html#vidioc-querycap

*/
#include <cerrno>
#include <cstring>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    // video device
    const char* device = "/dev/video0";


    v4l2_capability capability{};

    // fd is file descriptor
    // It is an integer that represents the open connection to the v4l2 device
    int fd = open(device , O_RDWR);

    if(fd == -1)
    {
        std::cerr << "Failed to open" << device << " : " << std::strerror(errno) << std::endl;
        return 1;
    }
    
    int result = ioctl(fd , VIDIOC_QUERYCAP , &capability);

    if(result)
    {
        std::cerr << "VIDIOC_QUERYCAP failed" << std::strerror(errno) << std::endl;
    }

    std::cout << "Driver : " << reinterpret_cast<const char*>(capability.driver) << std::endl;
    std::cout << "Card : " << reinterpret_cast<const char*>(capability.card) << std::endl;
    std::cout << "Bus : " << reinterpret_cast<const char*>(capability.bus_info) << std::endl;
    std::cout << "version : " << reinterpret_cast<const char*>(capability.version) << std::endl;

    /*
        1. V4L2_CAP_VIDEO_CAPTURE - Device supports the single planar API through the Video Capture Interface.
        2. V4L2_CAP_VIDEO_CAPTURE_MPLANE - Device supports the multi planar API through the Video Capture Interface.
        3. V4L2_CAP_VIDEO_OUTPUT - Device supports the single planar API through the Video Output Interface
        4. V4L2_CAP_VIDEO_OUTPUT_MPLANE - Device supports the Multi Planar API through the Video Output Interface.
        5. V4L2_CAP_VBI_CAPTURE - Device supports the Raw VBI Capture interface.
        6. V4L2_CAP_SDR_CAPTURE - Device supports the SDR capture interface.
        7. V4L2_CAP_EXT_PIX_FORMAT - Device supports the struct v4l2_pix_format extended fields.
        8. V4L2_CAP_EXT_SDR_OUTPUT - Device supports the SDR output interface.
    
    */

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if(capability.device_caps & V4L2_CAP_VIDEO_CAPTURE)
    {
        std::cout << "Supports Video Capture" << std::endl;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if(capability.device_caps & V4L2_CAP_STREAMING)
    {
        std::cout << "Supports Streaming Buffers" << std::endl;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if(capability.device_caps & V4L2_CAP_VIDEO_CAPTURE)
    {
        std::cout << "Device Supports the single planar API through the Video Capture Interface" << std::endl;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if(capability.device_caps & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
    {
        std::cout << "Device Supports the Multi Planar API through the Video Capture Interface" << std::endl;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if(capability.device_caps & V4L2_CAP_VIDEO_OUTPUT_MPLANE)
    {
        std::cout << "Device Supports the Multi Planar API through the Video Output interface" << std::endl;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if(capability.device_caps & V4L2_CAP_VBI_CAPTURE)
    {
        std::cout << "Device Supports the RAW VBI Capture Interface , providing Teletext and Closed Caption Data" << std::endl;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if(capability.device_caps & V4L2_CAP_SDR_CAPTURE)
    {
        std::cout << "Device Supports the SDR Capture Interface" << std::endl;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if(capability.device_caps & V4L2_CAP_EXT_PIX_FORMAT)
    {
        std::cout << "Device Supports the struct V4L2_PIX_FORMAT extended fields" << std::endl;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if(capability.device_caps & V4L2_CAP_SDR_OUTPUT)
    {
        std::cout << "Device Supports the SDR Output Interface" << std::endl;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    close(fd);
    return 0;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
