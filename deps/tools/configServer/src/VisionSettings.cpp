// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include "VisionSettings.h"

#include <fmt/format.h>
#include <wpi/MemoryBuffer.h>
#include <wpi/fs.h>
#include <wpi/json.h>
#include <wpi/raw_istream.h>
#include <wpi/raw_ostream.h>

#include "VisionStatus.h"

std::shared_ptr<VisionSettings> VisionSettings::GetInstance() {
  static auto inst = std::make_shared<VisionSettings>(private_init{});
  return inst;
}

void VisionSettings::Set(const wpi::json& data,
                         std::function<void(std::string_view)> onFail) {
  {
    // write file
    std::error_code ec;
    wpi::raw_fd_ostream os(FRC_JSON, ec, fs::F_Text);
    if (ec) {
      onFail("could not write " FRC_JSON);
      return;
    }
    data.dump(os, 4);
    os << '\n';
  }

  // terminate vision process so it reloads the file
  VisionStatus::GetInstance()->Terminate(onFail);

  UpdateStatus();
}

void VisionSettings::UpdateStatus() { status(GetStatusJson()); }

wpi::json VisionSettings::GetStatusJson() {
  std::error_code ec;
  std::unique_ptr<wpi::MemoryBuffer> fileBuffer =
      wpi::MemoryBuffer::GetFile(FRC_JSON, ec);

  if (fileBuffer == nullptr || ec) {
    fmt::print(stderr, "could not read {}\n", FRC_JSON);
    return wpi::json();
  }

  try {
    wpi::json j = {{"type", "visionSettings"},
                   {"settings", wpi::json::parse(fileBuffer->GetCharBuffer())}};
    return j;
  } catch (wpi::json::exception& e) {
    fmt::print(stderr, "could not parse {}\n", FRC_JSON);
    return wpi::json();
  }
}
