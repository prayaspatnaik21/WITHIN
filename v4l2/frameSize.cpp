///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/videodev2.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <cstring>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// which resolutions are supported for the pixel format ?
/*

    1. Discrete - The driver gives exact supported resolutions.
    2. StepWise - Width and Height may vary within a range using fixed steps.
    3. Continuous - Any Value within a range may be accepted.
    
*/
int main()
{
    const char* device = "/dev/video0";

    struct v4l2_fmtdesc format{};
    struct v4l2_frmsizeenum frameSize{};
    
    int fd = open(device , O_RDWR);
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    for(format.index = 0 ; ioctl(fd , VIDIOC_ENUM_FMT , &format) == 0 ; ++format.index)
    {
        std::cout << "Format Index : " << format.index << std::endl;
        std::cout << "Description : " << reinterpret_cast<const char*>(format.description) << std::endl;
    }

    frameSize.pixel_format = format.pixelformat;

    for(frameSize.index = 0 ; ioctl(fd , VIDIOC_ENUM_FRAMESIZES , &frameSize) == 0 ; ++frameSize.index)
    {
        if(frameSize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
        {
            std::cout << frameSize.discrete.width << " x " << frameSize.discrete.height << std::endl;
        }
    }
    close(fd);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
