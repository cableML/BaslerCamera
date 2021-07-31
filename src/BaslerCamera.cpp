#include <basler_camera/BaslerCamera.hpp>

#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>

#include <pylon/PylonIncludes.h>

#include <iostream>

using namespace basler;

class DataStream::Impl
{
public:
   Impl(Pylon::IPylonDevice* pylonDevice)
      : _instantCamera{pylonDevice}
   {
      std::cout << "Using device " << _instantCamera.GetDeviceInfo().GetModelName() << std::endl;

      // The parameter MaxNumBuffer can be used to control the count of buffers
      // allocated for grabbing. The default value of this parameter is 10.
      _instantCamera.MaxNumBuffer = 5;
      _instantCamera.Open();
       auto& nodeMap = _instantCamera.GetNodeMap();
       Pylon::CEnumParameter pixelFormat(nodeMap, "PixelFormat");
       std::cout << "pixelFormat: " << pixelFormat.GetValue() << std::endl;
       if (pixelFormat.CanSetValue("RGB8"))
       {
           pixelFormat.SetValue("BGR8");
       }
       _instantCamera.Close();
   }

   ~Impl()
   {
       std::cout << "DataStream::Impl::~Impl" << std::endl;
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
      auto& nodemap = _instantCamera.GetNodeMap();
      Pylon::CFloatParameter exposureTimeParam(nodemap, "ExposureTime");
      exposureTimeParam.SetValue(exposureTime);
   }

   void SetGain(uint32_t gain)
   {
       try
       {
           auto &nodemap = _instantCamera.GetNodeMap();
           if (_instantCamera.GetSfncVersion() >= Pylon::Sfnc_2_0_0)
           {
               Pylon::CFloatParameter gainParam(nodemap, "Gain");
               gainParam.SetValue(static_cast<double>(gain));
           }
           else
           {
               Pylon::CIntegerParameter gainRaw(nodemap, "GainRaw");
               gainRaw.SetValue(gain);
           }
       }
       catch (const Pylon::GenericException &e)
       {
           std::cout << "Error: "  << e.GetDescription() << std::endl;
       }
   }

   auto GetGain() -> uint32_t
   {
       uint32_t result{};
       try
       {
           auto &nodemap = _instantCamera.GetNodeMap();
           if (_instantCamera.GetSfncVersion() >= Pylon::Sfnc_2_0_0)
           {
               Pylon::CFloatParameter gainParam(nodemap, "Gain");
               result = gainParam.GetValue();
           }
           else
           {
               Pylon::CIntegerParameter gainRaw(nodemap, "GainRaw");
               result = gainRaw.GetValue();
           }
       }
       catch (const Pylon::GenericException &e)
       {
           std::cout << "Error: "  << e.GetDescription() << std::endl;
       }
   }

   auto GetExposureTime() -> uint64_t
   {
       auto& nodemap = _instantCamera.GetNodeMap();
       Pylon::CFloatParameter exposureTimeParam(nodemap, "ExposureTime");
       return exposureTimeParam.GetValue();
   }

   auto GetVendor() -> std::string
   {
       return _instantCamera.GetDeviceInfo().GetVendorName().c_str();
   }

   auto GetSerialNumber() -> std::string
   {
       return _instantCamera.GetDeviceInfo().GetSerialNumber().c_str();
   }

   auto GetDeviceTemperature() -> float
   {
       auto& nodemap = _instantCamera.GetNodeMap();
       Pylon::CFloatParameter deviceTemperatureParam(nodemap, "DeviceTemperature");
       return deviceTemperatureParam.GetValue();
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
      std::cout << "BaslerCamera::Impl::~Impl" << std::endl;
      _dataStreams.clear();
      Pylon::PylonTerminate();
   }

   auto GetAvailableCameras() -> std::vector<DataStream>&
   {
#if 0
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
#endif
      Pylon::DeviceInfoList_t deviceInfoList;
      auto countDevices = Pylon::CTlFactory::GetInstance().EnumerateDevices(deviceInfoList);
      for (auto& deviceInfo : deviceInfoList)
      {
#if 0
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
#endif
         _dataStreams.emplace_back(std::make_unique<DataStream::Impl>(Pylon::CTlFactory::GetInstance().CreateDevice(deviceInfo)));
      }
      return _dataStreams;
   }

private:
   std::vector<DataStream> _dataStreams;
};

BaslerCamera::~BaslerCamera()
{
    std::cout << "BaslerCamera::~BaslerCamera" << std::endl;
}

BaslerCamera::BaslerCamera(BaslerCamera&&) noexcept = default;
BaslerCamera& BaslerCamera::operator=(BaslerCamera&&) noexcept = default;

BaslerCamera::BaslerCamera()
   : _pImpl{std::make_unique<Impl>()}
{
}

auto BaslerCamera::GetAvailableCameras() -> std::vector<DataStream>&
{
   return _pImpl->GetAvailableCameras();
}

DataStream::~DataStream() = default;
DataStream::DataStream(DataStream&&) noexcept = default;
DataStream& DataStream::operator=(DataStream&&) noexcept = default;

DataStream::DataStream(std::unique_ptr<Impl>&& pImpl)
   : _pImpl{std::move(pImpl)}
{
}

void DataStream::StartCamera()
{
   _pImpl->StartCamera();
}

void DataStream::StopCamera()
{
   _pImpl->StopCamera();
}

bool DataStream::GetFrame(cv::Mat& frame)
{
   return _pImpl->GetFrame(frame);
}

void DataStream::SetExposureTime(uint64_t exposureTime)
{
   _pImpl->SetExposureTime(exposureTime);
}

void DataStream::SetGain(uint32_t gain)
{
   _pImpl->SetGain(gain);
}

auto DataStream::GetVendor() -> std::string
{
    return _pImpl->GetVendor();
}

auto DataStream::GetSerialNumber() -> std::string
{
    return _pImpl->GetSerialNumber();
}

auto DataStream::GetDeviceTemperature() -> float
{
    return _pImpl->GetDeviceTemperature();
}

auto basler::DataStream::GetGain() -> uint32_t
{
    return _pImpl->GetGain();
}

auto basler::DataStream::GetExposureTime() -> uint64_t
{
    return _pImpl->GetExposureTime();
}