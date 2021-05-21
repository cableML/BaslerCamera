#include <basler_camera/BaslerCamera.hpp>

#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>

#include <pylon/PylonIncludes.h>

#include <iostream>

class BaslerCamera::DataStream::Impl
{
public:
   Impl(Pylon::IPylonDevice* pylonDevice)
      : _instantCamera{pylonDevice}
   {
      std::cout << "Using device " << _instantCamera.GetDeviceInfo().GetModelName() << std::endl;

      // The parameter MaxNumBuffer can be used to control the count of buffers
      // allocated for grabbing. The default value of this parameter is 10.
      _instantCamera.MaxNumBuffer = 5;
   }

   void StartCamera()
   {
      _instantCamera.StartGrabbing();
   }

   void StopCamera()
   {
      _instantCamera.StopGrabbing();
   }

   bool GetFrame(cv::Mat& frame)
   {
      Pylon::CGrabResultPtr ptrGrabResult;

      _instantCamera.RetrieveResult(5000, ptrGrabResult, Pylon::TimeoutHandling_ThrowException);

      // Image grabbed successfully?
      if (ptrGrabResult->GrabSucceeded())
      {
         std::cout << "SizeX: " << ptrGrabResult->GetWidth() << std::endl;
         std::cout << "SizeY: " << ptrGrabResult->GetHeight() << std::endl;
         const uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();
         auto pixelType = ptrGrabResult->GetPixelType();
         std::cout << "pixelType: " << pixelType << std::endl;
         std::cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0] << std::endl;
         frame = cv::Mat((int)ptrGrabResult->GetHeight(), (int)ptrGrabResult->GetWidth(), CV_8UC3, (void*)pImageBuffer);
      }
      else
      {
         std::cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << std::endl;
      }
      return true;
   }

   void SetExposureTime(uint64_t exposureTime)
   {
      // _instantCamera.GetN
   }

   void SetGain(uint32_t gain)
   {

   }

   ~Impl()
   {
   }

private:
   Pylon::CInstantCamera _instantCamera;
};

class BaslerCamera::Impl
{
public:
   Impl()
   {
      Pylon::PylonInitialize();
   }

   ~Impl()
   {
      Pylon::PylonTerminate();
   }

