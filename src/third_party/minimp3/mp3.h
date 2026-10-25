// ps5-homebrew-ui - Declarations for the vendored minimp3 (see mp3.c).
// Copyright (C) 2026 BlackBearReloaded
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// The same switches as mp3.c: the sample type depends on them.
#define MINIMP3_NO_STDIO 1
#define MINIMP3_NO_SIMD 1
#define MINIMP3_FLOAT_OUTPUT 1
#include "minimp3_ex.h"
