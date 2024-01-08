// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#ifndef RPICONFIGSERVER_VISIONSTATUS_H_
#define RPICONFIGSERVER_VISIONSTATUS_H_

#include <functional>
#include <memory>
#include <string_view>
#include <vector>

#include <cscore.h>
#include <wpi/Signal.h>
#include <wpi/json_fwd.h>
#include <wpinet/uv/Loop.h>

namespace wpi::uv {
class Buffer;
}  // namespace wpi::uv

class VisionStatus {
  struct private_init {};

 public:
  explicit VisionStatus(const private_init&) {}
  VisionStatus(const VisionStatus&) = delete;
  VisionStatus& operator=(const VisionStatus&) = delete;

  void SetLoop(std::shared_ptr<wpi::uv::Loop> loop);

  void Up(std::function<void(std::string_view)> onFail);
  void Down(std::function<void(std::string_view)> onFail);
  void Terminate(std::function<void(std::string_view)> onFail);
  void Kill(std::function<void(std::string_view)> onFail);

  void UpdateStatus();
  void ConsoleLog(wpi::uv::Buffer& buf, size_t len);
  void UpdateCameraList();

  wpi::sig::Signal<const wpi::json&> update;
  wpi::sig::Signal<const wpi::json&> log;
  wpi::sig::Signal<const wpi::json&> cameraList;

  static std::shared_ptr<VisionStatus> GetInstance();

 private:
  void RunSvc(const char* cmd, std::function<void(std::string_view)> onFail);
  void RefreshCameraList();

  std::shared_ptr<wpi::uv::Loop> m_loop;

  struct CameraInfo {
    cs::UsbCameraInfo info;
    std::vector<cs::VideoMode> modes;
  };
  std::vector<CameraInfo> m_cameraInfo;
};

#endif  // RPICONFIGSERVER_VISIONSTATUS_H_
