#include "utils.h"
cv::Mat convertYUY2ToBGR(cv::Mat imageData)
{

    int rows = imageData.rows;
    int cols = imageData.cols;

    cv::Mat bgrImageData(rows , cols , CV_8UC3);
    uint8_t* out = bgrImageData.data;

    for(int row_id = 0 ; row_id < rows ; row_id++)
    {
        uint8_t* row = imageData.ptr<uint8_t>(row_id);
        for(int col_id = 0 ; col_id < cols ; col_id += 2)
        {
            auto index =col_id * 2;

            auto y0 = static_cast<uint8_t>(row[index + 0]);
            auto u = static_cast<uint8_t>(row[index + 1]);
            auto y1 = static_cast<uint8_t>(row[index + 2]);
            auto v = static_cast<uint8_t>(row[index + 3]);
            
            //std::cout << static_cast<int>(y0) << std::endl;
            auto b1 = static_cast<uint8_t>(1.164 * (y0 - 16) + 2.017 * (u - 128));
            auto g1 = static_cast<uint8_t>(1.164 * (y0 - 16) - 0.813 * (v - 128) - 0.392 * (u - 128));
            auto r1 = static_cast<uint8_t>(1.164 * ( y0 - 16) + 1.596 * (v - 128));


            auto b2 = static_cast<uint8_t>(1.164 * (y1 - 16) + 2.018 * (u - 128));
            auto g2 = static_cast<uint8_t>(1.164 * (y1 - 16) - 0.813 * (v - 128) - 0.391 * (u - 128));
            auto r2 = static_cast<uint8_t>(1.164 * ( y1 - 16) + 1.596 * (v - 128));

            int out_idx = (row_id * cols + col_id) * 3;
            out[out_idx] = b1;
            out[out_idx + 1] = g1;
            out[out_idx + 2] = r1;

            out[out_idx + 3] = b2;
            out[out_idx + 4] = g2;
            out[out_idx + 5] = r2;

        }
    }
    std::cout << static_cast<int>(bgrImageData.ptr<uint8_t>(0)[0]) << std::endl;
    return bgrImageData;
    /*
    
    B = 1.164(Y - 16)                   + 2.018(U - 128)

G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)

R = 1.164(Y - 16) + 1.596(V - 128)
    */
}