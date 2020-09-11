#include <basler_camera/BaslerCamera.hpp>

#include <pylon/PylonIncludes.h>

#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>

namespace {
  auto CreateCameraInstance() -> Pylon::CInstantCamera
  {
    Pylon::PylonInitialize();
    return Pylon::CInstantCamera(Pylon::CTlFactory::GetInstance().CreateFirstDevice());
  }
} /// end namespace anonymous

class BaslerCamera::Impl {
public:
  Impl()
    : _instantCamera{CreateCameraInstance()}
  {
    std::cout << "Using device " << _instantCamera.GetDeviceInfo().GetModelName() << std::endl;

    // The parameter MaxNumBuffer can be used to control the count of buffers
    // allocated for grabbing. The default value of this parameter is 10.
    _instantCamera.MaxNumBuffer = 5;

    // Start the grabbing of c_countOfImagesToGrab images.
    // The camera device is parameterized with a default configuration which
    // sets up free-running continuous acquisition.
    _instantCamera.StartGrabbing(100);

    // This smart pointer will receive the grab result data.
    Pylon::CGrabResultPtr ptrGrabResult;

    // Camera.StopGrabbing() is called automatically by the RetrieveResult() method
    // when c_countOfImagesToGrab images have been retrieved.
    while (_instantCamera.IsGrabbing())
    {
      // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
      _instantCamera.RetrieveResult(5000, ptrGrabResult, Pylon::TimeoutHandling_ThrowException);

      // Image grabbed successfully?
      if (ptrGrabResult->GrabSucceeded())
      {
        // Access the image data.
        std::cout << "SizeX: " << ptrGrabResult->GetWidth() << std::endl;
        std::cout << "SizeY: " << ptrGrabResult->GetHeight() << std::endl;
        const uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();
        auto pixelType = ptrGrabResult->GetPixelType();
        std::cout << pixelType << std::endl;
        std::cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0] << std::endl;
        cv::Mat const frame = cv::Mat((int)ptrGrabResult->GetHeight(), (int)ptrGrabResult->GetWidth(), CV_8UC3, (void*)pImageBuffer);
        cv::imshow("", frame);
        cv::waitKey(1);
      }
      else
      {
        std::cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << std::endl;
      }
    }
  }

  ~Impl()
  {
    Pylon::PylonTerminate();
  }

private:
  Pylon::CInstantCamera _instantCamera;
};

BaslerCamera::~BaslerCamera() = default;
BaslerCamera::BaslerCamera(BaslerCamera&&) noexcept = default;
BaslerCamera& BaslerCamera::operator=(BaslerCamera&&) noexcept = default;

BaslerCamera::BaslerCamera()
  : _pImpl{std::make_unique<Impl>()}
{
}
