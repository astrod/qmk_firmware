// Copyright 2021-2026 Google LLC
// SPDX-License-Identifier: Apache-2.0
//
// Adapted from Pascal Getreuer's Select Word module, commit
// 8c55ac1c5d547d1ff324ae2834b26f2075222c97 (select_word/select_word.c):
// https://github.com/getreuer/qmk-modules
// See LICENSE.select-word for the Apache License, Version 2.0.
//
// Model 100 changes: Mac-only word/line keys, complete taps with timed word
// repeats, preserve held modifiers, and reset after input or a five-second gap.
// No arrow or modifier stays held while the user rolls into Copy/Cut/Paste.
#include "selection.h"
#include "timeless.h"

static uint16_t last_selection = KC_NO;
static uint32_t selection_timer;
static bool word_repeat_active;
static keypos_t word_repeat_key;
static uint32_t word_repeat_at;

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

static void select_once(uint16_t keycode, bool repeat) {
    // A 32-bit timer avoids keeping a stale selection after a 16-bit wrap.
    const bool extending = repeat || (last_selection == keycode && timer_elapsed32(selection_timer) < SELECT_WORD_TIMEOUT);
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
}

void pre_process_timeless_selection(keyrecord_t *record) {
    if (!word_repeat_active || !IS_KEYEVENT(record->event)) {
        return;
    }
    // Do not resume after an interrupting key is released. E must be pressed
    // anew. Cancelling here also covers keys still waiting for tap/hold.
    const bool owner = KEYEQ(record->event.key, word_repeat_key);
    if ((owner && !record->event.pressed) || (!owner && record->event.pressed)) {
        word_repeat_active = false;
    }
}

void timeless_selection_task(void) {
    if (!word_repeat_active) {
        return;
    }
    if (get_highest_layer(layer_state) != FUN || !matrix_is_on(word_repeat_key.row, word_repeat_key.col)) {
        word_repeat_active = false;
        return;
    }
    if (timer_expired32(timer_read32(), word_repeat_at)) {
        select_once(SELECT_WORD, true);
        // Schedule from now: a delayed scan must not cause a burst of repeats.
        word_repeat_at = timer_read32() + SELECT_WORD_REPEAT_INTERVAL;
    }
}

bool process_timeless_selection(uint16_t keycode, keyrecord_t *record) {
    const bool selecting = keycode == SELECT_WORD || keycode == SELECT_LINE;
    if (!record->event.pressed) {
        if (keycode == SELECT_WORD) {
            word_repeat_active = false;
        }
        return !selecting;
    }
    word_repeat_active = false;
    if (!selecting) {
        if (!selection_control(keycode, record)) {
            last_selection = KC_NO;
        }
        return true;
    }

    const uint32_t pressed_at = timer_read32();
    select_once(keycode, false);
    if (keycode == SELECT_WORD && IS_KEYEVENT(record->event)) {
        word_repeat_key = record->event.key;
        word_repeat_at = pressed_at + SELECT_WORD_REPEAT_DELAY;
        word_repeat_active = true;
    }
    return false;
}
