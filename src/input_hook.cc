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
#include <getopt.h>
#include <assert.h>
#include <sys/mman.h>
#include <wayland-util.h>
#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "log.h"
#include "input_hook.h"

namespace flutter {

void
WaylandDisplay::wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
               uint32_t format, int32_t fd, uint32_t size)
{
  LOG_INFO("[wl_keyboard] wl_keyboard_keymap\n");

  // WaylandDisplay* myWayland = reinterpret_cast<WaylandDisplay*>(data);
  // assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);

  // char* map_shm = (char *)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  // assert(map_shm != MAP_FAILED);

  // struct xkb_keymap* xkb_keymap = xkb_keymap_new_from_string(
  //     myWayland->xkb_context, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1,
  //     XKB_KEYMAP_COMPILE_NO_FLAGS);
  // munmap(map_shm, size);
  // close(fd);

  // struct xkb_state* xkb_state = xkb_state_new(xkb_keymap);
  // xkb_keymap_unref(myWayland->xkb_keymap);
  // xkb_state_unref(myWayland->xkb_state);
  // myWayland->xkb_keymap = xkb_keymap;
  // myWayland->xkb_state = xkb_state;
}

void
WaylandDisplay::wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
               uint32_t serial, struct wl_surface *surface,
               struct wl_array *keys)
{
  LOG_INFO("[wl_keyboard] wl_keyboard_enter\n");
  // WaylandDisplay* myWayland = reinterpret_cast<WaylandDisplay*>(data);
  // LOG_ERROR(stderr, "keyboard enter; keys pressed are:\n");
  // uint32_t* key = (uint32_t*)keys->data;

  // for (int i=0; i<keys->size; i++) {
  //   char buf[128];
  //   xkb_keysym_t sym =
  //       xkb_state_key_get_one_sym(myWayland->xkb_state, key[i] + 8);
  //   xkb_keysym_get_name(sym, buf, sizeof(buf));
  //   LOG_ERROR(stderr, "sym: %-12s (%d), ", buf, sym);
  //   xkb_state_key_get_utf8(myWayland->xkb_state, key[i] + 8, buf, sizeof(buf));
  //   LOG_ERROR(stderr, "utf8: '%s'\n", buf);
  // }
}

void
WaylandDisplay::wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
               uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    LOG_ERROR(stderr, "[wl_keyboard] wl_keyboard_key serial:%d keycode:0x%08x state:%d\n", serial, key, state);

        static constexpr char kChannelName[] = "flutter/keyevent";  //flutter默认channelname

        static constexpr char kKeyCodeKey[] = "keyCode";
        static constexpr char kKeyMapKey[] = "keymap";
        static constexpr char kLinuxKeyMap[] = "linux";
        static constexpr char kScanCodeKey[] = "scanCode";
        static constexpr char kModifiersKey[] = "modifiers";
        static constexpr char kTypeKey[] = "type";
        static constexpr char kToolkitKey[] = "toolkit";
        static constexpr char kGLFWKey[] = "glfw";
        static constexpr char kUnicodeScalarValues[] = "unicodeScalarValues";
        static constexpr char kKeyUp[] = "keyup";
        static constexpr char kKeyDown[] = "keydown";

        rapidjson::Document document;
        auto& allocator = document.GetAllocator();
        document.SetObject();
        document.AddMember(kKeyCodeKey, key, allocator);
        document.AddMember(kKeyMapKey, kLinuxKeyMap, allocator);
        document.AddMember(kScanCodeKey, key + 8, allocator);
        document.AddMember(kModifiersKey, 0, allocator);
        document.AddMember(kToolkitKey, kGLFWKey, allocator);
        // document.AddMember(kUnicodeScalarValues, utf32, allocator);

        switch (state) {
          case WL_KEYBOARD_KEY_STATE_PRESSED:
            document.AddMember(kTypeKey, kKeyDown, allocator);
            break;
          case WL_KEYBOARD_KEY_STATE_RELEASED:
            document.AddMember(kTypeKey, kKeyUp, allocator);
            break;
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        std::cout << buffer.GetString() << std::endl;

        FlutterPlatformMessage message = {};
        message.struct_size = sizeof(FlutterPlatformMessage);
        message.channel = kChannelName;
        message.message = reinterpret_cast<const uint8_t*>(buffer.GetString());
        message.message_size = buffer.GetSize();
        message.response_handle = nullptr;

        auto result = FlutterApplication::FlutterSendMessage(kChannelName, reinterpret_cast<const uint8_t*>(buffer.GetString()),buffer.GetSize());
        if (result != kSuccess)
          FLWAY_LOG << "FlutterEngineSendPlatformMessage Result: " << result
                    << std::endl;
  // WaylandDisplay* myWayland = reinterpret_cast<WaylandDisplay*>(data);
  // char buf[128];
  // uint32_t keycode = key + 8;
  // xkb_keysym_t sym = xkb_state_key_get_one_sym(myWayland->xkb_state, keycode);
  // xkb_keysym_get_name(sym, buf, sizeof(buf));
  // const char* action =
  //     state == WL_KEYBOARD_KEY_STATE_PRESSED ? "press" : "release";
  // LOG_ERROR(stderr, "key %s: sym: %-12s (%d), ", action, buf, sym);
  // xkb_state_key_get_utf8(myWayland->xkb_state, keycode, buf, sizeof(buf));
  // LOG_ERROR(stderr, "utf8: '%s'\n", buf);
}

