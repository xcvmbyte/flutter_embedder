// Copyright 2013 The Flutter Authors. All rights reserved.
/*
 *  Copyright (C) 2020-2021 XCVMByte Ltd.
 *  All Rights Reserved.
 * 
 */
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EMBEDDER_FLUTTER_APPLICATION_H_
#define EMBEDDER_FLUTTER_APPLICATION_H_

#include <functional>
#include <vector>

#include <flutter_embedder.h>

#include "macros.h"
#include "platform_channel.h"

namespace flutter {

class FlutterApplication {
 public:
  class RenderDelegate {
   public:
    virtual bool OnApplicationContextMakeCurrent() = 0;

    virtual bool OnApplicationContextClearCurrent() = 0;

    virtual bool OnApplicationPresent() = 0;

    virtual uint32_t OnApplicationGetOnscreenFBO() = 0;

    virtual bool OnApplicationMakeResourceCurrent() = 0;

    virtual void OnApplicationGetTaskrunner(FlutterTask task, uint64_t target_time) = 0;
  };

  FlutterApplication(std::string bundle_path,
                    //  const std::vector<std::string>& args,
                     RenderDelegate& render_delegate);

  ~FlutterApplication();

  bool IsValid() const;

  bool SetWindowSize(size_t width, size_t height);

  static FlutterEngineResult SendInputEventToFlutter(FlutterPointerEvent* inputEvents,int count);
  static FlutterEngineResult FlutterSendMessage(const char *channel, const uint8_t *message, const size_t message_size);
  static FlutterEngineResult FlutterRunTask(const FlutterTask* task);
  static inline FlutterEngine GetFlutterEngine()   {return engine_;}
 private:
  bool valid_;
  RenderDelegate& render_delegate_;
  int last_button_ = 0;
  PlatformChannel platform_channel_;
  static FlutterEngine engine_;  
  
  bool SendFlutterPointerEvent(FlutterPointerPhase phase, double x, double y);

  FLWAY_DISALLOW_COPY_AND_ASSIGN(FlutterApplication);
};

}  // namespace flutter

#endif  // EMBEDDER_FLUTTER_APPLICATION_H_