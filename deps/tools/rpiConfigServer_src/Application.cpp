/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "Application.h"

#include <sys/stat.h>
#include <unistd.h>

#include <wpi/FileSystem.h>
#include <wpi/json.h>
#include <wpi/raw_istream.h>
#include <wpi/raw_ostream.h>

#include "VisionStatus.h"

#define TYPE_TAG "### TYPE:"

std::shared_ptr<Application> Application::GetInstance() {
  static auto inst = std::make_shared<Application>(private_init{});
  return inst;
}

void Application::Set(wpi::StringRef appType,
                      std::function<void(wpi::StringRef)> onFail) {
  wpi::StringRef appDir;
  wpi::StringRef appCommand;

  if (appType == "builtin") {
    appCommand = "./multiCameraServer";
  } else if (appType == "example-java") {
    appDir = "java-multiCameraServer";
    appCommand =
        "env LD_LIBRARY_PATH=/usr/local/frc/lib java -jar "
        "build/libs/java-multiCameraServer-all.jar";
  } else if (appType == "example-cpp") {
    appDir = "cpp-multiCameraServer";
    appCommand = "./multiCameraServerExample";
  } else if (appType == "example-python") {
    appDir = "python-multiCameraServer";
    appCommand = "./multiCameraServer.py";
  } else if (appType == "upload-java") {
    appCommand =
        "env LD_LIBRARY_PATH=/usr/local/frc/lib java -jar uploaded.jar";
  } else if (appType == "upload-cpp") {
    appCommand = "./uploaded";
  } else if (appType == "upload-python") {
    appCommand = "./uploaded.py";
  } else if (appType == "custom") {
    return;
  } else {
    wpi::SmallString<64> msg;
    msg = "unrecognized application type '";
    msg += appType;
    msg += "'";
    onFail(msg);
    return;
  }

  {
    // write file
    std::error_code ec;
    wpi::raw_fd_ostream os(EXEC_HOME "/runCamera", ec, wpi::sys::fs::F_Text);
    if (ec) {
      onFail("could not write " EXEC_HOME "/runCamera");
      return;
    }
    os << "#!/bin/sh\n";
    os << TYPE_TAG << ' ' << appType << '\n';
    os << "echo \"Waiting 5 seconds...\"\n";
    os << "sleep 5\n";
    if (!appDir.empty()) os << "cd " << appDir << '\n';
    os << "exec " << appCommand << '\n';
  }

  m_appType = appType;

  // terminate vision process so it reloads
  VisionStatus::GetInstance()->Terminate(onFail);

  UpdateStatus();
}

void Application::Upload(wpi::ArrayRef<uint8_t> contents,
                         std::function<void(wpi::StringRef)> onFail) {
  wpi::StringRef filename;
  if (m_appType == "upload-java") {
    filename = "/uploaded.jar";
  } else if (m_appType == "upload-cpp") {
    filename = "/uploaded";
  } else if (m_appType == "upload-python") {
    filename = "/uploaded.py";
  } else {
    wpi::SmallString<64> msg;
    msg = "cannot upload application type '";
    msg += m_appType;
    msg += "'";
    onFail(msg);
    return;
  }

  wpi::SmallString<64> pathname;
  pathname = EXEC_HOME;
  pathname += filename;

  // remove old file (need to do this as we can't overwrite a running exe)
  if (unlink(pathname.c_str()) == -1) {
    wpi::errs() << "could not remove app executable: " << std::strerror(errno)
                << '\n';
  }

  {
    // open file for writing
    std::error_code ec;
    int fd;
    if (wpi::sys::fs::openFileForWrite(pathname, fd, wpi::sys::fs::F_None)) {
      wpi::SmallString<64> msg;
      msg = "could not write ";
      msg += pathname;
      onFail(msg);
      return;
    }

    // change ownership
    if (fchown(fd, APP_UID, APP_GID) == -1) {
      wpi::errs() << "could not change app ownership: " << std::strerror(errno)
                  << '\n';
    }

    // set file to be executable
    if (fchmod(fd, 0775) == -1) {
      wpi::errs() << "could not change app permissions: "
                  << std::strerror(errno) << '\n';
    }

    // write contents and close file
    wpi::raw_fd_ostream(fd, true) << contents;
  }

  // terminate vision process so it reloads
  VisionStatus::GetInstance()->Terminate(onFail);
}

void Application::UpdateStatus() { status(GetStatusJson()); }

wpi::json Application::GetStatusJson() {
  wpi::json j = {{"type", "applicationSettings"},
                 {"applicationType", "custom"}};

  std::error_code ec;
  wpi::raw_fd_istream is(EXEC_HOME "/runCamera", ec);
  if (ec) {
    wpi::errs() << "could not read " EXEC_HOME "/runCamera\n";
    return j;
  }

  // scan file
  wpi::SmallString<256> lineBuf;
  while (!is.has_error()) {
    wpi::StringRef line = is.getline(lineBuf, 256).trim();
    if (line.startswith(TYPE_TAG)) {
      j["applicationType"] = line.substr(strlen(TYPE_TAG)).trim();
      break;
    }
  }

  return j;
}
