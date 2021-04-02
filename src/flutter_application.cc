// Copyright 2013 The Flutter Authors. All rights reserved.
/*
 *  Copyright (C) 2020-2021 XCVMByte Ltd.
 *  All Rights Reserved.
 * 
 */
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include <chrono>
#include <sstream>
#include <vector>

#include <string.h>
#include <unistd.h>
#include <EGL/egl.h>
#include <sys/types.h>
//#include <include/core/SkBitmap.h>

#include "flutter_application.h"
#include "log.h"
#include "utils.h"
#include "input_hook.h"

namespace flutter {

static_assert(FLUTTER_ENGINE_VERSION == 1, "");

static const char* kICUDataFileName = "icudtl.dat";

FlutterEngine FlutterApplication::engine_ = nullptr;

static std::string GetICUDataPath() {
  std::string icu_path1 = (std::string)"/usr/lib/"+kICUDataFileName;

  if (FileExistsAtPath(icu_path1.c_str())) {
    return icu_path1;
  } else {
    FLWAY_ERR << "Could not find ICUData of flutter engine:" << icu_path1 << std::endl;
    FLWAY_ERR << "  Now try to search in working directory" << std::endl;
  }

  auto exe_dir = GetExecutableDirectory();
  if (exe_dir == "") {
    return "";
  }
  std::stringstream stream;
  stream << exe_dir << kICUDataFileName;

  auto icu_path = stream.str();

  if (!FileExistsAtPath(icu_path.c_str())) {
    FLWAY_ERR << "Could not find " << icu_path << std::endl;
    return "";
  }

  return icu_path;
}

FlutterApplication::FlutterApplication(
    std::string bundle_path,
    //const std::vector<std::string>& command_line_args,
    RenderDelegate& render_delegate)
    : render_delegate_(render_delegate) {
  if (!FlutterAssetBundleIsValid(bundle_path)) {
    FLWAY_ERR << "Flutter asset bundle was not valid." << std::endl;
    return;
  }

  FLWAY_LOG << "Flutter asset bundle is valid. " << std::endl;

  FlutterRendererConfig config = {};

#define USE_OPENGL_RENDERER 1
#ifdef USE_OPENGL_RENDERER
  config.type = kOpenGL;
  config.open_gl.struct_size = sizeof(config.open_gl);
  config.open_gl.make_current = [](void* userdata) -> bool {
    return reinterpret_cast<FlutterApplication*>(userdata)
        ->render_delegate_.OnApplicationContextMakeCurrent();
  };
  FLWAY_LOG << "register OnApplicationContextMakeCurrent() " << std::endl;

  config.open_gl.clear_current = [](void* userdata) -> bool {
    return reinterpret_cast<FlutterApplication*>(userdata)
        ->render_delegate_.OnApplicationContextClearCurrent();
  };
  FLWAY_LOG << "register OnApplicationContextClearCurrent() " << std::endl;

  config.open_gl.present = [](void* userdata) -> bool {
    return reinterpret_cast<FlutterApplication*>(userdata)
        ->render_delegate_.OnApplicationPresent();
  };
  FLWAY_LOG << "register OnApplicationPresent() " << std::endl;

  config.open_gl.fbo_callback = [](void* userdata) -> uint32_t {
    return reinterpret_cast<FlutterApplication*>(userdata)
        ->render_delegate_.OnApplicationGetOnscreenFBO();
  };
  FLWAY_LOG << "register OnApplicationGetOnscreenFBO() " << std::endl;
  config.open_gl.make_resource_current = [](void * userdata) -> bool {
    return reinterpret_cast<FlutterApplication*>(userdata)
        ->render_delegate_.OnApplicationMakeResourceCurrent();
  };

  config.open_gl.gl_proc_resolver = [](void* userdata,
                                       const char* name) -> void* {
    auto address = eglGetProcAddress(name);
    if (address != nullptr) {
      return reinterpret_cast<void*>(address);
    }
    FLWAY_ERR << "Tried unsuccessfully to resolve: " << name << std::endl;
    return nullptr;
  };
  FLWAY_LOG << "register eglGetProcAddress() " << std::endl;

  config.open_gl.surface_transformation = NULL; //on_view_to_display_transformation;
  config.open_gl.gl_external_texture_frame_callback = NULL;
#else
  // Software rendering using skia
  config.type = kSoftware;
  config.software.struct_size = sizeof(config.software);
  config.software.surface_present_callback = 
        [](void* context, const void* allocation, size_t row_bytes,
         size_t height) {
        // auto image_info =
        //     SkImageInfo::MakeN32Premul(SkISize::Make(row_bytes / 4, height));
        // SkBitmap bitmap;
        // if (!bitmap.installPixels(image_info, const_cast<void*>(allocation),
        //                           row_bytes)) {
        //   FLWAY_ERR << "Could not copy pixels for the software "
        //                     "composition from the engine.";
        //   return false;
        // }
        // bitmap.setImmutable();
        return true;
        //reinterpret_cast<EmbedderTestContextSoftware*>(context)->Present(
        //    SkImage::MakeFromBitmap(bitmap));
      };

#endif

  auto icu_data_path = GetICUDataPath();

  if (icu_data_path == "") {
    FLWAY_ERR << "Could not find ICU data. It should be placed next to the "
                   "executable but it wasn't there."
                << std::endl;
    return;
  }
  FLWAY_LOG << "icu data path:" << icu_data_path << std::endl;

  FlutterProjectArgs project_args;
  memset(&project_args, 0, sizeof(project_args));
  project_args.struct_size = sizeof(FlutterProjectArgs);
  project_args.assets_path = bundle_path.c_str();
  project_args.icu_data_path = icu_data_path.c_str();
  //platform channel callback
  project_args.platform_message_callback = [](const FlutterPlatformMessage* message,
                                      void* context) {
    reinterpret_cast<FlutterApplication*>(context)
        ->platform_channel_.PlatformMessageCallback(message);
  };

  // Configure task runner interop
  FlutterTaskRunnerDescription platform_task_runner = {};
  platform_task_runner.struct_size = sizeof(FlutterTaskRunnerDescription);
  platform_task_runner.user_data = this;
  platform_task_runner.runs_task_on_current_thread_callback =
      [](void* context) -> bool { return true; };
  platform_task_runner.post_task_callback =
      [](FlutterTask task, uint64_t target_time, void* context) -> void {
    reinterpret_cast<FlutterApplication*>(context)
    ->render_delegate_.OnApplicationGetTaskrunner(task,target_time);
  };

  FlutterCustomTaskRunners custom_task_runners = {};
  custom_task_runners.struct_size = sizeof(FlutterCustomTaskRunners);
  custom_task_runners.platform_task_runner = &platform_task_runner;
  project_args.custom_task_runners = &custom_task_runners;
  
  FLWAY_LOG << "Prepare to run flutter engine..." << std::endl;

  FlutterEngine engine = nullptr;
  auto result = FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config, &project_args,
                                 this /* userdata */, &engine_);

