#pragma once

#include <memory>

#include <opencv2/core/mat.hpp>

class BaslerCamera
{
private:
   class Impl;

public:
   class DataStream
   {
   private:
      class Impl;

   public:
      DataStream(std::unique_ptr<Impl>&& pImpl);
      ~DataStream();
      DataStream(DataStream&&) noexcept;
      DataStream& operator=(DataStream&&) noexcept;

      void StartCamera();
      void StopCamera();
      bool GetFrame(cv::Mat& frame);
      auto GetVendor() -> std::string { return "Balser"; }
      auto GetSerialNumber() -> std::string { static auto fakeNumber = 123456; return std::to_string(fakeNumber); }
      void SetExposureTime(uint64_t exposure);
      void SetGain(uint32_t gain);

   private:
      std::unique_ptr<Impl> _pImpl;
      friend BaslerCamera::Impl;
   };

public:
  BaslerCamera();
  ~BaslerCamera();
  BaslerCamera(BaslerCamera&&) noexcept;
  BaslerCamera& operator=(BaslerCamera&&) noexcept;

  auto GetAvailableCameras() -> std::vector<DataStream>&;

private:
  std::unique_ptr<Impl> _pImpl;
};
