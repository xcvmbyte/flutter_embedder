// Copyright 2013 The Flutter Authors. All rights reserved.
/*
 *  Copyright (C) 2020-2021 XCVMByte Ltd.
 *  All Rights Reserved.
 * 
 */
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EMBEDDER_WAYLAND_DISPLAY_H_
#define EMBEDDER_WAYLAND_DISPLAY_H_

#include <memory>
#include <string>
#include <queue>

#include <EGL/egl.h>
#define  EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#define  GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>

#include <wayland-client.h>
#include <wayland-egl.h>

#include "flutter_application.h"
#include "macros.h"

namespace flutter {

enum pointer_event_mask {
       POINTER_EVENT_ENTER = 1 << 0,
       POINTER_EVENT_LEAVE = 1 << 1,
       POINTER_EVENT_MOTION = 1 << 2,
       POINTER_EVENT_BUTTON = 1 << 3,
       POINTER_EVENT_AXIS = 1 << 4,
       POINTER_EVENT_AXIS_SOURCE = 1 << 5,
       POINTER_EVENT_AXIS_STOP = 1 << 6,
       POINTER_EVENT_AXIS_DISCRETE = 1 << 7,
};

struct pointer_event {
       uint32_t event_mask;
       wl_fixed_t surface_x, surface_y;
       uint32_t button, state;
       uint32_t time;
       uint32_t serial;
       struct {
               bool valid;
               wl_fixed_t value;
               int32_t discrete;
       } axes[2];
       uint32_t axis_source;
};

enum touch_event_mask {
       TOUCH_EVENT_DOWN = 1 << 0,
       TOUCH_EVENT_UP = 1 << 1,
       TOUCH_EVENT_MOTION = 1 << 2,
       TOUCH_EVENT_CANCEL = 1 << 3,
       TOUCH_EVENT_SHAPE = 1 << 4,
       TOUCH_EVENT_ORIENTATION = 1 << 5,
};

struct touch_point {
       bool valid;
       int32_t id;
       uint32_t event_mask;
       wl_fixed_t surface_x, surface_y;
       wl_fixed_t major, minor;
       wl_fixed_t orientation;
};

struct touch_event {
       uint32_t event_mask;
       uint32_t time;
       uint32_t serial;
       struct touch_point points[10];
};

class WaylandDisplay : public FlutterApplication::RenderDelegate {
 public:
  WaylandDisplay(size_t width, size_t height);

  ~WaylandDisplay();

  bool IsValid() const;

  bool Run();

  // For handling touch events
  static const struct wl_touch_listener wl_touch_listener;
  static const struct wl_seat_listener my_wl_seat_listener;
  static void handle_wl_seat_name(void* data,
                                  struct wl_seat* wl_seat,
                                  const char* name);
  static struct touch_point* get_touch_point(WaylandDisplay *myWayland, int32_t id);
  static void wl_touch_down(void* data,
                            struct wl_touch* wl_touch,
                            uint32_t serial,
                            uint32_t time,
                            struct wl_surface* surface,
                            int32_t id,
                            wl_fixed_t x,
                            wl_fixed_t y);
  static void wl_touch_up(void* data,
                          struct wl_touch* wl_touch,
                          uint32_t serial,
                          uint32_t time,
                          int32_t id);
  static void wl_touch_motion(void* data,
                              struct wl_touch* wl_touch,
                              uint32_t time,
                              int32_t id,
                              wl_fixed_t x,
                              wl_fixed_t y);
  static void wl_touch_cancel(void* data, struct wl_touch* wl_touch);
  static void wl_touch_shape(void* data,
                             struct wl_touch* wl_touch,
                             int32_t id,
                             wl_fixed_t major,
                             wl_fixed_t minor);
  static void wl_touch_orientation(void* data,
                                   struct wl_touch* wl_touch,
                                   int32_t id,
                                   wl_fixed_t orientation);
  static void wl_touch_frame(void* data, struct wl_touch* wl_touch);


  static void display_handle_mode(void* data,
                                  struct wl_output* wl_output,
                                  uint32_t flags,
                                  int width,
                                  int height,
                                  int refresh);
  static void display_handle_geometry(void* data,
                                      struct wl_output* wl_output,
                                      int x,
                                      int y,
                                      int physical_width,
                                      int physical_height,
                                      int subpixel,
                                      const char* make,
                                      const char* model,
                                      int transform);

