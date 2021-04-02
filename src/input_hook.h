// Copyright 2013 The Flutter Authors. All rights reserved.
/*
 *  Copyright (C) 2020-2021 XCVMByte Ltd.
 *  All Rights Reserved.
 * 
 */
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef EMBEDDER_FLUTTER_INPUTHOOK_H_
#define EMBEDDER_FLUTTER_INPUTHOOK_H_


#include <limits.h>
#include <linux/input.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include <stdio.h> 
#include <glob.h>

#include <libinput.h>

#include <EGL/egl.h>
//#include <epoxy/egl.h>
//#define  EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
//#define  GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>
#include <flutter_embedder.h>

enum device_orientation {
	kPortraitUp, kLandscapeLeft, kPortraitDown, kLandscapeRight
};

#define ANGLE_FROM_ORIENTATION(o) \
	((o) == kPortraitUp ? 0 : \
	 (o) == kLandscapeLeft ? 90 : \
	 (o) == kPortraitDown ? 180 : \
	 (o) == kLandscapeRight ? 270 : 0)
	
#define ANGLE_BETWEEN_ORIENTATIONS(o_start, o_end) \
	(ANGLE_FROM_ORIENTATION(o_end) \
	- ANGLE_FROM_ORIENTATION(o_start) \
	+ (ANGLE_FROM_ORIENTATION(o_start) > ANGLE_FROM_ORIENTATION(o_end) ? 360 : 0))

#define FLUTTER_TRANSLATION_TRANSFORMATION(translate_x, translate_y) ((FlutterTransformation) \
	{.scaleX = 1, .skewX  = 0, .transX = translate_x, \
	 .skewY  = 0, .scaleY = 1, .transY = translate_y, \
	 .pers0  = 0, .pers1  = 0, .pers2  = 1})

#define FLUTTER_ROTX_TRANSFORMATION(deg) ((FlutterTransformation) \
	{.scaleX = 1, .skewX  = 0,                                .transX = 0, \
	 .skewY  = 0, .scaleY = cos(((double) (deg))/180.0*M_PI), .transY = -sin(((double) (deg))/180.0*M_PI), \
	 .pers0  = 0, .pers1  = sin(((double) (deg))/180.0*M_PI), .pers2  = cos(((double) (deg))/180.0*M_PI)})

#define FLUTTER_ROTY_TRANSFORMATION(deg) ((FlutterTransformation) \
	{.scaleX = cos(((double) (deg))/180.0*M_PI),  .skewX  = 0, .transX = sin(((double) (deg))/180.0*M_PI), \
	 .skewY  = 0,                                 .scaleY = 1, .transY = 0, \
	 .pers0  = -sin(((double) (deg))/180.0*M_PI), .pers1  = 0, .pers2  = cos(((double) (deg))/180.0*M_PI)})

#define FLUTTER_ROTZ_TRANSFORMATION(deg) ((FlutterTransformation) \
	{.scaleX = cos(((double) (deg))/180.0*M_PI), .skewX  = -sin(((double) (deg))/180.0*M_PI), .transX = 0, \
	 .skewY  = sin(((double) (deg))/180.0*M_PI), .scaleY = cos(((double) (deg))/180.0*M_PI),  .transY = 0, \
	 .pers0  = 0,                                .pers1  = 0,                                 .pers2  = 1})

