// Copyright 2021-2026 Google LLC
// SPDX-License-Identifier: Apache-2.0
//
// Adapted from Pascal Getreuer's Select Word module, commit
// 8c55ac1c5d547d1ff324ae2834b26f2075222c97 (select_word/select_word.c):
// https://github.com/getreuer/qmk-modules
// See LICENSE.select-word for the Apache License, Version 2.0.
//
// Model 100 changes: Mac-only word/line keys, complete each action on keydown,
// preserve held modifiers, and reset on other input or a five-second gap.
// No arrow or modifier stays held while the user rolls into Copy/Cut/Paste.
#include "selection.h"
#include "timeless.h"

static uint16_t last_selection = KC_NO;
static uint32_t selection_timer;

static bool selection_control(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case MODIFIER_KEYCODE_RANGE:
        case QK_MOMENTARY ... QK_MOMENTARY_MAX:
        case QK_LAYER_MOD ... QK_LAYER_MOD_MAX:
        case QK_LAYER_TAP_TOGGLE ... QK_LAYER_TAP_TOGGLE_MAX:
        case QK_TO ... QK_TO_MAX:
        case QK_TOGGLE_LAYER ... QK_TOGGLE_LAYER_MAX:
        case QK_ONE_SHOT_LAYER ... QK_ONE_SHOT_LAYER_MAX:
        case QK_ONE_SHOT_MOD ... QK_ONE_SHOT_MOD_MAX:
        case QK_LAYER_LOCK:
            return true;
        case QK_MOD_TAP ... QK_MOD_TAP_MAX:
        case QK_LAYER_TAP ... QK_LAYER_TAP_MAX:
            return record->tap.count == 0;
        default:
            return false;
    }
}

bool process_timeless_selection(uint16_t keycode, keyrecord_t *record) {
    const bool selecting = keycode == SELECT_WORD || keycode == SELECT_LINE;
    if (!record->event.pressed) {
        return !selecting;
    }
    if (!selecting) {
        if (!selection_control(keycode, record)) {
            last_selection = KC_NO;
        }
        return true;
    }

    // A 32-bit timer avoids keeping a stale selection after a 16-bit wrap.
    const bool extending = last_selection == keycode && timer_elapsed32(selection_timer) < SELECT_WORD_TIMEOUT;
    caps_word_off();
    const uint8_t mods = get_mods();
    const uint8_t weak_mods = get_weak_mods();
    clear_mods();
    clear_weak_mods();
    clear_oneshot_mods();
    send_keyboard_report();

    if (keycode == SELECT_WORD) {
        if (!extending) {
            tap_code16_delay(LALT(KC_RGHT), TAP_CODE_DELAY);
            tap_code16_delay(LALT(KC_LEFT), TAP_CODE_DELAY);
        }
        tap_code16_delay(LSFT(LALT(KC_RGHT)), TAP_CODE_DELAY);
    } else {
        if (extending) {
            tap_code16_delay(LSFT(KC_DOWN), TAP_CODE_DELAY);
        } else {
            tap_code16_delay(LGUI(KC_LEFT), TAP_CODE_DELAY);
        }
        // Extend all the way to the next line's end, even if lengths differ.
        tap_code16_delay(LSFT(LGUI(KC_RGHT)), TAP_CODE_DELAY);
    }

    set_mods(mods);
    set_weak_mods(weak_mods);
    send_keyboard_report();
    last_selection = keycode;
    selection_timer = timer_read32();
    return false;
}
