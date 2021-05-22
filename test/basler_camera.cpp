#include <basler_camera/BaslerCamera.hpp>

#include <opencv2/opencv.hpp>

#ifdef _MSC_VER
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#include <sstream>
#include "../src/TimeMeasuring.hpp"

auto main(int argc, char** argv) -> int32_t
{
    auto exposition = std::stod(argv[1]);
    auto gain = std::stod(argv[2]);
    auto delay = std::stoi(argv[3]);
    std::string outDirectory = argv[4];

    if (outDirectory.empty())
    {
        std::cout << "Should be set folder for output" << std::endl;
        return 0;
    }
    BaslerCamera baslerCamera{};
    auto& availableCameras = baslerCamera.GetAvailableCameras();

    cv::Mat frame;

    auto i = 0;
    for (auto& availableCamera : availableCameras)
    {
       if (availableCamera.GetVendor() != "Basler")
       {
           continue;
       }
       availableCamera.StartCamera();
       availableCamera.SetExposureTime(exposition);
       availableCamera.SetGain(gain);
       std::cout << "Camera: " << availableCamera.GetVendor()
                 << " " << availableCamera.GetSerialNumber()
                 << " " << availableCamera.GetDeviceTemperature() << std::endl;
       fs::create_directories(outDirectory + "/" + std::to_string(i++));
    }
    auto frameNumber = 0;
    while(true)
    {
        TAKEN_TIME();
        auto index = 0;
        i = 0;
        for (auto& availableCamera : availableCameras)
        {
            if (availableCamera.GetVendor() != "Basler")
            {
                continue;
            }
            availableCamera.GetFrame(frame);
            cv::imshow(std::string("Camera #") + std::to_string(index++), frame);
            auto frameNumberStr = std::to_string(frameNumber);
            cv::imwrite(outDirectory + "/" + std::to_string(i++) + "/" + std::string(8 - frameNumberStr.length(), '0') + frameNumberStr + ".png", frame);
        }
        frameNumber++;
        if (cv::waitKey(delay) == 27)
        {
            break;
        }
    }
    for (auto& availableCamera : availableCameras)
    {
        //if (availableCamera.GetVendor() != "Basler")
        {
            continue;
        }
        availableCamera.StopCamera();
    }
    return 0;
}