void
WaylandDisplay::wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
               uint32_t serial, struct wl_surface *surface)
{
  LOG_ERROR(stderr, "keyboard leave\n");
}

void
WaylandDisplay::wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
               uint32_t serial, uint32_t mods_depressed,
               uint32_t mods_latched, uint32_t mods_locked,
               uint32_t group)
{
  WaylandDisplay* myWayland = reinterpret_cast<WaylandDisplay*>(data);
  // xkb_state_update_mask(myWayland->xkb_state, mods_depressed, mods_latched,
  //                       mods_locked, 0, 0, group);
  LOG_ERROR(stderr, "[wl_keyboard] wl_keyboard_modifiers\n");
}

void
WaylandDisplay::wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
               int32_t rate, int32_t delay)
{
  LOG_ERROR(stderr, "[wl_keyboard] wl_keyboard_repeat_info\n");
       /* Left as an exercise for the reader */
}

const struct wl_keyboard_listener WaylandDisplay::wl_keyboard_listener = {
       .keymap = WaylandDisplay::wl_keyboard_keymap,
       .enter = WaylandDisplay::wl_keyboard_enter,
       .leave = WaylandDisplay::wl_keyboard_leave,
       .key = WaylandDisplay::wl_keyboard_key,
       .modifiers = WaylandDisplay::wl_keyboard_modifiers,
       .repeat_info = WaylandDisplay::wl_keyboard_repeat_info,
};

