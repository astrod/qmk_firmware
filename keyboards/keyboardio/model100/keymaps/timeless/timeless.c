// SPDX-License-Identifier: GPL-2.0-or-later
#include "timeless.h"
#include "selection.h"

// Model 100 has the left half in rows 0-3 and the right half in rows 4-7.
// '*' permits thumb and palm keys to form a chord with either hand, matching
// the source's thumb exceptions. This table follows keyboard.json's matrix.
const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] PROGMEM = {
    {'*', 'L', 'L', 'L', 'L', 'L', 'L', 'L'},
    {'*', 'L', 'L', 'L', 'L', 'L', 'L', 'L'},
    {'*', 'L', 'L', 'L', 'L', 'L', 'L', 'L'},
    {'*', '*', 'L', 'L', 'L', 'L', 'L', 'L'},
    {'R', 'R', 'R', 'R', 'R', 'R', 'R', '*'},
    {'R', 'R', 'R', 'R', 'R', 'R', 'R', '*'},
    {'R', 'R', 'R', 'R', 'R', 'R', 'R', '*'},
    {'R', 'R', 'R', 'R', 'R', 'R', '*', '*'},
};

// QMK measures Quick Tap from release to next press, whereas ZMK measures
// press to press. Preserve the requested numbers, using QMK's native timing.
uint16_t get_quick_tap_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case HRM_A:
        case HRM_S:
        case HRM_D:
        case HRM_F:
            return 175;
        case HRM_J:
        case HRM_K:
        case HRM_L:
        case HRM_SCLN:
            return 210;
        default:
            return QUICK_TAP_TERM;
    }
}

// Like Getreuer, allow speculation for Alt and GUI as well as Ctrl and Shift.
// config.h limits this to one speculative key and suppresses it while typing.
bool get_speculative_hold(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case HRM_A:
        case HRM_S:
        case HRM_D:
        case HRM_F:
        case HRM_J:
        case HRM_K:
        case HRM_L:
        case HRM_SCLN:
            return true;
        default:
            return false;
    }
}

// Keep native QMK Flow Tap eligibility: letters, selected punctuation, space;
// Ctrl/GUI/left-Alt shortcuts are exempt. No TailorKey recovery layers are used.

static bool is_digit(uint16_t keycode) {
    return (keycode >= KC_1 && keycode <= KC_0) || (keycode >= KC_P1 && keycode <= KC_P0);
}

static bool auto_unlock_key(uint8_t layer, uint16_t keycode) {
    if (layer == FUN) {
        // A palm still held after locking keeps all momentary Fn bindings.
        if (matrix_is_on(3, 1) || matrix_is_on(7, 6)) {
            return false;
        }
        // Only locked Fn turns the number row back into digits.
        if (keycode >= KC_F1 && keycode <= KC_F10) {
            return true;
        }
    } else if (layer == SYM) {
        if (matrix_is_on(2, 0) || matrix_is_on(6, 7)) {
            return false;
        }
        // Symbols can inherit the number row through a lower Fn layer.
        if (keycode >= KC_F1 && keycode <= KC_F10) {
            return true;
        }
    } else {
        return false;
    }

    if (IS_QK_MOD_TAP(keycode)) {
        keycode = QK_MOD_TAP_GET_TAP_KEYCODE(keycode);
    }
    if ((keycode >= KC_A && keycode <= KC_Z) || is_digit(keycode)) {
        return true;
    }
    switch (keycode) {
        case KC_SPC:
        case KC_ENT:
        case KC_TAB:
        case KC_ESC:
            return true;
        case KC_GRV:
        case KC_MINS:
        case KC_EQL:
        case KC_SCLN:
        case KC_QUOT:
        case KC_COMM:
        case KC_DOT:
        case KC_SLSH:
            // In Fn these are ordinary typing keys. In Sym they are symbols.
            return layer == FUN;
        default:
            return false;
    }
}

bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Cancel repeats at physical key input, before a home-row tap/hold waits.
    pre_process_timeless_selection(record);
    if (!IS_KEYEVENT(record->event) || !record->event.pressed) {
        return true;
    }
    // Unwind only qualifying locked layers, from the top. NUM is never changed.
    // Do this before tap/hold processing so the exit key uses the new layer's
    // normal action, including home-row mods, and is not swallowed or replayed.
    for (uint8_t i = 0; i < 2; ++i) {
        const uint8_t layer = get_highest_layer(layer_state);
        if (!is_layer_locked(layer) || !auto_unlock_key(layer, keycode)) {
            break;
        }
        layer_lock_off(layer);
        keycode = get_record_keycode(record, true);
    }
    // QMK's source-layer cache retains releases of keys held before the exit.
    return true;
}

uint8_t timeless_active_indicators(void) {
    uint8_t flags = 0;
    if (layer_state_is(SYM)) {
        flags |= TIMELESS_SYMBOL;
    }
    if ((get_oneshot_mods() | get_oneshot_locked_mods()) & MOD_MASK_SHIFT) {
        flags |= TIMELESS_SHIFT;
    }
    // Show the toggled number layer even when another layer is above it.
    if (layer_state_is(NUM)) {
        flags |= TIMELESS_NUM;
    }
    if (is_caps_word_on()) {
        flags |= TIMELESS_CAPS_WORD;
    }
    if (is_layer_locked(FUN) || is_layer_locked(SYM)) {
        flags |= TIMELESS_LAYER_LOCK;
    }
    return flags;
}

void housekeeping_task_user(void) {
    timeless_selection_task();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_timeless_selection(keycode, record)) {
        return false;
    }
    if (keycode == QK_LAYER_LOCK) {
        // Only lock Fn and symbols. Num already has a toggle; the base layer
        // should ignore this key. Let native Layer Lock handle both edges.
        const uint8_t layer = get_highest_layer(layer_state);
        return layer == FUN || layer == SYM;
    }
    if (keycode != SYM_ARROW) {
        return true;
    }
    if (record->event.pressed) {
        // This macro returns before QMK's Caps Word handler. End the word
        // here so the arrow is a delimiter and no weak Shift is restored.
        caps_word_off();
        // Type a literal arrow even with a held or one-shot modifier. Restore
        // held modifiers, but consume one-shot modifiers with this macro.
        const uint8_t mods = get_mods();
        const uint8_t weak_mods = get_weak_mods();
        clear_mods();
        clear_weak_mods();
        clear_oneshot_mods();
        send_keyboard_report();
        SEND_STRING("->");
        set_mods(mods);
        set_weak_mods(weak_mods);
        send_keyboard_report();
    }
    // QMK consumes a pending one-shot layer when this press returns false.
    return false;
}
