#include "v4l2Camera.h"

#include <cerrno>
#include <cstdint>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

#include <sys/ioctl.h>
#include <unistd.h>

#include <sys/mman.h>
#include <algorithm>
#include <poll.h>

/*

    1. This implements the Linux V4L2 protocol
            1. query capabilities
            2. configure format
            3. request buffers
            4. query each buffer
            5. mmap each buffer
            6. queue buffers
            7. STREAMON
            8. poll
            9. DQBUF
            10. process
            11. STREAMOFF
            12. munmap
            13.close

    
*/
namespace within ::capture
{
    V4L2Camera :: V4L2Camera(CameraConfiguration configuration)
    :configuration_(std::move(configuration))
    {

    }

    V4L2Camera :: ~V4L2Camera()
    {
        stop();
        unmapBuffers();
        closeDevice();
    }

    /*
    
        1. Initialize()
            1. openDevice succeeds
            2. queryCapabilities fails -> catch executes -> closeDevice -> Original Exception is rethrown
        2. what VIDIOC_S_FMT does?
            1. This structure contains your request
    */
    void V4L2Camera::initialize()
    {
        if(initialized_)
        {
            throw std::logic_error("V4L2Camera has already been initialized");
        }

        try
        {
            openDevice();
            queryCapabilities();
            configureFormat();
            requestAndMapBuffers();

            initialized_=true;
        }
        catch(...)
        {
            unmapBuffers();
            closeDevice();
            throw;
        }
    }

    void V4L2Camera :: openDevice()
    {
        /*
            1. The returned file descriptor is a handle to the device
            2. It is used for:
                1. configuration commands.
                2. buffer management commands.
                3. starting and stopping capture
                4. waiting for frames.
        */
        if(fileDescriptor_ != -1)
        {
            throw std::logic_error("Camera Device is already Open");
        }

        // O_RDWR - The device is opened for reading and writing.
        // O_NONBLOCK - dequeue operation will not freeze the thread indefinitely
        fileDescriptor_ = ::open(configuration_.devicePath.c_str() , O_RDWR | O_NONBLOCK);

        if(fileDescriptor_ == -1)
        {
            throw std::system_error(errno , std::generic_category() , "Failed to open camera device" + configuration_.devicePath);
        }
    }

    void V4L2Camera ::queryCapabilities()
    {

        /*
            VIDIOC_QUERYCAP asks the driver what it supports.
        */
        if(fileDescriptor_ == -1)
        {
            throw std::logic_error("Cannot query capabilities before opening the camera");
        }

        v4l2_capability capability{};

        int result;

        do
        {
            result = ::ioctl(fileDescriptor_ , VIDIOC_QUERYCAP , &capability);
        }
        while(result == -1 && errno ==EINTR);

        if(result == -1)
        {
            throw std::system_error(errno , std::generic_category() , "VIDIOC_QUERYCAP failed");
        }

        std::uint32_t deviceCapabilities = capability.capabilities;

        if(capability.capabilities & V4L2_CAP_DEVICE_CAPS)
        {
            deviceCapabilities = capability.device_caps;
        }

        if(!(deviceCapabilities & V4L2_CAP_VIDEO_CAPTURE))
        {
            throw std::runtime_error("Camera Does not support single plane video capture");
        }

        if(!(deviceCapabilities & V4L2_CAP_STREAMING))
        {
            throw std::runtime_error("Camera does not support streaming buffers");
        }
    }

    void V4L2Camera :: closeDevice() noexcept
    {
        if(fileDescriptor_ != -1)
        {
            ::close(fileDescriptor_);
            fileDescriptor_ = -1;
        }

        initialized_ = false;
    }

    bool V4L2Camera :: isInitialized() const noexcept
    {
        return initialized_;
    }

    bool V4L2Camera :: isStreaming() const noexcept
    {
        return streaming_;
    }

