// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "quantum.h"
#include "layers.h"

enum timeless_indicator_flags {
    TIMELESS_SYMBOL    = 1 << 0,
    TIMELESS_SHIFT     = 1 << 1,
    TIMELESS_NUM       = 1 << 2,
    TIMELESS_CAPS_WORD = 1 << 3,
    TIMELESS_LAYER_LOCK = 1 << 4,
};

uint8_t timeless_active_indicators(void);

// TailorKey HRM_macOS layer. In particular, L uses LEFT Alt on purpose.
enum timeless_keycodes {
    SYM_ARROW = QK_USER_0,
    SELECT_WORD = QK_USER_1,
    SELECT_LINE = QK_USER_2,
    HRM_A = LCTL_T(KC_A),
    HRM_S = LALT_T(KC_S),
    HRM_D = LGUI_T(KC_D),
    HRM_F = LSFT_T(KC_F),
    HRM_J = RSFT_T(KC_J),
    HRM_K = RGUI_T(KC_K),
    HRM_L = LALT_T(KC_L),
    HRM_SCLN = RCTL_T(KC_SCLN),
};
