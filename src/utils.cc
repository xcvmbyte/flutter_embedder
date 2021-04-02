// Copyright 2013 The Flutter Authors. All rights reserved.
/*
 *  Copyright (C) 2020-2021 XCVMByte Ltd.
 *  All Rights Reserved.
 * 
 */
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "utils.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sstream>

namespace flutter {

static std::string GetExecutablePath() {
  char executable_path[1024] = {0};
  std::stringstream stream;
  stream << "/proc/" << getpid() << "/exe";
  auto path = stream.str();
  auto executable_path_size =
      ::readlink(path.c_str(), executable_path, sizeof(executable_path));
  if (executable_path_size <= 0) {
    return "";
  }
  return std::string{executable_path,
                     static_cast<size_t>(executable_path_size)};
}

std::string GetExecutableName() {
  auto path_string = GetExecutablePath();
  auto found = path_string.find_last_of('/');
  if (found == std::string::npos) {
    return "";
  }
  return path_string.substr(found + 1);
}

std::string GetExecutableDirectory() {
  auto path_string = GetExecutablePath();
  auto found = path_string.find_last_of('/');
  if (found == std::string::npos) {
    return "";
  }
  return path_string.substr(0, found + 1);
}

bool FileExistsAtPath(const std::string& path) {
  return ::access(path.c_str(), R_OK) == 0;
}

bool FlutterAssetBundleIsValid(const std::string& bundle_path) {
  if (!FileExistsAtPath(bundle_path)) {
    FLWAY_ERR << "Bundle directory does not exist." << std::endl;
    return false;
  }

  if (!FileExistsAtPath(bundle_path + std::string{"/kernel_blob.bin"})) {
    FLWAY_ERR << "Kernel blob does not exist." << std::endl;
    return false;
  }

  return true;
}

std::string JoinPaths(std::initializer_list<std::string> components) {
  std::stringstream stream;
  size_t i = 0;
  const size_t size = components.size();
  for (const auto& component : components) {
    i++;
    stream << component;
    if (i != size) {
#if OS_WIN
      stream << "\\";
#else   // OS_WIN
      stream << "/";
#endif  // OS_WIN
    }
  }
  return stream.str();
}

bool IsFile(const std::string& path) {
  struct stat buf;
  if (stat(path.c_str(), &buf) != 0) {
    return false;
  }

  return S_ISREG(buf.st_mode);
}

}  // namespace flutter
