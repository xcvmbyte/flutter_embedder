// Copyright 2013 The Flutter Authors. All rights reserved.
/*
 *  Copyright (C) 2020-2021 XCVMByte Ltd.
 *  All Rights Reserved.
 * 
 */
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#pragma once

#include <flutter/standard_method_codec.h>
#include <flutter_embedder.h>

#include <functional>
#include <map>

namespace flutter {

class PlatformChannel {
 public:
  PlatformChannel();
  void PlatformMessageCallback(const FlutterPlatformMessage* message);
  void SetEngine(FlutterEngine engine);

 private:
  FlutterEngine engine_;
  std::map<std::string, std::function<void(const FlutterPlatformMessage*)>>
      platform_message_handlers_;
      
  void OnAccessibilityChannelPlatformMessage(const FlutterPlatformMessage*);
  void OnFlutterPlatformChannelPlatformMessage(const FlutterPlatformMessage*);
  void OnFlutterTextInputChannelPlatformMessage(const FlutterPlatformMessage*);
  void OnFlutterPlatformViewsChannelPlatformMessage(
      const FlutterPlatformMessage*);

  void OnFlutterPluginIoUrlLauncher(const FlutterPlatformMessage*);
  void OnFlutterPluginConnectivity(const FlutterPlatformMessage*);
  void OnFlutterPluginConnectivityStatus(const FlutterPlatformMessage*);
  void OnFlutterPluginIoVideoPlayer(const FlutterPlatformMessage*);
  void OnFlutterPluginIoVideoPlayerEvents(const FlutterPlatformMessage*);
};

}  // namespace flutter