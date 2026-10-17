// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "quantum.h"

bool process_timeless_selection(uint16_t keycode, keyrecord_t *record);
void pre_process_timeless_selection(keyrecord_t *record);
void timeless_selection_task(void);

