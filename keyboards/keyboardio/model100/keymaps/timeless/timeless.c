// SPDX-License-Identifier: GPL-2.0-or-later
#include "timeless.h"

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
    return flags;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode != SYM_ARROW) {
        return true;
    }
    if (record->event.pressed) {
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
