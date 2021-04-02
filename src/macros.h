// Copyright 2013 The Flutter Authors. All rights reserved.
/*
 *  Copyright (C) 2020-2021 XCVMByte Ltd.
 *  All Rights Reserved.
 * 
 */
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef EMBEDDER_FLUTTER_MACROS_H_
#define EMBEDDER_FLUTTER_MACROS_H_

#include <iostream>

#define FLWAY_DISALLOW_COPY(TypeName) TypeName(const TypeName&) = delete;

#define FLWAY_DISALLOW_ASSIGN(TypeName) \
  void operator=(const TypeName&) = delete;

#define FLWAY_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  FLWAY_DISALLOW_COPY(TypeName)                  \
  FLWAY_DISALLOW_ASSIGN(TypeName)

#define __FLWAY_LINE_PREFIX << __FILE__ << ":" << __LINE__ << ": "

#define FLWAY_LOG std::cout << "LOG: " __FLWAY_LINE_PREFIX
#define FLWAY_ERR std::cerr << "ERROR: " __FLWAY_LINE_PREFIX
#define FLWAY_WIP                                          \
  std::cerr << "Work In Progress. Aborting." << std::endl; \
  abort();


#endif  // EMBEDDER_FLUTTER_MACROS_H_