void
WaylandDisplay::handle_wl_seat_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities)
{
  LOG_INFO("    Handling wayland seat: handle_wl_seat_capabilities\n");

    WaylandDisplay *myWayland = reinterpret_cast<WaylandDisplay*>(data);
    /* TODO */
    bool have_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;

    if (have_pointer && myWayland->wl_pointer == NULL) {
      myWayland->wl_pointer = wl_seat_get_pointer(myWayland->seat_);
      LOG_INFO("[wl_seat] wl_seat_get_pointer not registered\n");
      // wl_pointer_add_listener(myWayland->wl_pointer, &wl_pointer_listener, myWayland);
    } else if (!have_pointer && myWayland->wl_pointer != NULL) {
      wl_pointer_release(myWayland->wl_pointer);
      myWayland->wl_pointer = NULL;
    }

    bool have_touch = capabilities & WL_SEAT_CAPABILITY_TOUCH;

    if (have_touch && myWayland->wl_touch == NULL) {
      myWayland->wl_touch = wl_seat_get_touch(myWayland->seat_);
      wl_touch_add_listener(myWayland->wl_touch, &wl_touch_listener, myWayland);
      LOG_INFO("[wl_seat] register wl_touch_listener\n");
    } else if (!have_touch && myWayland->wl_touch != NULL) {
      wl_touch_release(myWayland->wl_touch);
      myWayland->wl_touch = NULL;
    }

    bool have_keyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;

    if (have_keyboard && myWayland->wl_keyboard == NULL) {
      myWayland->wl_keyboard = wl_seat_get_keyboard(myWayland->seat_);
      if (myWayland->wl_keyboard != NULL) {
        LOG_INFO("[wl_seat] register wl_keyboard_listener\n");
        wl_keyboard_add_listener(myWayland->wl_keyboard, &wl_keyboard_listener,
                                 myWayland);
      } else {
        LOG_ERROR(stderr, "Can't get wl_keyboard object\n");
      }
    } else if (!have_keyboard && myWayland->wl_keyboard != NULL) {
      wl_keyboard_release(myWayland->wl_keyboard);
      myWayland->wl_keyboard = NULL;
    }
}

void
WaylandDisplay::handle_wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name)
{
    LOG_ERROR(stderr, "seat name: %s\n", name);
}

const struct wl_seat_listener WaylandDisplay::my_wl_seat_listener = {
    .capabilities = handle_wl_seat_capabilities,
    .name = handle_wl_seat_name,
};

const struct wl_touch_listener WaylandDisplay::wl_touch_listener = {
    .down = WaylandDisplay::wl_touch_down,
    .up = WaylandDisplay::wl_touch_up,
    .motion = WaylandDisplay::wl_touch_motion,
    .frame = WaylandDisplay::wl_touch_frame,
    .cancel = WaylandDisplay::wl_touch_cancel,
    .shape = WaylandDisplay::wl_touch_shape,
    .orientation = WaylandDisplay::wl_touch_orientation,
};

struct touch_point* WaylandDisplay::get_touch_point(WaylandDisplay* myWayland,
                                                    int32_t id) {
  struct touch_event* touch = &myWayland->touch_event;
  const size_t nmemb = sizeof(touch->points) / sizeof(struct touch_point);
  int invalid = -1;
  for (size_t i = 0; i < nmemb; ++i) {
    if (touch->points[i].id == id) {
        // LOG_ERROR(stderr, "    Found existing active point #%d\n", id);
      touch->points[i].valid = true;
      return &touch->points[i];
    }
    if (invalid == -1 && !touch->points[i].valid) {
      invalid = i;
    }
  }
  if (invalid == -1) {
    return NULL;
  }
  LOG_ERROR(stderr, "    Use touch point #%d for id#%d\n", invalid, id);
  touch->points[invalid].valid = true;
  touch->points[invalid].id = id;
  return &touch->points[invalid];
}

void WaylandDisplay::wl_touch_down(void* data,
                                   struct wl_touch* wl_touch,
                                   uint32_t serial,
                                   uint32_t time,
                                   struct wl_surface* surface,
                                   int32_t id,
                                   wl_fixed_t x,
                                   wl_fixed_t y) {
  LOG_ERROR(stderr, "[wl_touch] wl_touch_down (%lf, %lf) serial:%d id:%d time:%d\n",
          wl_fixed_to_double(x), wl_fixed_to_double(y), serial, id, time);
  WaylandDisplay* myWayland = reinterpret_cast<WaylandDisplay*>(data);
  struct touch_point* point = get_touch_point(myWayland, id);
  if (point == NULL) {
    return;
  }
  // LOG_ERROR(stderr, "    Set initial value for point #%d\n", id);
  point->event_mask = TOUCH_EVENT_DOWN;
  point->surface_x = wl_fixed_to_double(x),
  point->surface_y = wl_fixed_to_double(y);
  myWayland->touch_event.time = time;
  myWayland->touch_event.serial = serial;

  FlutterPointerEvent singlePointerEvent =
      (FlutterPointerEvent){.struct_size = sizeof(FlutterPointerEvent),
                            .phase = kDown,
                            .timestamp = time,
                            .x = (double)point->surface_x,
                            .y = (double)point->surface_y,
                            .device = 2,
                            .signal_kind = kFlutterPointerSignalKindNone,
                            .scroll_delta_x = 0.0,
                            .scroll_delta_y = 0.0,
                            .device_kind = kFlutterPointerDeviceKindTouch,
                            .buttons = 0};
  FlutterApplication::SendInputEventToFlutter(&singlePointerEvent, 1);
}

