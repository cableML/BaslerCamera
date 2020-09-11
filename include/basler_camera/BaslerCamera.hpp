#pragma once

#include <memory>

class BaslerCamera
{
public:
  BaslerCamera();
  ~BaslerCamera();
  BaslerCamera(BaslerCamera&&) noexcept;
  BaslerCamera& operator=(BaslerCamera&&) noexcept;

private:
  class Impl;
  std::unique_ptr<Impl> _pImpl;
};
