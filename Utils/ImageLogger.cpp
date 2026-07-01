////////////////////////////////////////////////////////////////////////////////////

#include "ImageLogger.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <sstream>
#include <utility>

////////////////////////////////////////////////////////////////////////////////////

ImageLogger :: ImageLogger(std::filesystem::path rootDirectory)
:rootDirectory(std::move(rootDirectory))
{

}

////////////////////////////////////////////////////////////////////////////////////

bool ImageLogger :: saveCurrentComparison(const ProcessedFrame& frame ,
                                          std::string& savedDirectory ,
                                          std::string& errorMessage) const
{
    if(frame.input.empty() || frame.output.empty())
    {
        errorMessage = "No valid frame is available to save.";
        return false;
    }

    const std::string timestamp = formatTimestamp(frame.timestamp);
    const std::string pipelineName = buildPipelineName(frame.pipeline);
    const std::string sessionName = sanitizeFileName(timestamp + "_" + pipelineName + "_" + frame.runtimeName);
    const std::filesystem::path sessionDirectory = rootDirectory / sessionName;

    try
    {
        std::filesystem::create_directories(sessionDirectory);
    }
    catch(const std::filesystem::filesystem_error& error)
    {
        errorMessage = error.what();
        return false;
    }

    const std::filesystem::path inputPath = sessionDirectory / "00_input_bgr.png";
    const std::filesystem::path outputPath = sessionDirectory / "01_output_pipeline.png";
    const std::filesystem::path comparisonPath = sessionDirectory / "02_comparison_input_output.png";
    const std::filesystem::path metadataPath = sessionDirectory / "metadata.json";

    const cv::Mat inputBGR = convertToBGR(frame.input);
    const cv::Mat outputBGR = convertToBGR(frame.output);
    const cv::Mat comparison = createComparisonImage(inputBGR , outputBGR);

    if(!cv::imwrite(inputPath.string() , inputBGR))
    {
        errorMessage = "Failed to write " + inputPath.string();
        return false;
    }

    if(!cv::imwrite(outputPath.string() , outputBGR))
    {
        errorMessage = "Failed to write " + outputPath.string();
        return false;
    }

    if(!cv::imwrite(comparisonPath.string() , comparison))
    {
        errorMessage = "Failed to write " + comparisonPath.string();
        return false;
    }

    if(!writeMetadata(metadataPath , frame , sessionName , errorMessage))
        return false;

    savedDirectory = sessionDirectory.string();
    return true;
}

////////////////////////////////////////////////////////////////////////////////////

std::string ImageLogger :: algoTypeToString(AlgoType algo)
{
    switch(algo)
    {
        case AlgoType::Threshold:
            return "threshold";
        case AlgoType::Hdr:
            return "hdr";
        case AlgoType::Edge:
            return "edge";
        case AlgoType::GreyScaleConversion:
            return "grayscale_conversion";
        case AlgoType::LinearContrastStretch:
            return "linear_contrast_stretch";
        case AlgoType::HistogramEqualization:
            return "histogram_equalization";
        case AlgoType::blur:
            return "blur";
        case AlgoType::sharpening:
            return "sharpening";
        case AlgoType::None:
        default:
            return "none";
    }
}

////////////////////////////////////////////////////////////////////////////////////

std::string ImageLogger :: buildPipelineName(const std::vector<AlgoType>& pipeline)
{
    if(pipeline.empty())
        return "no_algorithm";

    std::ostringstream stream;

    for(size_t index = 0 ; index < pipeline.size() ; index++)
    {
        if(index > 0)
            stream << "_";

        stream << algoTypeToString(pipeline[index]);
    }

    return stream.str();
}

////////////////////////////////////////////////////////////////////////////////////

std::string ImageLogger :: formatTimestamp(const std::chrono::system_clock::time_point& timestamp)
{
    const std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
    std::tm localTime{};
    localtime_r(&time , &localTime);

    std::ostringstream stream;
    stream << std::put_time(&localTime , "%Y-%m-%d_%H-%M-%S");
    return stream.str();
}

////////////////////////////////////////////////////////////////////////////////////

std::string ImageLogger :: sanitizeFileName(std::string value)
{
    std::replace_if(value.begin() , value.end() ,
                    [](unsigned char character) {
                        return !(std::isalnum(character) || character == '_' || character == '-');
                    },
                    '_');

    return value;
}

////////////////////////////////////////////////////////////////////////////////////

cv::Mat ImageLogger :: convertToBGR(const cv::Mat& image)
{
    if(image.channels() == 1)
    {
        cv::Mat bgrImage;
        cv::cvtColor(image , bgrImage , cv::COLOR_GRAY2BGR);
        return bgrImage;
    }

    if(image.channels() == 4)
    {
        cv::Mat bgrImage;
        cv::cvtColor(image , bgrImage , cv::COLOR_BGRA2BGR);
        return bgrImage;
    }

    return image.clone();
}

////////////////////////////////////////////////////////////////////////////////////

cv::Mat ImageLogger :: createComparisonImage(const cv::Mat& input , const cv::Mat& output)
{
    cv::Mat resizedOutput;

    if(input.size() != output.size())
        cv::resize(output , resizedOutput , input.size());
    else
        resizedOutput = output;

    cv::Mat comparison;
    cv::hconcat(input , resizedOutput , comparison);

    cv::putText(comparison , "Input BGR" , cv::Point(20 , 40) ,
                cv::FONT_HERSHEY_SIMPLEX , 1.0 , cv::Scalar(255 , 255 , 255) , 2);
    cv::putText(comparison , "Output Pipeline" , cv::Point(input.cols + 20 , 40) ,
                cv::FONT_HERSHEY_SIMPLEX , 1.0 , cv::Scalar(255 , 255 , 255) , 2);

    return comparison;
}

////////////////////////////////////////////////////////////////////////////////////

bool ImageLogger :: writeMetadata(const std::filesystem::path& metadataPath ,
                                  const ProcessedFrame& frame ,
                                  const std::string& sessionName ,
                                  std::string& errorMessage)
{
    std::ofstream file(metadataPath);

    if(!file.is_open())
    {
        errorMessage = "Failed to write " + metadataPath.string();
        return false;
    }

    file << "{\n";
    file << "  \"session\": \"" << sessionName << "\",\n";
    file << "  \"timestamp\": \"" << formatTimestamp(frame.timestamp) << "\",\n";
    file << "  \"runtime\": \"" << frame.runtimeName << "\",\n";
    file << "  \"input_file\": \"00_input_bgr.png\",\n";
    file << "  \"output_file\": \"01_output_pipeline.png\",\n";
    file << "  \"comparison_file\": \"02_comparison_input_output.png\",\n";
    file << "  \"input_width\": " << frame.input.cols << ",\n";
    file << "  \"input_height\": " << frame.input.rows << ",\n";
    file << "  \"input_channels\": " << frame.input.channels() << ",\n";
    file << "  \"output_width\": " << frame.output.cols << ",\n";
    file << "  \"output_height\": " << frame.output.rows << ",\n";
    file << "  \"output_channels\": " << frame.output.channels() << ",\n";
    file << "  \"pipeline\": [";

    for(size_t index = 0 ; index < frame.pipeline.size() ; index++)
    {
        if(index > 0)
            file << ", ";

        file << "\"" << algoTypeToString(frame.pipeline[index]) << "\"";
    }

    file << "]\n";
    file << "}\n";

    return true;
}

////////////////////////////////////////////////////////////////////////////////////
