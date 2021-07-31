#pragma once

#include <memory>

#include <opencv2/core/mat.hpp>

namespace basler {
class DataStream;
class BaslerCamera
{
public:
  BaslerCamera();
  ~BaslerCamera();
  BaslerCamera(BaslerCamera&&) noexcept;
  BaslerCamera& operator=(BaslerCamera&&) noexcept;

  auto GetAvailableCameras() -> std::vector<DataStream>&;

public:
  class Impl;
private:
  std::unique_ptr<Impl> _pImpl;
};

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
  auto GetVendor() -> std::string;
  auto GetSerialNumber() -> std::string;
  void SetExposureTime(uint64_t exposure);
  void SetGain(uint32_t gain);
  auto GetGain() -> uint32_t;
  auto GetExposureTime() -> uint64_t;
  auto GetDeviceTemperature() -> float;

private:
  std::unique_ptr<Impl> _pImpl;
  friend BaslerCamera::Impl;
};
} /// end namespace basler