    void V4L2Camera::configureFormat()
    {
        /*
            1. VIDIOC_S_FMT sends the requested:
                1. width
                2. height
                3. pixel format
                4. buffer type
            2. After the call , the driver writes the negotiated values back into the same structure.
            3. width , height , byterperline , sizeimage are important.
            4. Each pixel occupies 16 bits in memory even though only 10 bits contain sensor data.
        */
        if(fileDescriptor_ == -1)
        {
            throw std::logic_error("Cannot configure format before opening the camera");
        }
        
        v4l2_format format{};
        format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        format.fmt.pix.width = configuration_.width;
        format.fmt.pix.height = configuration_.height;
        format.fmt.pix.pixelformat = configuration_.pixelFormat;
        format.fmt.pix.field = V4L2_FIELD_NONE;

        int result;

        do
        {
            result = ::ioctl(fileDescriptor_ , VIDIOC_S_FMT , &format);
        }while(result == -1 && errno == EINTR);

        if(result == -1)
        {
            throw std::system_error(errno , std::generic_category() , "VIDIOC_S_FMT failed");
        }

        actualWidth_ = format.fmt.pix.width;
        actualHeight_ = format.fmt.pix.height;
        actualPixelFormat_ = format.fmt.pix.pixelformat;
        bytesPerLine_ = format.fmt.pix.bytesperline;
        imageSize_ = format.fmt.pix.sizeimage;

        if(actualPixelFormat_ != configuration_.pixelFormat)
        {
            throw std::runtime_error("The Camera did not accept the requested pixel format");
        }

        if(actualWidth_ == 0 || actualHeight_ == 0)
        {
            throw std::runtime_error("The Camera returned invalid image dimensions");
        }

        if(bytesPerLine_ == 0 || imageSize_ == 0)
        {
            throw std::runtime_error("The camera returned an invalid memory layout");
        }

        /*
        
            Tegra T_R16 stores every RAW10 sample in one 16 bit word.
            Every tow must contain at least width * 2 bytes
        */

        if(configuration_.storage == RawStorage::TegraTR16)
        {
            const std::uint64_t minimumBytesPerLine = static_cast<std::uint64_t>(actualWidth_) * 2;

            if(bytesPerLine_ < minimumBytesPerLine)
            {
                throw std::runtime_error("T_R16 row stride is smaller than width * 2");
            }
        }

        /*
            sizeimage must contain at least all rows , including any row padding
        */
        
        const std::uint64_t minimumImageSize = static_cast<std::uint64_t>(bytesPerLine_) * actualHeight_;

        if(imageSize_ < minimumImageSize)
        {
            throw std::runtime_error("V4L2 size image is smaller than stride * height");
        }
    }

    void V4L2Camera::requestAndMapBuffers()
    {
        if(fileDescriptor_ == -1)
        {
            throw std::logic_error("Cannot request buffers before opening the camera");
        }

        if(!mappedBuffers_.empty())
        {
            throw std::logic_error("Camera buffers have already been mapped");
        }

        v4l2_requestbuffers request{};

        request.count = configuration_.requestedBufferCount;
        request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        request.memory = V4L2_MEMORY_MMAP;

        int result;

        do
        {
            result = ::ioctl(fileDescriptor_ , VIDIOC_REQBUFS , &request);
        }while(result == -1 && errno == EINTR);

        if(result == -1)
        {
            throw std::system_error(errno , std::generic_category() , "VIDIOC_REQBUFS failed");
        }

        if(request.count < 2)
        {
            throw std::runtime_error("V4L2 allocated fewer than two capture buffers");
        }

        mappedBuffers_.reserve(request.count);

        for(std::uint32_t index = 0 ; index < request.count ; ++index)
        {
            v4l2_buffer buffer{};

            buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buffer.memory = V4L2_MEMORY_MMAP;
            buffer.index = index;

            do
            {
                result = ::ioctl(fileDescriptor_ , VIDIOC_QUERYBUF , &buffer);
            }while(result == -1 && errno == EINTR);

            if(result == -1)
            {
                throw std::system_error(errno , std::generic_category() , "VIDIOC_QUERYBUF failed for buffer " + std::to_string(index));
            }

            if(buffer.length < imageSize_)
            {
                throw std::runtime_error("Mapped V4L2 buffer is smaller than sizeimage");
            }

            void* address = ::mmap(nullptr , buffer.length , PROT_READ | PROT_WRITE , MAP_SHARED ,
                                 fileDescriptor_ ,
                                  static_cast<off_t>(buffer.m.offset));
            
            if(address == MAP_FAILED)
            {
                throw std::system_error(errno , std::generic_category() , "mmap failed for buffer" + std::to_string(index));
            }

            mappedBuffers_.push_back(MappedBuffer{address , static_cast<std::size_t>(buffer.length)});
        }

        bufferDequeued_.assign(mappedBuffers_.size(), 0);
    }

