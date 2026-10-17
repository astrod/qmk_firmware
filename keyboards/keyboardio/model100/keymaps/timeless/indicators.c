// SPDX-License-Identifier: GPL-2.0-or-later
#include "timeless.h"

#ifdef RGB_MATRIX_ENABLE
// Reserve these seven keys for state feedback instead of the RGB animation.
// Resolve LED indices through keyboard.json's matrix map, not physical order.
static const struct {
    uint8_t row, col, flag, hue;
} indicators[] = {
    {2, 0, TIMELESS_SYMBOL,   170}, // Left thumb Sym: blue.
    {6, 7, TIMELESS_SYMBOL,   170}, // Right thumb Sym: blue.
    {0, 0, TIMELESS_SHIFT,     43}, // Left outer thumb Shift: yellow.
    {4, 7, TIMELESS_SHIFT,     43}, // Right outer thumb Shift: yellow.
    {4, 0, TIMELESS_NUM,       85}, // Top-right Num: green.
    {6, 6, TIMELESS_CAPS_WORD, 21}, // Below right Enter: orange.
    {4, 6, TIMELESS_LAYER_LOCK, 191}, // Above right Enter: purple.
};

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    const uint8_t active = timeless_active_indicators();
    for (uint8_t i = 0; i < ARRAY_SIZE(indicators); ++i) {
        const uint8_t led = g_led_config.matrix_co[indicators[i].row][indicators[i].col];
        if (led == NO_LED || led < led_min || led >= led_max) {
            continue;
        }
        // Follow RGB brightness. Global RGB off/suspend also disables indicators.
        const hsv_t hsv = {.h = indicators[i].hue, .s = 255,
                           .v = (active & indicators[i].flag) ? rgb_matrix_get_val() : 0};
        const rgb_t rgb = hsv_to_rgb(hsv);
        rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
    }
    return true;
}
#endif
