// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

// Getreuer's personal timing, mapped by modifier role in timeless.c.
#define TAPPING_TERM 240
#define TAPPING_TERM_PER_KEY
#define FLOW_TAP_TERM 100
#define QUICK_TAP_TERM_PER_KEY

// Getreuer's native QMK approach, with the source's balanced hold behavior:
// opposite-hand nested taps hold, while rolled letter pairs remain taps.
#define CHORDAL_HOLD
#define PERMISSIVE_HOLD
#define SPECULATIVE_HOLD
#define SPECULATIVE_HOLD_ONE_KEY
// Keep Getreuer's separate guard against speculative mods during typing.
#define SPECULATIVE_HOLD_FLOW_TERM 200

// Getreuer's protection against lone Alt/GUI taps during speculation.
#define DUMMY_MOD_NEUTRALIZER_KEYCODE KC_RCTL
#define MODS_TO_NEUTRALIZE { MOD_BIT(KC_LALT), MOD_BIT(KC_LGUI), MOD_BIT(KC_RGUI) }

// Start a fresh word/line selection after five seconds without a selection tap.
#define SELECT_WORD_TIMEOUT 5000
// Word selection: first hold repeat at 400 ms, then one word every 150 ms.
#define SELECT_WORD_REPEAT_DELAY 400
#define SELECT_WORD_REPEAT_INTERVAL 150
