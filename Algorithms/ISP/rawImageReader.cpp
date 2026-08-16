#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{

constexpr std::size_t kWidth = 4032;
constexpr std::size_t kHeight = 3040;
constexpr std::size_t kBytesPerStoredPixel = 2;
constexpr std::size_t kExpectedFrameBytes =
    kWidth * kHeight * kBytesPerStoredPixel;

// This standalone program intentionally uses one known capture.
// Change this path when you want to inspect another RAW frame.
const std::string kRawImagePath =
    "/Users/prayaspatnaik/opt/anaconda3/ProjectMe/Captures/"
    "imx477_rggb10_4032x3040_frame_0.raw";

struct RawImage
{
    std::size_t width = 0;
    std::size_t height = 0;
    std::size_t sourceByteCount = 0;

    // Row-major RAW10 matrix. Pixel (row, column) is stored at
    // pixels[row * width + column]. Each value is in the range 0..1023.
    std::vector<std::uint16_t> pixels;

    const std::uint16_t& at(
        std::size_t row,
        std::size_t column) const
    {
        return pixels.at(row * width + column);
    }
};

RawImage readTegraRaw10(const std::string& filePath)
{
    std::ifstream input(
        filePath,
        std::ios::binary | std::ios::ate
    );

    if (!input)
    {
        throw std::runtime_error(
            "Could not open RAW image: " + filePath);
    }

    const std::streamoff fileSize = input.tellg();

    if (fileSize < 0)
    {
        throw std::runtime_error(
            "Could not determine RAW image size");
    }

    if (static_cast<std::size_t>(fileSize) != kExpectedFrameBytes)
    {
        throw std::runtime_error(
            "Unexpected RAW image size. Expected " +
            std::to_string(kExpectedFrameBytes) +
            " bytes, received " +
            std::to_string(fileSize) +
            " bytes");
    }

    input.seekg(0, std::ios::beg);

    std::vector<std::uint8_t> storedBytes(kExpectedFrameBytes);

    input.read(
        reinterpret_cast<char*>(storedBytes.data()),
        static_cast<std::streamsize>(storedBytes.size())
    );

    if (!input)
    {
        throw std::runtime_error(
            "Could not read the complete RAW image");
    }

    RawImage image;
    image.width = kWidth;
    image.height = kHeight;
    image.sourceByteCount = kExpectedFrameBytes;
    image.pixels.resize(kWidth * kHeight);

    for (std::size_t index = 0;
         index < image.pixels.size();
         ++index)
    {
        const std::size_t byteIndex = index * 2;

        // The captured Tegra T_R16 word is little-endian.
        const std::uint16_t storedWord =
            static_cast<std::uint16_t>(storedBytes[byteIndex]) |
            (static_cast<std::uint16_t>(storedBytes[byteIndex + 1]) << 8);

        // RAW10 occupies bits 15..6 in Tegra's T_R16 representation.
        image.pixels[index] =
            static_cast<std::uint16_t>(storedWord >> 6);
    }

    return image;
}

const char* bayerColorAt(
    std::size_t row,
    std::size_t column)
{
    if ((row % 2 == 0) && (column % 2 == 0))
    {
        return "R";
    }

    if ((row % 2 == 0) && (column % 2 == 1))
    {
        return "Gr";
    }

    if ((row % 2 == 1) && (column % 2 == 0))
    {
        return "Gb";
    }

    return "B";
}

void printImageSummary(const RawImage& image)
{
    std::cout
        << "RAW image path : " << kRawImagePath << '\n'
        << "File size      : " << image.sourceByteCount << " bytes\n"
        << "Dimensions     : " << image.width << " x " << image.height << '\n'
        << "Pixel count    : " << image.pixels.size() << '\n'
        << "Decoded format : 10-bit RGGB, values 0..1023\n\n";

    constexpr std::size_t rowsToPrint = 4;
    constexpr std::size_t columnsToPrint = 8;

    std::cout
        << "First " << rowsToPrint << " rows and "
        << columnsToPrint << " columns:\n\n";

    for (std::size_t row = 0; row < rowsToPrint; ++row)
    {
        for (std::size_t column = 0;
             column < columnsToPrint;
             ++column)
        {
            std::cout
                << "[" << std::setw(2) << bayerColorAt(row, column)
                << ":" << std::setw(4) << image.at(row, column)
                << "] ";
        }

        std::cout << '\n';
    }
}

////////////////////////////////////////////////////////////////////////////////
// Write your demosaic data structures and algorithm below this line.
//
// The input to your algorithm can be:
//
//     const RawImage& raw
//
// Access one RAW10 sample with:
//
//     raw.at(row, column)
//
// The CFA order is:
//
//     R  Gr  R  Gr ...
//     Gb B   Gb B  ...
////////////////////////////////////////////////////////////////////////////////

RawImage demosaicBilinearInterpolation(const RawImage& raw)
{
    
}
 // namespace

int main()
{
    try
    {
        const RawImage raw = readTegraRaw10(kRawImagePath);

        printImageSummary(raw);

        // Later, call your demosaic function here using `raw`.

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
