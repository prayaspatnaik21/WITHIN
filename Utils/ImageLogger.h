////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ProcessedFrame.h"

#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////

class ImageLogger
{
    public:
        explicit ImageLogger(std::filesystem::path rootDirectory = "logs");
        bool saveCurrentComparison(const ProcessedFrame& frame , std::string& savedDirectory , std::string& errorMessage) const;

    private:
        std::filesystem::path rootDirectory;

        static std::string algoTypeToString(AlgoType algo);
        static std::string buildPipelineName(const std::vector<AlgoType>& pipeline);
        static std::string formatTimestamp(const std::chrono::system_clock::time_point& timestamp);
        static std::string sanitizeFileName(std::string value);
        static cv::Mat convertToBGR(const cv::Mat& image);
        static cv::Mat createComparisonImage(const cv::Mat& input , const cv::Mat& output);
        static bool writeMetadata(const std::filesystem::path& metadataPath ,
                                  const ProcessedFrame& frame ,
                                  const std::string& sessionName ,
                                  std::string& errorMessage);
};

////////////////////////////////////////////////////////////////////////////////////
