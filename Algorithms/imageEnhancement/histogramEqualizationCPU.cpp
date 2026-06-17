////////////////////////////////////////////////////////////////////////////////////

#include "histogramEqualizationCPU.h"
#include "ColorConversionCPU.h"

////////////////////////////////////////////////////////////////////////////////////
/*

    1. use advance loop for compute histogram and cumulative histogram
    2. if possible use uint8_t
*/
void computeHistogram(const cv::Mat& in , int width , int height , std::vector<int>& histogram)
{

    std::for_each(in.begin<uchar>() , in.end<uchar>() , [&](uchar pixel){
        histogram[pixel]++;
    });
}

////////////////////////////////////////////////////////////////////////////////////

void computeCumulativeHistogram(const std::vector<int>& pdf , std::vector<int>& cdf)
{
    // Prefix Sum
    std::partial_sum(begin(pdf) , end(pdf) , begin(cdf));
}
////////////////////////////////////////////////////////////////////////////////////

void computeMappedDigitalLevels(const std::vector<int>& cumulativeHistogram ,
                                std::vector<int>& mappedDigitalLevels,
                                int totalPixels,
                                int minHist)
{

   std::transform(begin(cumulativeHistogram) , end(cumulativeHistogram) ,
                  begin(mappedDigitalLevels) , [&](int value){
                    return static_cast<int>((255.0 * (value - minHist)) / (totalPixels - minHist));
                  });
}

////////////////////////////////////////////////////////////////////////////////////

cv::Mat computeHistogramEqualizedImage(const cv::Mat& in , std::vector<int>& mappedDigitalLevels , int width , int height)
{
    cv::Mat out(height , width , CV_8UC1);

    for(int row_id = 0 ; row_id < height ; row_id++)
    {
        for(int col_id = 0 ; col_id < width ; col_id++)
        {
            int pixelValue = static_cast<int>(in.at<uchar>(row_id , col_id));
            uchar mappedPixelValue = static_cast<uchar>(mappedDigitalLevels[pixelValue]);
            out.at<uchar>(row_id , col_id) = mappedPixelValue; 
        }
    }
    return out;
}

////////////////////////////////////////////////////////////////////////////////////
cv::Mat histogramEqualizationCPU(const cv::Mat& in)
{
    /*
        General Algorithm
        =================
            1. creata histogram array of the input image.
            2. find the cdf of the distribution.
            3. find the resultant pdf we want , that is (height * width) / 256 approx frequency 
                for each digital level.
            4. find the cdf of the resultant pdf.
            5. find the mapping of each Digital level in original cdf to resultant cdf (frequency should be equal or higher).
            6. create a new image with new mapping.

            7. Writing this function strictly for 8 bit image
        Approach
        ========

            1. Use brute approach and use as many data structure possible to store intermediate results.
    */

    ////////////////////////////////////////////////////////////////////////////////////

    int width = in.cols;
    int height = in.rows;
    int totalPixels = width * height;

    int channels = in.channels();
    cv::Mat colorConverted;

    if(channels == 3)
        colorConverted = grayScaleConversion_cpu(in);
    else
        colorConverted = std::move(in);
    ////////////////////////////////////////////////////////////////////////////////////

    std::vector<int> histogram(256,0);
    std::vector<int> cumulativeHistogram(256 , 0 );
    
    ////////////////////////////////////////////////////////////////////////////////////

    // histogram
    computeHistogram(colorConverted , width , height , histogram);

    // cumulative histogram
    computeCumulativeHistogram(histogram , cumulativeHistogram);
    ////////////////////////////////////////////////////////////////////////////////////

    auto minHist = *std::min_element(begin(cumulativeHistogram) , end(cumulativeHistogram));

    ////////////////////////////////////////////////////////////////////////////////////

    std::vector<int> mappedDigitalLevels(256 , 0);

    computeMappedDigitalLevels(cumulativeHistogram , mappedDigitalLevels, totalPixels , minHist);

    cv::Mat out = computeHistogramEqualizedImage(colorConverted , mappedDigitalLevels , width , height);

    ////////////////////////////////////////////////////////////////////////////////////

    return out;
}

////////////////////////////////////////////////////////////////////////////////////