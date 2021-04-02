// Copyright 2013 The Flutter Authors. All rights reserved.
/*
 *  Copyright (C) 2020-2021 XCVMByte Ltd.
 *  All Rights Reserved.
 * 
 */
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EMBEDDER_FLUTTER_UTILS_H_
#define EMBEDDER_FLUTTER_UTILS_H_

#include "macros.h"

namespace flutter {

std::string GetExecutableName();

std::string GetExecutableDirectory();

bool FileExistsAtPath(const std::string& path);

bool FlutterAssetBundleIsValid(const std::string& bundle_path);

std::string JoinPaths(std::initializer_list<std::string> components);

bool IsFile(const std::string& path);

}  // namespace flutter

#endif  // EMBEDDER_FLUTTER_UTILS_H_