/*
    Copyright (C) 2024 lifehackerhansol
	Additional modifications Copyright (C) 2025-2026 coderkei & Rocket Robz

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef NO_PICO_LAUNCHER

#pragma once

#include <nds/ndstypes.h>

#include "common/picoLoader7.h"

extern int picoLaunchRom(std::string romPath, std::string savePath);

#endif