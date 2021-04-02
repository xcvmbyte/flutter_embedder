// Copyright 2013 The Flutter Authors. All rights reserved.
/*
 *  Copyright (C) 2020-2021 XCVMByte Ltd.
 *  All Rights Reserved.
 * 
 */
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WL_EGL_PLATFORM
#define WL_EGL_PLATFORM 1
#endif

#include "wayland_display.h"

#include <stdlib.h>
#include <unistd.h>

#include <cstring>

#include "log.h"

namespace flutter {

static int output_width, output_height;
void WaylandDisplay::display_handle_geometry (void *data,
			struct wl_output *wl_output,
			int x,
			int y,
			int physical_width,
			int physical_height,
			int subpixel,
			const char *make,
			const char *model,
      int transform){
  LOG_INFO("  output listener display_handle_geometry corner:(%d,%d) size:(%d %d) make:%s model:%s transform:%d\n",
    x, y, physical_width, physical_height, make, model, transform);
}

void WaylandDisplay::display_handle_mode (void *data,
		    struct wl_output *wl_output,
		    uint32_t flags,
		    int width,
		    int height,
		    int refresh) {
	output_width = width;
	output_height = height;
  LOG_INFO("  output listener display_handle_mode flags:0x%x size:(%d,%d)\n", flags, width, height);
}

const struct wl_output_listener WaylandDisplay::output_listener = {
	.geometry = display_handle_geometry,
	.mode = display_handle_mode,
  .done = [](void *data,
		     struct wl_output *wl_output) -> void {
    LOG_INFO("  output done callback\n");
  },
  .scale = [](void *data,
		      struct wl_output *wl_output,
		      int32_t factor) -> void {
    LOG_INFO ("  output scale callback factor:%d\n", factor);
  },
};

#define DISPLAY reinterpret_cast<WaylandDisplay*>(data)

const wl_registry_listener WaylandDisplay::kRegistryListener = {
    .global = [](void* data,
                 struct wl_registry* wl_registry,
                 uint32_t name,
                 const char* interface,
                 uint32_t version) -> void {
      LOG_INFO("Registry interface name id:%d interface:%s version:%x\n", name, interface, version);
      DISPLAY->AnnounceRegistryInterface(wl_registry, name, interface, version);
    },

    .global_remove =
        [](void* data, struct wl_registry* wl_registry, uint32_t name) -> void {
      DISPLAY->UnannounceRegistryInterface(wl_registry, name);
    },
};

const wl_surface_listener WaylandDisplay::kSurfaceListener = {
  .enter = [](void *data,
		      struct wl_surface *wl_surface,
		      struct wl_output *output){

      LOG_INFO("kSurfaceListener enter:\n");
      if (output != DISPLAY->output_){
        LOG_INFO("  Error: a different output object\n");
      }
  },
  .leave = [](void *data,
		      struct wl_surface *wl_surface,
		      struct wl_output *output){
      LOG_INFO("kSurfaceListener leave:\n");
  },
};

const wl_shell_surface_listener WaylandDisplay::kShellSurfaceListener = {
    .ping = [](void* data,
               struct wl_shell_surface* wl_shell_surface,
               uint32_t serial) -> void {
      wl_shell_surface_pong(DISPLAY->shell_surface_, serial);
    },

    .configure = [](void* data,
                    struct wl_shell_surface* wl_shell_surface,
                    uint32_t edges,
                    int32_t width,
                    int32_t height) -> void {
      FLWAY_ERR << "\n\nUnhandled resize." << std::endl;
    },

    .popup_done = [](void* data,
                     struct wl_shell_surface* wl_shell_surface) -> void {
      // Nothing to do.
    },
};

const wl_display_listener WaylandDisplay::kDisplayListener = {
    .error = [](void *data,
		      struct wl_display *wl_display,
		      void *object_id,
		      uint32_t code,
		      const char *message) -> void {
      LOG_INFO("  wl_display_listener --error callback-- obj:0x%x code:%d message:%s\n", object_id, code, message);
    },
    .delete_id = [](void *data,
			  struct wl_display *wl_display,
			  uint32_t id) -> void {
      LOG_INFO(" wl_display_listener delete_id callback obj id:0x%x\n", id);
    }
};

WaylandDisplay::WaylandDisplay(size_t width, size_t height)
    : screen_width_(width), screen_height_(height) {
  
  // clean member data structures before we do anything real
  for (int i=0; i < sizeof(touch_event.points)/sizeof(touch_point); i++){
      touch_event.points[i].id = i;
      touch_event.points[i].valid = false;
  }
  
  if (screen_width_ == 0 || screen_height_ == 0) {
    FLWAY_ERR << "Invalid screen dimensions." << std::endl;
    return;
  }
  FLWAY_LOG << " Screen dimensions: " + screen_width_ <<  ","  << screen_height_  << std::endl;

  display_ = wl_display_connect(nullptr);

  if (!display_) {
    FLWAY_ERR << "Could not connect to the wayland display." << std::endl;
    return;
  }
  FLWAY_LOG << "wl_display connected:" << std::endl;

  wl_display_add_listener(display_, &kDisplayListener, this);
	// wl_display_add_global_listener(display, handle_global, &screenshooter);
	// wl_display_iterate(display, WL_DISPLAY_READABLE);
	// wl_display_roundtrip(display);

  registry_ = wl_display_get_registry(display_);
  if (!registry_) {
    FLWAY_ERR << "Could not get the wayland registry." << std::endl;
    return;
  }

  // xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

  wl_registry_add_listener(registry_, &kRegistryListener, this);

  wl_display_roundtrip(display_);

  if (!SetupEGL()) {
    FLWAY_ERR << "Could not setup EGL." << std::endl;
    return;
  }
  FLWAY_LOG << "EGL setup OK" << std::endl;

  valid_ = true;
}

WaylandDisplay::~WaylandDisplay() {
  // TODO: Not all member objects destroyed.
  if (shell_surface_) {
    wl_shell_surface_destroy(shell_surface_);
    shell_surface_ = nullptr;
  }

  if (shell_) {
    wl_shell_destroy(shell_);
    shell_ = nullptr;
  }

  if (egl_surface_) {
    eglDestroySurface(egl_display_, egl_surface_);
    egl_surface_ = nullptr;
  }

  if (egl_display_) {
    eglTerminate(egl_display_);
    egl_display_ = nullptr;
  }

  if (window_) {
    wl_egl_window_destroy(window_);
    window_ = nullptr;
  }

  if (compositor_surface_) {
    wl_surface_destroy(compositor_surface_);
    compositor_surface_ = nullptr;
  }

  if (compositor_) {
    wl_compositor_destroy(compositor_);
    compositor_ = nullptr;
  }

  if (registry_) {
    wl_registry_destroy(registry_);
    registry_ = nullptr;
  }

  if (display_) {
    wl_display_flush(display_);
    wl_display_disconnect(display_);
    display_ = nullptr;
  }
}

bool WaylandDisplay::IsValid() const {
  return valid_;
}

bool WaylandDisplay::Run() {
  if (!valid_) {
    FLWAY_ERR << "Could not run an invalid display." << std::endl;
    return false;
  }

  while (valid_) {
    wl_display_dispatch(display_);
    if (!TaskRunner.empty()) {
      uint64_t current = FlutterEngineGetCurrentTime();
      if (current >= TaskRunner.top().first) {
        auto item = TaskRunner.top();
        TaskRunner.pop();
        FlutterApplication::FlutterRunTask(&item.second);
      }
    }
  }

  return true;
}

static void LogLastEGLError() {
  struct EGLNameErrorPair {
    const char* name;
    EGLint code;
  };

#define _EGL_ERROR_DESC(a) \
  { #a, a }

  const EGLNameErrorPair pairs[] = {
      _EGL_ERROR_DESC(EGL_SUCCESS),
      _EGL_ERROR_DESC(EGL_NOT_INITIALIZED),
      _EGL_ERROR_DESC(EGL_BAD_ACCESS),
      _EGL_ERROR_DESC(EGL_BAD_ALLOC),
      _EGL_ERROR_DESC(EGL_BAD_ATTRIBUTE),
      _EGL_ERROR_DESC(EGL_BAD_CONTEXT),
      _EGL_ERROR_DESC(EGL_BAD_CONFIG),
      _EGL_ERROR_DESC(EGL_BAD_CURRENT_SURFACE),
      _EGL_ERROR_DESC(EGL_BAD_DISPLAY),
      _EGL_ERROR_DESC(EGL_BAD_SURFACE),
      _EGL_ERROR_DESC(EGL_BAD_MATCH),
      _EGL_ERROR_DESC(EGL_BAD_PARAMETER),
      _EGL_ERROR_DESC(EGL_BAD_NATIVE_PIXMAP),
      _EGL_ERROR_DESC(EGL_BAD_NATIVE_WINDOW),
      _EGL_ERROR_DESC(EGL_CONTEXT_LOST),
  };

#undef _EGL_ERROR_DESC

  const auto count = sizeof(pairs) / sizeof(EGLNameErrorPair);

  EGLint last_error = eglGetError();

  for (size_t i = 0; i < count; i++) {
    if (last_error == pairs[i].code) {
      FLWAY_ERR << "EGL Error: " << pairs[i].name << " (" << pairs[i].code
                  << ")" << std::endl;
      return;
    }
  }

  FLWAY_ERR << "Unknown EGL Error" << std::endl;
}

bool WaylandDisplay::SetupEGL() {
  if (!compositor_ || !shell_ || !output_) {
    FLWAY_ERR << "EGL setup needs: compositor / shell / output connection."
                << std::endl;
    return false;
  }
  wl_output_add_listener(output_, &output_listener, this);

  compositor_surface_ = wl_compositor_create_surface(compositor_);

  if (!compositor_surface_) {
    FLWAY_ERR << "Could not create compositor surface." << std::endl;
    return false;
  }

  wl_surface_add_listener(compositor_surface_, &kSurfaceListener, this);
  
  shell_surface_ = wl_shell_get_shell_surface(shell_, compositor_surface_);

  if (!shell_surface_) {
    FLWAY_ERR << "Could not shell surface." << std::endl;
    return false;
  }

  wl_shell_surface_add_listener(shell_surface_, &kShellSurfaceListener, this);

  wl_shell_surface_set_title(shell_surface_, "Flutter");

  wl_shell_surface_set_toplevel(shell_surface_);

  window_ = wl_egl_window_create(compositor_surface_, screen_width_, screen_height_);

  if (!window_) {
    FLWAY_ERR << "Could not create EGL window." << std::endl;
    return false;
  }

  if (eglBindAPI(EGL_OPENGL_ES_API) != EGL_TRUE) {
    LogLastEGLError();
    FLWAY_ERR << "Could not bind the ES API." << std::endl;
    return false;
  }

  egl_display_ = eglGetDisplay(display_);
  if (egl_display_ == EGL_NO_DISPLAY) {
    LogLastEGLError();
    FLWAY_ERR << "Could not access EGL display." << std::endl;
    return false;
  }

  if (eglInitialize(egl_display_, nullptr, nullptr) != EGL_TRUE) {
    LogLastEGLError();
    FLWAY_ERR << "Could not initialize EGL display." << std::endl;
    return false;
  }

  EGLConfig egl_config = nullptr;

  // Choose an EGL config to use for the surface and context.
  {
    EGLint attribs[] = {
        // clang-format off
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
      EGL_RED_SIZE,        8,
      EGL_GREEN_SIZE,      8,
      EGL_BLUE_SIZE,       8,
      EGL_ALPHA_SIZE,      8,
      EGL_DEPTH_SIZE,      0,
      EGL_STENCIL_SIZE,    0,
      EGL_NONE,            // termination sentinel
        // clang-format on
    };

    EGLint config_count = 0;

    if (eglChooseConfig(egl_display_, attribs, &egl_config, 1, &config_count) !=
        EGL_TRUE) {
      LogLastEGLError();
      FLWAY_ERR << "Error when attempting to choose an EGL surface config."
                  << std::endl;
      return false;
    }

    if (config_count == 0 || egl_config == nullptr) {
      LogLastEGLError();
      FLWAY_ERR << "No matching configs." << std::endl;
      return false;
    }
  }

  // Create an EGL window surface with the matched config.
  {
    const EGLint attribs[] = {EGL_NONE};

    egl_surface_ = eglCreateWindowSurface(egl_display_, egl_config, window_, attribs);

    if (egl_surface_ == EGL_NO_SURFACE) {
      LogLastEGLError();
      FLWAY_ERR << "EGL surface was null during surface selection."
                  << std::endl;
      return false;
    }
  }

  EGLint egl_error;

  // Create an EGL context with the match config.
  {
    const EGLint attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

    egl_root_context_ = eglCreateContext(egl_display_, egl_config, EGL_NO_CONTEXT, attribs);
    if ((egl_error = eglGetError()) != EGL_SUCCESS) {
      LOG_ERROR(stderr, " Could not create OpenGL ES root context. eglCreateContext: 0x%08X\n", egl_error);
      return false;
    }
    egl_render_context_ = eglCreateContext(egl_display_, egl_config, egl_root_context_, attribs);
    if ((egl_error = eglGetError()) != EGL_SUCCESS) {
      LOG_ERROR(stderr, " Could not create OpenGL ES renderer context. eglCreateContext: 0x%08X\n", egl_error);
      return false;
    }
    egl_uploading_context_ = eglCreateContext(egl_display_, egl_config, egl_root_context_, attribs);
    if ((egl_error = eglGetError()) != EGL_SUCCESS) {
      LOG_ERROR(stderr, " Could not create OpenGL ES resource uploading context. eglCreateContext: 0x%08X\n", egl_error);
      return false;
    }

  }

  wl_surface_commit(compositor_surface_);

  eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_root_context_);
	if ((egl_error = eglGetError()) != EGL_SUCCESS) {
		LOG_ERROR(stderr, "[flutter-rk] Could not make OpenGL ES root context current to get OpenGL information. eglMakeCurrent: 0x%08X\n", egl_error);
		return EIO;
	}

	gl_renderer_ = (char*) glGetString(GL_RENDERER);

	gl_exts_ = (char*) glGetString(GL_EXTENSIONS);
	LOG_INFO("OpenGL ES information:\n");
	LOG_INFO("  version: \"%s\"\n", glGetString(GL_VERSION));
	LOG_INFO("  shading language version: \"%s\"\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	LOG_INFO("  vendor: \"%s\"\n", glGetString(GL_VENDOR));
	LOG_INFO("  renderer: \"%s\"\n", gl_renderer_);
	LOG_INFO("  extensions: \"%s\"\n", gl_exts_);
	LOG_INFO("===================================\n");

	eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	if ((egl_error = eglGetError()) != EGL_SUCCESS) {
		LOG_ERROR(stderr, "Could not clear OpenGL ES context. eglMakeCurrent: 0x%08X\n", egl_error);
		return EIO;
	}


  return true;
}

void WaylandDisplay::AnnounceRegistryInterface(struct wl_registry* wl_registry,
                                               uint32_t name,
                                               const char* interface_name,
                                               uint32_t version) {
  if (strcmp(interface_name, "wl_compositor") == 0) {
    LOG_INFO("  wl_compositor object found\n");
    compositor_ = static_cast<decltype(compositor_)>(
        wl_registry_bind(wl_registry, name, &wl_compositor_interface, 1));
    return;
  }

  if (strcmp(interface_name, "wl_shell") == 0) {
    LOG_INFO("  wl_shell object found\n");
    shell_ = static_cast<decltype(shell_)>(
        wl_registry_bind(wl_registry, name, &wl_shell_interface, 1));
    return;
  }

  if (strcmp(interface_name, "wl_output") == 0){
    LOG_INFO("  wl_output object found\n");
    output_ = static_cast<decltype(output_)>(
        wl_registry_bind(wl_registry, name, &wl_output_interface, version));
    return;
  }

  if (strcmp(interface_name, "wl_seat") == 0){
    LOG_INFO("  wl_seat object found\n");
    seat_ = static_cast<decltype(seat_)>(
        wl_registry_bind(wl_registry, name, &wl_seat_interface, version));
    wl_seat_add_listener(seat_, &my_wl_seat_listener, this);
    return;
  }

}

void WaylandDisplay::UnannounceRegistryInterface(
    struct wl_registry* wl_registry,
    uint32_t name) {}

// |flutter::FlutterApplication::RenderDelegate|
bool WaylandDisplay::OnApplicationContextMakeCurrent() {
  // FLWAY_LOG << "Entering OnApplicationContextMakeCurrent" << std::endl;

  if (!valid_) {
    FLWAY_ERR << "Invalid display." << std::endl;
    return false;
  }

  if (eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_render_context_) !=
      EGL_TRUE) {
    LogLastEGLError();
    FLWAY_ERR << "Could not make the onscreen context current" << std::endl;
    return false;
  }

