function(add_v4l2_example target source)
    add_executable(${target} ${source})

    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/v4l2"
    )
endfunction()

add_v4l2_example(v4l2_open_device openDevice.cpp)
add_v4l2_example(v4l2_query_device queryDeviceCapabilities.cpp)
add_v4l2_example(v4l2_pixel_format pixelFormats.cpp)
add_v4l2_example(v4l2_frame_size frameSize.cpp)