  if (result != kSuccess) {
    FLWAY_ERR << "Could not run the Flutter engine" << std::endl;
    return;
  }
  platform_channel_.SetEngine(engine_);
  LOG_INFO("  Started engine:0x%lx\n\n", engine_);

  valid_ = true;
}

FlutterApplication::~FlutterApplication() {
  if (engine_ == nullptr) {
    return;
  }

  auto result = FlutterEngineShutdown(engine_);

  if (result != kSuccess) {
    FLWAY_ERR << "Could not shutdown the Flutter engine." << std::endl;
  }
}

bool FlutterApplication::IsValid() const {
  return valid_;
}

bool FlutterApplication::SetWindowSize(size_t width, size_t height) {
  LOG_INFO("Set windows metrics event: (%d,%d) on engine:0x%lx\n", width, height, engine_);

  FlutterWindowMetricsEvent event = {};
  event.struct_size = sizeof(event);
  event.width = width;
  event.height = height;
  event.pixel_ratio = 1.0;
  return FlutterEngineSendWindowMetricsEvent(engine_, &event) == kSuccess;
}

FlutterEngineResult FlutterApplication::SendInputEventToFlutter(FlutterPointerEvent* inputEvents,
                                            int count) {
  FlutterEngineResult result = kInternalInconsistency;
  
  LOG_INFO("  Pointer events collected %d. Will send to flutter engine...\n\n", count);
  FlutterEngine myEngine = engine_;
  if (myEngine != NULL) {
    result = FlutterEngineSendPointerEvent(myEngine, inputEvents, count);
  } else {
    LOG_ERROR(stderr,
            "    Warning:Flutter engine is NULL. Input event ingored!\n");
  }
  
  if (result != kSuccess) {
    LOG_ERROR(stderr,
            "[flutter-rk] Failed to send input event to flutter engine. "
            "FlutterEngineSendPointerEvent: %s\n",
            FLUTTER_RESULT_TO_STRING(result));
  }
  return result;
}

FlutterEngineResult FlutterApplication::FlutterSendMessage(const char *channel, const uint8_t *message, const size_t message_size) {
  FlutterEngineResult message_result = kInternalInconsistency;
  FlutterEngine myEngine = engine_;
  FlutterPlatformMessageResponseHandle *response_handle = nullptr;

  FlutterPlatformMessage platform_message = {
      .struct_size     = sizeof(FlutterPlatformMessage),
      .channel         = channel,
      .message         = message,
      .message_size    = message_size,
      .response_handle = response_handle,
  };

  message_result = FlutterEngineSendPlatformMessage(myEngine, &platform_message);

  if (response_handle != nullptr) {
    FlutterPlatformMessageReleaseResponseHandle(myEngine, response_handle);
  }
  return message_result;
}

FlutterEngineResult FlutterApplication::FlutterRunTask(const FlutterTask* task) {
  FlutterEngineResult message_result = kInternalInconsistency;
  message_result = FlutterEngineRunTask(engine_,task);
  return message_result;
}
}  // namespace flutter

