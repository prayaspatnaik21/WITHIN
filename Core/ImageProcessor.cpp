////////////////////////////////////////////////////////////////////////////////////

#include "ImageProcessor.h"
#include "CPURuntime.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////////////////////

ImageProcessor :: ImageProcessor()
:runTimePtr(std::make_shared<CPURuntime>())
{
    
}
////////////////////////////////////////////////////////////////////////////////////

void ImageProcessor :: setRunTime(std::shared_ptr<IRuntime> runtime)
{
    std::lock_guard<std::mutex> lock(processorMutex);
    runTimePtr = runtime;
}

////////////////////////////////////////////////////////////////////////////////////

void ImageProcessor :: addAlgorithm(AlgoType algo)
{
    std::lock_guard<std::mutex> lock(processorMutex);
    pipeline.push_back(algo);
}

////////////////////////////////////////////////////////////////////////////////////

void ImageProcessor ::removeAlgorithm(AlgoType algo)
{
    std::lock_guard<std::mutex> lock(processorMutex);
    pipeline.erase(std::remove(pipeline.begin() , pipeline.end() ,algo) , pipeline.end());
}

////////////////////////////////////////////////////////////////////////////////////

cv::Mat ImageProcessor :: process(const cv::Mat& in)
{
    return processWithMetadata(in).output;
}

////////////////////////////////////////////////////////////////////////////////////

ProcessedFrame ImageProcessor :: processWithMetadata(const cv::Mat& in)
{
    std::shared_ptr<IRuntime> runtimeSnapshot;
    std::vector<AlgoType> pipelineSnapshot;

    {
        std::lock_guard<std::mutex> lock(processorMutex);
        runtimeSnapshot = runTimePtr;
        pipelineSnapshot = pipeline;
    }

    cv::Mat frame = in.clone();

    for(auto algo : pipelineSnapshot)
    {
        frame = runtimeSnapshot->execute(algo , frame);
    }

    ProcessedFrame processedFrame;
    processedFrame.input = in.clone();
    processedFrame.output = frame.empty() ? in.clone() : frame;
    processedFrame.pipeline = std::move(pipelineSnapshot);
    processedFrame.runtimeName = runtimeSnapshot ? runtimeSnapshot->name() : "Unknown";
    processedFrame.timestamp = std::chrono::system_clock::now();
    return processedFrame;
}

////////////////////////////////////////////////////////////////////////////////////
