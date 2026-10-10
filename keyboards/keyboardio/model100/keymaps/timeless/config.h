// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

// TailorKey JSON, base HRM behaviors: all eight keys use these values.
// The filename's 210 ms is not the base HRM tapping term in the file.
#define TAPPING_TERM 280
#define FLOW_TAP_TERM 220
#define QUICK_TAP_TERM_PER_KEY

// Native QMK approach used by getreuer/qmk-keymap.
#define CHORDAL_HOLD
#define PERMISSIVE_HOLD
#define SPECULATIVE_HOLD
#define SPECULATIVE_HOLD_ONE_KEY
// Reuse the source's typing interval for the speculation guard, too.
#define SPECULATIVE_HOLD_FLOW_TERM FLOW_TAP_TERM

// Getreuer's protection against lone Alt/GUI taps during speculation.
#define DUMMY_MOD_NEUTRALIZER_KEYCODE KC_RCTL
#define MODS_TO_NEUTRALIZE { MOD_BIT(KC_LALT), MOD_BIT(KC_LGUI), MOD_BIT(KC_RGUI) }