void WaylandDisplay::wl_touch_up(void* data,
                                 struct wl_touch* wl_touch,
                                 uint32_t serial,
                                 uint32_t time,
                                 int32_t id) {
  WaylandDisplay* myWayland = reinterpret_cast<WaylandDisplay*>(data);
  struct touch_point* point = get_touch_point(myWayland, id);
  if (point == NULL) {
    return;
  }
  point->event_mask |= TOUCH_EVENT_UP;
  // LOG_ERROR(stderr, "[wl_touch] wl_touch_up serial:%d id:%d time:%d existing pos:(%d, %d)\n", serial, id, time point->surface_x, point->surface_y);

  FlutterPointerEvent singlePointerEvent =
      (FlutterPointerEvent){.struct_size = sizeof(FlutterPointerEvent),
                            .phase = kUp,
                            .timestamp = time,
                            .x = (double)point->surface_x,
                            .y = (double)point->surface_y,
                            .device = 2,
                            .signal_kind = kFlutterPointerSignalKindNone,
                            .scroll_delta_x = 0.0,
                            .scroll_delta_y = 0.0,
                            .device_kind = kFlutterPointerDeviceKindTouch,
                            .buttons = 0};
  FlutterApplication::SendInputEventToFlutter(&singlePointerEvent, 1);
}

void WaylandDisplay::wl_touch_motion(void* data,
                                     struct wl_touch* wl_touch,
                                     uint32_t time,
                                     int32_t id,
                                     wl_fixed_t x,
                                     wl_fixed_t y) {
  // LOG_ERROR(stderr, "[wl_touch] wl_touch_motion pos:(%lf,%lf) id:%d time:%d\n",
  //         wl_fixed_to_double(x), wl_fixed_to_double(y), id, time);
  WaylandDisplay* myWayland = reinterpret_cast<WaylandDisplay*>(data);
  struct touch_point* point = get_touch_point(myWayland, id);
  if (point == NULL) {
    return;
  }
  point->event_mask |= TOUCH_EVENT_MOTION;
  point->surface_x = wl_fixed_to_double(x), point->surface_y = wl_fixed_to_double(y);
  myWayland->touch_event.time = time;

  FlutterPointerEvent singlePointerEvent =
      (FlutterPointerEvent){.struct_size = sizeof(FlutterPointerEvent),
                            .phase = kMove,
                            .timestamp = time,
                            .x = (double)point->surface_x,
                            .y = (double)point->surface_y,
                            .device = 2,
                            .signal_kind = kFlutterPointerSignalKindNone,
                            .scroll_delta_x = 0.0,
                            .scroll_delta_y = 0.0,
                            .device_kind = kFlutterPointerDeviceKindTouch,
                            .buttons = 0};
  FlutterApplication::SendInputEventToFlutter(&singlePointerEvent, 1);
}

void WaylandDisplay::wl_touch_cancel(void* data, struct wl_touch* wl_touch) {
  WaylandDisplay* myWayland = reinterpret_cast<WaylandDisplay*>(data);
  myWayland->touch_event.event_mask |= TOUCH_EVENT_CANCEL;
}