   auto GetAvailableCameras() -> std::vector<DataStream>&
   {
      Pylon::TlInfoList_t transportInfoList;
      auto countTransports = Pylon::CTlFactory::GetInstance().EnumerateTls(transportInfoList);
      for (auto& transportInfo : transportInfoList)
      {
         std::cout << "Transport:"
                   << "\n\t\t" << transportInfo.GetFriendlyName()
                   << "\n\t\t" << transportInfo.GetFullName()
                   << "\n\t\t" << transportInfo.GetVendorName()
                   << "\n\t\t" << transportInfo.GetDeviceClass()
                   << "\n\t\t" << transportInfo.GetTLType()
                   << "\n\t\t" << transportInfo.GetFileName()
                   << "\n\t\t" << transportInfo.GetInfoID()
                   << "\n\t\t" << transportInfo.GetModelName()
                   << "\n\t\t" << transportInfo.GetVersion()
                   << std::endl;
      }

      Pylon::DeviceInfoList_t deviceInfoList;
      auto countDevices = Pylon::CTlFactory::GetInstance().EnumerateDevices(deviceInfoList);
      for (auto& deviceInfo : deviceInfoList)
      {
         std::cout << "Device:"
            << "\n\t\t" << deviceInfo.GetFriendlyName()
            << "\n\t\t" << deviceInfo.GetFullName()
            << "\n\t\t" << deviceInfo.GetVendorName()
            << "\n\t\t" << deviceInfo.GetDeviceClass()
            << "\n\t\t" << deviceInfo.GetTLType()
            << "\n\t\t" << deviceInfo.GetSerialNumber()
            << "\n\t\t" << deviceInfo.GetUserDefinedName()
            << "\n\t\t" << deviceInfo.GetModelName()
            << "\n\t\t" << deviceInfo.GetDeviceVersion()
            << "\n\t\t" << deviceInfo.GetDeviceFactory()
            << "\n\t\t" << deviceInfo.GetXMLSource()
            << "\n\t\t" << deviceInfo.GetInterfaceID()
            << "\n\t\t" << deviceInfo.GetAddress()
            << "\n\t\t" << deviceInfo.GetIpAddress()
            << "\n\t\t" << deviceInfo.GetSubnetAddress()
            << "\n\t\t" << deviceInfo.GetDefaultGateway()
            << "\n\t\t" << deviceInfo.GetSubnetMask()
            << "\n\t\t" << deviceInfo.GetPortNr()
            << "\n\t\t" << deviceInfo.GetMacAddress()
            << "\n\t\t" << deviceInfo.GetInterface()
            << "\n\t\t" << deviceInfo.GetIpConfigOptions()
            << "\n\t\t" << deviceInfo.GetIpConfigCurrent()
            << "\n\t\t" << deviceInfo.GetDeviceGUID()
            << "\n\t\t" << deviceInfo.GetManufacturerInfo()
            << "\n\t\t" << deviceInfo.GetDeviceIdx()
            << "\n\t\t" << deviceInfo.GetProductId()
            << "\n\t\t" << deviceInfo.GetVendorId()
            << "\n\t\t" << deviceInfo.GetDriverKeyName()
            << "\n\t\t" << deviceInfo.GetUsbDriverType()
            << "\n\t\t" << deviceInfo.GetTransferMode()
            << "\n\t\t" << deviceInfo.GetInternalName()
            << "\n\t\t" << deviceInfo.GetBconAdapterLibraryName()
            << "\n\t\t" << deviceInfo.GetBconAdapterLibraryVersion()
            << "\n\t\t" << deviceInfo.GetBconAdapterLibraryApiVersion()
            << "\n\t\t" << deviceInfo.GetSupportedBconAdapterApiVersion()
            << "\n\t\t" << deviceInfo.GetPortID()
            << "\n\t\t" << deviceInfo.GetDeviceID()
            << "\n\t\t" << deviceInfo.GetInitialBaudRate()
            << "\n\t\t" << deviceInfo.GetDeviceXMLFileOverride()
            << "\n\t\t" << deviceInfo.GetDeviceSpecificString()
            << "\n\t\t" << deviceInfo.GetPortSpecificString()
            << std::endl;
         _dataStreams.emplace_back(std::make_unique<DataStream::Impl>(Pylon::CTlFactory::GetInstance().CreateDevice(deviceInfo)));
      }
      return _dataStreams;
   }

private:
   std::vector<DataStream> _dataStreams;
};

BaslerCamera::~BaslerCamera() = default;
BaslerCamera::BaslerCamera(BaslerCamera&&) noexcept = default;
BaslerCamera& BaslerCamera::operator=(BaslerCamera&&) noexcept = default;

BaslerCamera::BaslerCamera()
   : _pImpl{std::make_unique<Impl>()}
{
}

auto BaslerCamera::GetAvailableCameras() -> std::vector<BaslerCamera::DataStream>&
{
   return _pImpl->GetAvailableCameras();
}

BaslerCamera::DataStream::~DataStream() = default;
BaslerCamera::DataStream::DataStream(BaslerCamera::DataStream&&) noexcept = default;
BaslerCamera::DataStream& BaslerCamera::DataStream::operator=(BaslerCamera::DataStream&&) noexcept = default;

BaslerCamera::DataStream::DataStream(std::unique_ptr<Impl>&& pImpl)
   : _pImpl{std::move(pImpl)}
{
}

void BaslerCamera::DataStream::StartCamera()
{
   _pImpl->StartCamera();
}

void BaslerCamera::DataStream::StopCamera()
{
   _pImpl->StopCamera();
}

bool BaslerCamera::DataStream::GetFrame(cv::Mat& frame)
{
   return _pImpl->GetFrame(frame);
}

void BaslerCamera::DataStream::SetExposureTime(uint64_t exposureTime)
{
   _pImpl->SetExposureTime(exposureTime);
}

void BaslerCamera::DataStream::SetGain(uint32_t gain)
{
   _pImpl->SetGain(gain);
}

#if 0
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
#endif