  // keyboard events
  struct xkb_state* xkb_state;
  struct xkb_context* xkb_context;
  struct xkb_keymap* xkb_keymap;

  static const struct wl_keyboard_listener wl_keyboard_listener;
  static void
      wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
              uint32_t format, int32_t fd, uint32_t size);

  static void
      wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
              uint32_t serial, struct wl_surface *surface,
              struct wl_array *keys);

  static void
      wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
              uint32_t serial, uint32_t time, uint32_t key, uint32_t state);

  static void
      wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
              uint32_t serial, struct wl_surface *surface);
  static void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
          int32_t rate, int32_t delay);
  static void
      wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
              uint32_t serial, uint32_t mods_depressed,
              uint32_t mods_latched, uint32_t mods_locked,
              uint32_t group);

 private:
  static const wl_registry_listener kRegistryListener;
  static const wl_shell_surface_listener kShellSurfaceListener;
  static const wl_surface_listener kSurfaceListener;
  static const wl_display_listener kDisplayListener;
  static const struct wl_output_listener output_listener;

  bool valid_ = false;
  const int screen_width_;
  const int screen_height_;
  wl_display* display_ = nullptr;
  wl_registry* registry_ = nullptr;
  wl_compositor* compositor_ = nullptr;
  wl_shell* shell_ = nullptr;
  wl_output * output_ = nullptr;
  wl_shell_surface* shell_surface_ = nullptr;
  wl_surface* compositor_surface_ = nullptr;
  wl_egl_window* window_ = nullptr;
  EGLDisplay egl_display_ = EGL_NO_DISPLAY;
  EGLSurface egl_surface_ = nullptr;
  EGLContext egl_root_context_ = EGL_NO_CONTEXT;
  EGLContext egl_render_context_ = EGL_NO_CONTEXT;
  EGLContext egl_uploading_context_ = EGL_NO_CONTEXT;
  char *gl_renderer_;
  char *gl_exts_;

  // input handling
  wl_seat * seat_ = nullptr;
  struct xdg_surface *xdg_surface = nullptr;
  struct wl_keyboard *wl_keyboard = nullptr;
  struct wl_pointer *wl_pointer = nullptr;
  struct wl_touch *wl_touch = nullptr;
  float offset;
  uint32_t last_frame;
  int width, height;
  bool closed;
  struct pointer_event pointer_event={0};
  struct touch_event touch_event={0};

  class CompareFlutterTask {
   public:
    bool operator()(std::pair<uint64_t, FlutterTask> n1,
                    std::pair<uint64_t, FlutterTask> n2) {
      return n1.first > n2.first;
    }
  };
  std::priority_queue<std::pair<uint64_t, FlutterTask>,
                      std::vector<std::pair<uint64_t, FlutterTask>>,
                      CompareFlutterTask> 
                      TaskRunner;

  bool SetupEGL();

  void AnnounceRegistryInterface(struct wl_registry* wl_registry,
                                 uint32_t name,
                                 const char* interface,
                                 uint32_t version);

  void UnannounceRegistryInterface(struct wl_registry* wl_registry,
                                   uint32_t name);

  bool StopRunning();

  // |flutter::FlutterApplication::RenderDelegate|
  bool OnApplicationContextMakeCurrent() override;

  // |flutter::FlutterApplication::RenderDelegate|
  bool OnApplicationContextClearCurrent() override;

  // |flutter::FlutterApplication::RenderDelegate|
  bool OnApplicationPresent() override;

  // |flutter::FlutterApplication::RenderDelegate|
  uint32_t OnApplicationGetOnscreenFBO() override;

  // |flutter::FlutterApplication::RenderDelegate|
  bool OnApplicationMakeResourceCurrent() override;

  void OnApplicationGetTaskrunner(FlutterTask task, uint64_t target_time) override;

  FLWAY_DISALLOW_COPY_AND_ASSIGN(WaylandDisplay);

  static void handle_wl_seat_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities);

};

}  // namespace flutter

#endif  // EMBEDDER_WAYLAND_DISPLAY_H_