void WaylandDisplay::wl_touch_shape(void* data,
                                    struct wl_touch* wl_touch,
                                    int32_t id,
                                    wl_fixed_t major,
                                    wl_fixed_t minor) {
  LOG_ERROR(stderr, "[wl_touch] wl_touch_shape major:%d,minor:%d id:%d\n", major,
          minor, id);
  WaylandDisplay* myWayland = reinterpret_cast<WaylandDisplay*>(data);
  struct touch_point* point = get_touch_point(myWayland, id);
  if (point == NULL) {
    return;
  }
  point->event_mask |= TOUCH_EVENT_SHAPE;
  point->major = major, point->minor = minor;
}

void WaylandDisplay::wl_touch_orientation(void* data,
                                          struct wl_touch* wl_touch,
                                          int32_t id,
                                          wl_fixed_t orientation) {
  WaylandDisplay* myWayland = reinterpret_cast<WaylandDisplay*>(data);
  struct touch_point* point = get_touch_point(myWayland, id);
  if (point == NULL) {
    return;
  }
  point->event_mask |= TOUCH_EVENT_ORIENTATION;
  point->orientation = orientation;
}

void WaylandDisplay::wl_touch_frame(void* data, struct wl_touch* wl_touch) {
  WaylandDisplay* myWayland = reinterpret_cast<WaylandDisplay*>(data);
  struct touch_event* touch = &myWayland->touch_event;
  const size_t nmemb = sizeof(touch->points) / sizeof(struct touch_point);
  FlutterPointerPhase kPhase;
  FlutterPointerEvent pointerEvents[64];
  int pointerEventCount=0;


  // LOG_ERROR(stderr, "[wl_touch] wl_touch_frame @ %d  \n", touch->time);

  pointerEventCount = 0;

  for (size_t i = 0; i < nmemb; ++i) {
    struct touch_point* point = &touch->points[i];
    if (!point->valid) {
      continue;
    }
    // LOG_ERROR(stderr, "[wl_touch]         point %d: mask:0x%x ", touch->points[i].id, touch->points[i].event_mask);

    if (point->event_mask &
        (TOUCH_EVENT_DOWN | TOUCH_EVENT_UP | TOUCH_EVENT_MOTION)) {
      if (point->event_mask & TOUCH_EVENT_DOWN) {
        // LOG_ERROR(stderr, "down pos:%d,%d ", point->surface_x, point->surface_y);
        kPhase = kDown;
      }

      if (point->event_mask & TOUCH_EVENT_UP) {
        // LOG_ERROR(stderr, " up pos:(%d,%d)  ", point->surface_x, point->surface_y);
        kPhase = kUp;
      }

      if (point->event_mask & TOUCH_EVENT_MOTION) {
        // LOG_ERROR(stderr, " motion pos:%d,%d ", point->surface_x, point->surface_y);
        kPhase = kMove;
      }

      // Add to FlutterPointerEvent
      pointerEvents[pointerEventCount++] = (FlutterPointerEvent) {
						.struct_size = sizeof(FlutterPointerEvent),
						.phase = kPhase,
						.timestamp = touch->time,
						.x = (double)point->surface_x,
						.y = (double)point->surface_y,
						.device = 2,
						.signal_kind = kFlutterPointerSignalKindNone,
						.scroll_delta_x = 0.0,
						.scroll_delta_y = 0.0,
						.device_kind = kFlutterPointerDeviceKindTouch,
						.buttons = 0
					};

    }

    if (point->event_mask & TOUCH_EVENT_SHAPE) {
      LOG_ERROR(stderr, " shape %fx%f ", wl_fixed_to_double(point->major),
              wl_fixed_to_double(point->minor));
    }

    if (point->event_mask & TOUCH_EVENT_ORIENTATION) {
      LOG_ERROR(stderr, " orientation %f ",
              wl_fixed_to_double(point->orientation));
    }

    point->valid = false;
    // LOG_ERROR(stderr, "\n");
  }

//   if (pointerEventCount > 0){
//       LOG_ERROR(stderr, "    pointer event count:%d. Will send to flutter.\n", pointerEventCount);
//     //   FlutterEngineResult result = FlutterApplication::SendInputEventToFlutter(pointerEvents, pointerEventCount);
//   }
}

}  // namespace flutter