#define FLUTTER_MULTIPLIED_TRANSFORMATIONS(a, b) ((FlutterTransformation) \
	{.scaleX = a.scaleX * b.scaleX + a.skewX  * b.skewY  + a.transX * b.pers0, \
	 .skewX  = a.scaleX * b.skewX  + a.skewX  * b.scaleY + a.transX * b.pers1, \
	 .transX = a.scaleX * b.transX + a.skewX  * b.transY + a.transX * b.pers2, \
	 .skewY  = a.skewY  * b.scaleX + a.scaleY * b.skewY  + a.transY * b.pers0, \
	 .scaleY = a.skewY  * b.skewX  + a.scaleY * b.scaleY + a.transY * b.pers1, \
	 .transY = a.skewY  * b.transX + a.scaleY * b.transY + a.transY * b.pers2, \
	 .pers0  = a.pers0  * b.scaleX + a.pers1  * b.skewY  + a.pers2  * b.pers0, \
	 .pers1  = a.pers0  * b.skewX  + a.pers1  * b.scaleY + a.pers2  * b.pers1, \
	 .pers2  = a.pers0  * b.transX + a.pers1  * b.transY + a.pers2  * b.pers2})

#define FLUTTER_ADDED_TRANSFORMATIONS(a, b) ((FlutterTransformation) \
	{.scaleX = a.scaleX + b.scaleX, .skewX  = a.skewX  + b.skewX,  .transX = a.transX + b.transX, \
	 .skewY  = a.skewY  + b.skewY,  .scaleY = a.scaleY + b.scaleY, .transY = a.transY + b.transY, \
	 .pers0  = a.pers0  + b.pers0,  .pers1  = a.pers1  + b.pers1,  .pers2  = a.pers2  + b.pers2 \
	})

#define FLUTTER_RESULT_TO_STRING(result) \
	((result) == kSuccess ? "Success." : \
	 (result) == kInvalidLibraryVersion ? "Invalid library version." : \
	 (result) == kInvalidArguments ? "Invalid arguments." : \
	 (result) == kInternalInconsistency ? "Internal inconsistency." : "(?)")

#define LIBINPUT_EVENT_IS_TOUCH(event_type) (\
	((event_type) == LIBINPUT_EVENT_TOUCH_DOWN) || \
	((event_type) == LIBINPUT_EVENT_TOUCH_UP) || \
	((event_type) == LIBINPUT_EVENT_TOUCH_MOTION) || \
	((event_type) == LIBINPUT_EVENT_TOUCH_CANCEL) || \
	((event_type) == LIBINPUT_EVENT_TOUCH_FRAME))

#define LIBINPUT_EVENT_IS_POINTER(event_type) (\
	((event_type) == LIBINPUT_EVENT_POINTER_MOTION) || \
	((event_type) == LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE) || \
	((event_type) == LIBINPUT_EVENT_POINTER_BUTTON) || \
	((event_type) == LIBINPUT_EVENT_POINTER_AXIS))

#define LIBINPUT_EVENT_IS_KEYBOARD(event_type) (\
	((event_type) == LIBINPUT_EVENT_KEYBOARD_KEY))

enum frame_state {
	kFramePending,
	kFrameRendering,
	kFrameRendered
};

struct frame {
	/// The current state of the frame.
	/// - Pending, when the frame was requested using the FlutterProjectArgs' vsync_callback.
	/// - Rendering, when the baton was returned to the engine
	/// - Rendered, when the frame has been / is visible on the display.
	enum frame_state state;

	/// The baton to be returned to the flutter engine when the frame can be rendered.
	intptr_t baton;
};

struct compositor;

enum flutter_runtime_mode {
	kDebug, kRelease
};

struct wlFlutter{
    char *asset_bundle_path;
    char *kernel_blob_path;
    char *app_elf_path;
    void *app_elf_handle;
    char *icu_data_path;

    int engine_argc;
    char **engine_argv;
    enum flutter_runtime_mode runtime_mode;
    // struct libflutter_engine libflutter_engine;
    FlutterEngine engine;
};
	
struct platform_task {
	int (*callback)(void *userdata);
	void *userdata;
};

struct platform_message {
	bool is_response;
	union {
		FlutterPlatformMessageResponseHandle *target_handle;
		struct {
			char *target_channel;
			FlutterPlatformMessageResponseHandle *response_handle;
		};
	};
	uint8_t *message;
	size_t message_size;
};

#endif  // EMBEDDER_FLUTTER_INPUTHOOK_H_