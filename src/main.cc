// Copyright 2013 The Flutter Authors. All rights reserved.
/*
 *  Copyright (C) 2020-2021 XCVMByte Ltd.
 *  All Rights Reserved.
 * 
 */
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>

#include "log.h"
#include "flutter_application.h"
#include "utils.h"
#include "wayland_display.h"
#include "input_hook.h"

namespace flutter {

static void PrintUsage() {
  std::cerr << "Flutter Wayland Embedder for embedder ARM V1.0" << std::endl << std::endl;
  std::cerr << "========================" << std::endl;
  std::cerr << "Usage: `" << GetExecutableName()
            << " <asset_bundle_path> <flutter_flags>`" << std::endl
            << std::endl;
}

struct wlFlutter myWlFlutter;
int myDefaultDisplayWidth;
int myDefaultDisplayHeight;

bool parse_cmd_args(int argc, char** argv) {
  bool input_specified = false;
  int opt;
  int longopt_index = 0;
  int runtime_mode_int = kDebug;
  int disable_text_input_int = false;
  int ok;

  struct option long_options[] = {
      {"release", no_argument, &runtime_mode_int, kRelease},
      {"input", required_argument, NULL, 'i'},
      {"orientation", required_argument, NULL, 'o'},
      {"rotation", required_argument, NULL, 'r'},
      {"no-text-input", no_argument, &disable_text_input_int, true},
      {"dimensions", required_argument, NULL, 'd'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}};

  // TODO: Change to adapt to real display size
  myDefaultDisplayWidth = 720;
  myDefaultDisplayHeight = 1260;

  bool finished_parsing_options = false;
  while (!finished_parsing_options) {
    longopt_index = 0;
    opt = getopt_long(argc, argv, "+i:o:r:d:h", long_options, &longopt_index);

    switch (opt) {
      case 'd':;
        unsigned int width_mm, height_mm;

        ok = sscanf(optarg, "%u,%u", &width_mm, &height_mm);
        if ((ok == 0) || (ok == EOF)) {
          LOG_ERROR(stderr,
                    "ERROR: Invalid argument for --dimensions passed.\n");
          return false;
        }

        myDefaultDisplayWidth = width_mm;
        myDefaultDisplayHeight = height_mm;

        break;

      case 'h':
        PrintUsage();
        return false;

      case '?':
      case ':':
        return false;

      case -1:
        finished_parsing_options = true;
        break;

      default:
        break;
    }
  }

  if (optind >= argc) {
    LOG_ERROR(stderr, "error: expected asset bundle path after options.\n");
    PrintUsage();

    return false;
  }

  myWlFlutter.asset_bundle_path = strdup(argv[optind]);
  myWlFlutter.runtime_mode = (flutter_runtime_mode)runtime_mode_int;

  argv[optind] = argv[0];
  myWlFlutter.engine_argc = argc - optind;
  myWlFlutter.engine_argv = argv + optind;

  return true;
}

const char *get_asset_bundle_path(){
	return myWlFlutter.asset_bundle_path;
}

int get_view_width(){
	return myDefaultDisplayWidth;
}

int get_view_height(){
	return myDefaultDisplayHeight;
}

int get_engine_argc() {
	return myWlFlutter.engine_argc;
}

const char * const * get_engine_argv() {
	return (const char * const *)myWlFlutter.engine_argv;
}

bool setup_paths(void) {
	char *kernel_blob_path, *icu_data_path, *app_elf_path;
	#define PATH_EXISTS(path) (access((path),R_OK)==0)

	if (!PATH_EXISTS(myWlFlutter.asset_bundle_path)) {
		LOG_ERROR(stderr, "Asset Bundle Directory \"%s\" does not exist\n", myWlFlutter.asset_bundle_path);
		return false;
	}
	
	asprintf(&kernel_blob_path, "%s/kernel_blob.bin", myWlFlutter.asset_bundle_path);
	asprintf(&app_elf_path, "%s/app.so", myWlFlutter.asset_bundle_path);

	FLWAY_LOG << "\n  kernel blob: " <<  kernel_blob_path << std::endl;
	FLWAY_LOG << "app elf path: " << app_elf_path << std::endl;

	if (myWlFlutter.runtime_mode == kDebug) {
		if (!PATH_EXISTS(kernel_blob_path)) {
			LOG_ERROR(stderr, "[flutter-rk] Could not find \"kernel.blob\" file inside \"%s\", which is required for debug mode.\n", myWlFlutter.asset_bundle_path);
			return false;
		}
	} else if (myWlFlutter.runtime_mode == kRelease) {
		if (!PATH_EXISTS(app_elf_path)) {
			LOG_ERROR(stderr, "[flutter-rk] Could not find \"app.so\" file inside \"%s\", which is required for release and profile mode.\n", myWlFlutter.asset_bundle_path);
			return false;
		}
	}

	asprintf(&icu_data_path, "/usr/lib/icudtl.dat");
	if (!PATH_EXISTS(icu_data_path)) {
		LOG_ERROR(stderr, "[flutter-rk] Could not find \"icudtl.dat\" file inside \"/usr/lib/\".\n");
		return false;
	}

	myWlFlutter.kernel_blob_path = kernel_blob_path;
	myWlFlutter.icu_data_path = icu_data_path;
	myWlFlutter.app_elf_path = app_elf_path;

	return true;

	#undef PATH_EXISTS
}

static bool Main(int argc, char *argv[]) {
	int ok;

	ok = parse_cmd_args(argc, argv);
	if (ok == false) {
		return EINVAL;
	}

	ok = setup_paths();
	if (ok == false) {
		return EINVAL;
	}


  const auto asset_bundle_path = get_asset_bundle_path();

  if (!FlutterAssetBundleIsValid(asset_bundle_path)) {
    std::cerr << "   <Invalid Flutter Asset Bundle>   " << std::endl;
    PrintUsage();
    return false;
  }

  // ok = init_display();

  const size_t kWidth = get_view_width();
  const size_t kHeight = get_view_height();;

  FLWAY_LOG << " current application view width: " << kWidth << ", height: " <<  kHeight << std::endl;

  WaylandDisplay display(kWidth, kHeight);

  if (!display.IsValid()) {
    FLWAY_ERR << "Wayland display was not valid." << std::endl;
    return false;
  }
	
  FlutterApplication application(asset_bundle_path, display);
  if (!application.IsValid()) {
    FLWAY_ERR << "Flutter application was not valid." << std::endl;
    return false;
  }

  if (!application.SetWindowSize(kWidth, kHeight)) {
    FLWAY_ERR << "Could not update Flutter application size." << std::endl;
    return false;
  }

  FLWAY_LOG << "WaylandDisplay and flutter application is ready. Prepare to run......" << std::endl;
  display.Run();

  return true;
}

}  // namespace flutter

int main(int argc, char* argv[]) {
  FLWAY_LOG << "Starting Flutter Wayland Embedder for embedder ARM ...\n" << std::endl;
  return flutter::Main(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE;
}
