// Copyright (c) 2011 The TimberSaw Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_TimberSaw_HELPERS_MEMENV_MEMENV_H_
#define STORAGE_TimberSaw_HELPERS_MEMENV_MEMENV_H_

#include "TimberSaw/export.h"

namespace TimberSaw {

class Env;

// Returns a new environment that stores its data in memory and delegates
// all non-file-storage tasks to base_env. The caller must delete the result
// when it is no longer needed.
// *base_env must remain live while the result is in use.
TimberSaw_EXPORT Env* NewMemEnv(Env* base_env);

}  // namespace TimberSaw

#endif  // STORAGE_TimberSaw_HELPERS_MEMENV_MEMENV_H_