    void V4L2Camera::unmapBuffers() noexcept
    {
        for(MappedBuffer& buffer : mappedBuffers_)
        {
            if(buffer.address != nullptr && buffer.address != MAP_FAILED)
            {
                ::munmap(buffer.address , buffer.length);
            }

            buffer.address = nullptr;
            buffer.length = 0;
        }

        mappedBuffers_.clear();
        bufferDequeued_.clear();
    }

    void V4L2Camera :: queueAllBuffers()
    {
        if(mappedBuffers_.empty())
        {
            throw std::logic_error("Cannot queue buffers before mapping them");
        }

        if(bufferDequeued_.size() != mappedBuffers_.size())
        {
            throw std::logic_error("V4L2 buffer ownership state is inconsistent");
        }

        for(std::size_t index = 0 ; index < mappedBuffers_.size() ; ++index)
        {
            v4l2_buffer buffer{};
            buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buffer.memory = V4L2_MEMORY_MMAP;
            buffer.index = static_cast<std::uint32_t>(index);

            int result;

            do
            {
                result = ::ioctl(fileDescriptor_ , VIDIOC_QBUF , &buffer);
            }while(result == -1 && errno == EINTR);

            if(result == -1)
            {
                throw std::system_error(errno , std::generic_category() , "VIDIOC_QBUF failed for buffer" + std::to_string(index));
            }
            
            // The driver owns the buffer after QBUF succeeds.
            bufferDequeued_[index] = 0;
        }
    }

    void V4L2Camera::start()
    {
        if(!initialized_)
        {
            throw std::logic_error("Cannot Start an uninitialized camera");
        }

        if(streaming_)
        {
            throw std::logic_error("Camera is already streaming");
        }

        queueAllBuffers();

        v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        int result;

        do
        {
            result = ::ioctl(fileDescriptor_ , VIDIOC_STREAMON , &type);
        }while(result == -1 && errno == EINTR);

        if(result == -1)
        {
            throw std::system_error(errno , std::generic_category() , "VIDIOC_STREAMON failed");
        }

        streaming_ = true;
    }

