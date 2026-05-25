/////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>

/////////////////////////////////////////////////////////////////////////////////

void capabilityQuery(int fd)
{
    struct v4l2_capability cap;

    /*
        1. All v4l2 devices support the VIDIOC_QUERYCAP ioctl call.
        2. It is used to identify kernel compatible with this specification and to
            obtain information about driver and hardware capabilities.
        3. This ioctl takes a pointer to a struct v4l2_capability which is filled by the driver.
        4. when the driver is not compatible with this specification the ioctl returns an EINVAL error code.

    
    */
    if(ioctl(fd , VIDIOC_QUERYCAP,  &cap) < 0)
    {
        perror("VIDIOC_QUERYCAP failed");
        return;
    }

    printf("\n === camera info ===== \n");

    
    printf("Driver         : %s\n",cap.driver); // name of the driver
    printf("Card           : %s\n",cap.card); // name of the device
    printf("Bus Info       : %s\n", cap.bus_info); // location of the device in the system
    printf("Version        : %u\n", cap.version); // version number of the driver
    printf("Capabilities   : 0x%x\n",cap.capabilities); // Available capabilities of the physical 
                                                        // device as a whole.
                                                        // the value is represented as bit mask. 
                                                        // V4L2_CAP_VIDEO_CAPTURE , V4L2_CAP_STREAMING
}

///////////////////////////////////////////////////////////////////////////////////

int main()
{
    const char *device_name = "/dev/video0";

    int fd = open(device_name , O_RDWR);

    if(fd < 0)
    {
        printf("Failed to open device %s : %s \n" , device_name , strerror(errno));
        return -1;
    }

    printf("Camera opened successfully  ! fd = %d\n", fd);

    capabilityQuery(fd);
    close(fd);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////