  // FLWAY_LOG << "eglMakeCurrent OK" << std::endl;
  return true;
}

// |flutter::FlutterApplication::RenderDelegate|
bool WaylandDisplay::OnApplicationContextClearCurrent() {
  FLWAY_LOG << "Entering OnApplicationContextClearCurrent" << std::endl;

  if (!valid_) {
    FLWAY_ERR << "Invalid display." << std::endl;
    return false;
  }

  if (eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE,
                     EGL_NO_CONTEXT) != EGL_TRUE) {
    LogLastEGLError();
    FLWAY_ERR << "Could not clear the context." << std::endl;
    return false;
  }

  return true;
}

// |flutter::FlutterApplication::RenderDelegate|
bool WaylandDisplay::OnApplicationPresent() {
  // FLWAY_LOG << "Entering OnApplicationPresent" << std::endl;

  if (!valid_) {
    FLWAY_ERR << "Invalid display." << std::endl;
    return false;
  }

  if (eglSwapBuffers(egl_display_, egl_surface_) != EGL_TRUE) {
    LogLastEGLError();
    FLWAY_ERR << "Could not swap the EGL buffer." << std::endl;
    return false;
  }

  return true;
}

// |flutter::FlutterApplication::RenderDelegate|
uint32_t WaylandDisplay::OnApplicationGetOnscreenFBO() {
  FLWAY_LOG << "Entering OnApplicationGetOnscreenFBO" << std::endl;

  if (!valid_) {
    FLWAY_ERR << "Invalid display." << std::endl;
    return 999;
  }

  return 0;  // FBO0
}

bool WaylandDisplay::OnApplicationMakeResourceCurrent() {
  FLWAY_LOG << "Entering OnApplicationMakeResourceCurrent" << std::endl;

  if (!valid_) {
    FLWAY_ERR << "Invalid display." << std::endl;
    return false;
  }

  if (eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, egl_uploading_context_) != EGL_TRUE) {
    LogLastEGLError();
    FLWAY_ERR << "Could not make OnApplicationMakeResourceCurrent" << std::endl;
    return false;
  }

  return true;
}

void WaylandDisplay::OnApplicationGetTaskrunner(FlutterTask task, uint64_t target_time) {
  TaskRunner.push(std::make_pair(target_time, task));
}

}  // namespace flutter