    std::optional<RawFrameView> V4L2Camera :: waitForFrame(int timeoutMilliseconds)
    {
        if(!streaming_)
        {
            throw std::logic_error("Cannot wait for a frame before streaming starts");
        }

        if(timeoutMilliseconds < 0)
        {
            throw std::invalid_argument("Frame timeout cannot be negative");
        }

        pollfd pollDescriptor{};

        pollDescriptor.fd = fileDescriptor_;
        pollDescriptor.events = POLLIN;

        int pollResult;

        do
        {
            pollResult = ::poll(&pollDescriptor , 1 , timeoutMilliseconds);
        }while(pollResult == -1 && errno == EINTR);

        if(pollResult == 0)
        {
            // Timeout is not necessarily a camera error
            return std::nullopt;
        }

        if(pollResult == -1)
        {
            throw std::system_error(errno , std::generic_category() , "Poll failed while waiting for a camera frame");
        }

        if(pollDescriptor.revents & (POLLERR | POLLHUP | POLLNVAL))
        {
            throw std::runtime_error("Camera reported a polling error");
        }

        if(!(pollDescriptor.revents & POLLIN))
        {
            throw std::runtime_error("Poll returned without a readable camera frame");
        }

        v4l2_buffer buffer{};

        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;

        int dequeueResult;

        do
        {
            dequeueResult = ::ioctl(fileDescriptor_ , VIDIOC_DQBUF , &buffer);
        }while(dequeueResult == -1 && errno == EINTR);

        if(dequeueResult == -1)
        {
            if(errno == EAGAIN)
            {
                return std::nullopt;
            }

            throw std::system_error(errno , std::generic_category() , "VIDIOC_DQBUF failed");
        }

        if(buffer.index >= mappedBuffers_.size())
        {
            throw std::runtime_error("Driver returned an invalid V4L2 buffer index");
        }

        if(bufferDequeued_[buffer.index] != 0)
        {
            throw std::logic_error("Driver returned a buffer already owned by the application");
        }

        // DQBUF transfers ownership from the driver to application
        bufferDequeued_[buffer.index] = 1;

        if(buffer.bytesused > mappedBuffers_[buffer.index].length)
        {
            try
            {
                requeueBuffer(buffer.index);
            }
            catch(...)
            {
                // Preserve the original invalid - size error
            }

            throw std::runtime_error("bytesused is larger than the mapped buffer");
        }

        RawFrameView frame;

        frame.imageData = static_cast<const std::uint8_t*>(mappedBuffers_[buffer.index].address);
        frame.bytesUsed = buffer.bytesused;
        frame.width = actualWidth_;
        frame.height = actualHeight_;
        frame.bytesPerLine = bytesPerLine_;
        frame.bufferIndex = buffer.index;
        frame.sequence = buffer.sequence;
        frame.bayerPattern = configuration_.bayerPattern;
        frame.storage = configuration_.storage;
        
        return frame;
    }

    void V4L2Camera :: requeueBuffer(std::uint32_t bufferIndex)
    {
        if(!streaming_)
        {
            throw std::logic_error("Cannot requeue a buffer while not streaming");
        }

        if(bufferIndex >= mappedBuffers_.size())
        {
            throw std::out_of_range("V4L2 buffer index is out of range");
        }

        if(bufferDequeued_[bufferIndex] == 0)
        {
            throw std::logic_error("Cannot requeue a buffer not owned by the application");
        }

        v4l2_buffer buffer{};

        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = bufferIndex;

        int result;

        do
        {
            result = ::ioctl(fileDescriptor_ , VIDIOC_QBUF , &buffer);
        }while(result == -1 && errno == EINTR);

        if(result == -1)
        {
            throw std::system_error(errno , std::generic_category() , "VIDIOC_QBUF failed while requeueing buffer");
        }

        bufferDequeued_[bufferIndex] = 0;
    }

    void V4L2Camera::stop() noexcept
    {
        if(!streaming_)
        {
            return;
        }

        v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        int result;

        do
        {
            result = ::ioctl(fileDescriptor_ , VIDIOC_STREAMOFF , &type);
        }while(result == -1 && errno == EINTR);

        streaming_ = false;

        std::fill(bufferDequeued_.begin() , bufferDequeued_.end() , 0);

    }
    

    std::uint32_t V4L2Camera::width() const noexcept
    {
        return actualWidth_;
    }

    std::uint32_t V4L2Camera::height() const noexcept
    {
        return actualHeight_;
    }

    std::uint32_t V4L2Camera::bytesPerLine() const noexcept
    {
        return bytesPerLine_;
    }

    std::uint32_t V4L2Camera::imageSize() const noexcept
    {
        return imageSize_;
    }
}
