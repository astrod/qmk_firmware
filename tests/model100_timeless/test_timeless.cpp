// SPDX-License-Identifier: GPL-2.0-or-later
#include <algorithm>
#include <utility>
#include <vector>
#include "test_common.hpp"
extern "C" {
#include "action_tapping.h"
#include "keyboards/keyboardio/model100/keymaps/timeless/timeless.h"
}

using Press = std::pair<uint8_t, uint8_t>; // HID key and modifiers on keydown.
using testing::_;

class Model100Timeless : public TestFixture {
protected:
    std::vector<Press> presses;
    report_keyboard_t previous = {};

    void capture(TestDriver &driver) {
        EXPECT_CALL(driver, send_keyboard_mock(_)).Times(testing::AnyNumber())
            .WillRepeatedly([this](report_keyboard_t &report) {
                for (uint8_t key : report.keys) {
                    if (key != KC_NO && std::find(std::begin(previous.keys), std::end(previous.keys), key) == std::end(previous.keys)) {
                        presses.emplace_back(key, report.mods);
                    }
                }
                previous = report;
            });
        idle_for(600); // Clear QMK's previous typing interval between cases.
    }

    void press(KeymapKey &key, unsigned ms = 20) { key.press(); idle_for(ms); }
    void release(KeymapKey &key) { key.release(); run_one_scan_loop(); }
    void expect_clear() {
        EXPECT_EQ(get_mods(), 0);
        EXPECT_EQ(get_speculative_mods(), 0);
        EXPECT_EQ(previous.mods, 0);
        for (uint8_t key : previous.keys) EXPECT_EQ(key, KC_NO);
    }
};

struct HomeKey { uint8_t col, row; uint16_t code; uint8_t tap, mod; };
const HomeKey home[] = {
    {6, 2, HRM_A, KC_A, MOD_BIT(KC_LCTL)},
    {5, 2, HRM_S, KC_S, MOD_BIT(KC_LALT)},
    {4, 2, HRM_D, KC_D, MOD_BIT(KC_LGUI)},
    {3, 2, HRM_F, KC_F, MOD_BIT(KC_LSFT)},
    {4, 6, HRM_J, KC_J, MOD_BIT(KC_RSFT)},
    {3, 6, HRM_K, KC_K, MOD_BIT(KC_RGUI)},
    {2, 6, HRM_L, KC_L, MOD_BIT(KC_LALT)},
    {1, 6, HRM_SCLN, KC_SCLN, MOD_BIT(KC_RCTL)},
};

TEST_F(Model100Timeless, AllEightTapAsUnmodifiedLetters) {
    TestDriver driver; capture(driver);
    for (auto h : home) {
        auto key = KeymapKey(0, h.col, h.row, h.code);
        set_keymap({key}); presses.clear();
        tap_key(key, 40);
        EXPECT_EQ(presses, (std::vector<Press>{{h.tap, 0}}));
        expect_clear(); idle_for(600);
    }
}

TEST_F(Model100Timeless, AllEightHoldAt280msWithSourceModifiers) {
    TestDriver driver; capture(driver);
    for (auto h : home) {
        auto key = KeymapKey(0, h.col, h.row, h.code);
        set_keymap({key}); presses.clear();
        press(key, 270);
        EXPECT_EQ(get_mods(), 0); // Not settled; only speculative so far.
        EXPECT_EQ(get_speculative_mods(), h.mod);
        idle_for(20);
        EXPECT_EQ(get_mods(), h.mod);
        EXPECT_EQ(get_speculative_mods(), 0);
        release(key);
        EXPECT_TRUE(presses.empty()); expect_clear(); idle_for(600);
    }
}

TEST_F(Model100Timeless, SameHandNestedFWTypesLetters) {
    TestDriver driver; capture(driver);
    auto f = KeymapKey(0, 3, 2, HRM_F), w = KeymapKey(0, 5, 1, KC_W);
    set_keymap({f, w});
    press(f); tap_key(w); release(f);
    EXPECT_EQ(presses, (std::vector<Press>{{KC_F, 0}, {KC_W, 0}}));
    expect_clear();
}

TEST_F(Model100Timeless, OppositeHandNestedFUUsesShiftBeforeTimeout) {
    TestDriver driver; capture(driver);
    auto f = KeymapKey(0, 3, 2, HRM_F), u = KeymapKey(0, 4, 5, KC_U);
    set_keymap({f, u});
    press(f); tap_key(u);
    EXPECT_EQ(get_mods(), MOD_BIT(KC_LSFT));
    release(f);
    EXPECT_EQ(presses, (std::vector<Press>{{KC_U, MOD_BIT(KC_LSFT)}}));
    expect_clear();
}

TEST_F(Model100Timeless, TwoSameHandHomeModsCanFormCommandShiftU) {
    TestDriver driver; capture(driver);
    auto d = KeymapKey(0, 4, 2, HRM_D), f = KeymapKey(0, 3, 2, HRM_F), u = KeymapKey(0, 4, 5, KC_U);
    set_keymap({d, f, u});
    press(d); press(f);
    EXPECT_EQ(get_speculative_mods(), MOD_BIT(KC_LGUI)); // ONE_KEY guard.
    tap_key(u); release(f); release(d);
    EXPECT_EQ(presses, (std::vector<Press>{{KC_U, MOD_BIT(KC_LGUI) | MOD_BIT(KC_LSFT)}}));
    expect_clear();
}

TEST_F(Model100Timeless, FlowTapWithin220msStaysLetterEvenWhenHeld) {
    TestDriver driver; capture(driver);
    auto f = KeymapKey(0, 3, 2, HRM_F), u = KeymapKey(0, 4, 5, KC_U);
    set_keymap({f, u});
    tap_key(u); idle_for(210); press(f, 400);
    EXPECT_EQ(get_mods(), 0); EXPECT_EQ(get_speculative_mods(), 0);
    release(f);
    EXPECT_EQ(presses, (std::vector<Press>{{KC_U, 0}, {KC_F, 0}}));
    expect_clear();
}

TEST_F(Model100Timeless, After220msPauseOppositeHandShortcutWorks) {
    TestDriver driver; capture(driver);
    auto f = KeymapKey(0, 3, 2, HRM_F), u = KeymapKey(0, 4, 5, KC_U);
    set_keymap({f, u});
    tap_key(u); idle_for(230); press(f); tap_key(u); release(f);
    EXPECT_EQ(presses, (std::vector<Press>{{KC_U, 0}, {KC_U, MOD_BIT(KC_LSFT)}}));
    expect_clear();
}

TEST_F(Model100Timeless, SameHandThumbAllowsShiftBackspace) {
    TestDriver driver; capture(driver);
    auto f = KeymapKey(0, 3, 2, HRM_F), bs = KeymapKey(0, 0, 1, KC_BSPC);
    set_keymap({f, bs});
    press(f); tap_key(bs); release(f);
    EXPECT_EQ(presses, (std::vector<Press>{{KC_BSPC, MOD_BIT(KC_LSFT)}}));
    expect_clear();
}

TEST_F(Model100Timeless, LongHoldThenSameHandKeyDoesNotReplayF) {
    TestDriver driver; capture(driver);
    auto f = KeymapKey(0, 3, 2, HRM_F), w = KeymapKey(0, 5, 1, KC_W);
    set_keymap({f, w});
    press(f, 300); tap_key(w); release(f);
    EXPECT_EQ(presses, (std::vector<Press>{{KC_W, MOD_BIT(KC_LSFT)}}));
    expect_clear();
}

TEST_F(Model100Timeless, QuickTapUsesDifferentLeftAndRightWindows) {
    TestDriver driver; capture(driver);
    // A physical Ctrl disables Flow Tap so the Quick Tap windows can be tested.
    auto ctrl = KeymapKey(0, 0, 0, KC_LCTL);
    for (auto h : {home[3], home[4]}) {
        auto key = KeymapKey(0, h.col, h.row, h.code);
        set_keymap({ctrl, key}); presses.clear();
        press(ctrl); tap_key(key, 10); idle_for(190);
        press(key, 300);
        if (h.code == HRM_F) {
            EXPECT_EQ(get_mods(), MOD_BIT(KC_LCTL) | h.mod); // Past 175 ms.
            EXPECT_EQ(presses.size(), 1u);
        } else {
            EXPECT_EQ(get_mods(), MOD_BIT(KC_LCTL)); // Still within 210 ms.
            EXPECT_EQ(presses, (std::vector<Press>{{KC_J, MOD_BIT(KC_LCTL)}, {KC_J, MOD_BIT(KC_LCTL)}}));
        }
        release(key); release(ctrl); expect_clear(); idle_for(600);
    }
}

TEST_F(Model100Timeless, MatrixSidesAndThumbExceptions) {
    for (uint8_t row = 0; row < 8; ++row) {
        for (uint8_t col = 0; col < 8; ++col) {
            bool thumb = row < 4 ? (col == 0 || (row == 3 && col == 1)) : (col == 7 || (row == 7 && col == 6));
            EXPECT_EQ(chordal_hold_handedness({.col = col, .row = row}), thumb ? '*' : (row < 4 ? 'L' : 'R'));
        }
    }
}

TEST_F(Model100Timeless, RightSymbolThumbTypesLeftBracketThenReturnsToBaseC) {
    TestDriver driver; capture(driver);
    auto osl = KeymapKey(DEF, 7, 6, OSL(SYM));
    auto bracket = KeymapKey(SYM, 4, 3, KC_LBRC), c = KeymapKey(DEF, 4, 3, KC_C);
    set_keymap({osl, bracket, c});
    tap_key(osl, 40);
    EXPECT_EQ(timeless_active_indicators(), TIMELESS_SYMBOL);
    EXPECT_EQ(get_mods(), 0);
    tap_key(bracket, 40);
    EXPECT_EQ(timeless_active_indicators(), 0);
    tap_key(c, 40);
    EXPECT_EQ(presses, (std::vector<Press>{{KC_LBRC, 0}, {KC_C, 0}}));
    expect_clear();
}

TEST_F(Model100Timeless, AlternatingSymbolThumbsDoNotLeaveTheLayerActive) {
    TestDriver driver; capture(driver);
    auto left = KeymapKey(DEF, 0, 2, OSL(SYM)), right = KeymapKey(DEF, 7, 6, OSL(SYM));
    auto underscore = KeymapKey(SYM, 3, 7, KC_UNDS);
    set_keymap({left, right, underscore});
    for (auto key : {&left, &right, &left, &right}) {
        presses.clear();
        tap_key(*key, 30);
        EXPECT_EQ(timeless_active_indicators(), TIMELESS_SYMBOL);
        tap_key(underscore, 30);
        EXPECT_EQ(timeless_active_indicators(), 0);
        EXPECT_EQ(presses, (std::vector<Press>{{KC_MINS, MOD_BIT(KC_LSFT)}}));
        expect_clear();
    }
    expect_clear();
}

TEST_F(Model100Timeless, BothOuterThumbsApplyOneShotLeftShiftOnce) {
    TestDriver driver; capture(driver);
    auto u = KeymapKey(0, 4, 5, KC_U);
    for (auto side : {0, 1}) {
        auto shift = KeymapKey(0, side ? 7 : 0, side ? 4 : 0, OSM(MOD_LSFT));
        set_keymap({shift, u}); presses.clear();
        tap_key(shift, 40);
        EXPECT_EQ(timeless_active_indicators(), TIMELESS_SHIFT);
        tap_key(u);
        EXPECT_EQ(timeless_active_indicators(), 0);
        tap_key(u);
        EXPECT_EQ(presses, (std::vector<Press>{{KC_U, MOD_BIT(KC_LSFT)}, {KC_U, 0}}));
        EXPECT_EQ(get_oneshot_mods(), 0);
        expect_clear(); idle_for(600);
    }
}

TEST_F(Model100Timeless, OneShotShiftCombinesWithHomeRowTaps) {
    TestDriver driver; capture(driver);
    auto shift = KeymapKey(0, 0, 0, OSM(MOD_LSFT));
    auto u = KeymapKey(0, 4, 5, KC_U);
    for (auto h : home) {
        auto key = KeymapKey(0, h.col, h.row, h.code);
        set_keymap({shift, key, u}); presses.clear();
        tap_key(shift, 40); tap_key(key, 40); tap_key(u);
        EXPECT_EQ(presses, (std::vector<Press>{{h.tap, MOD_BIT(KC_LSFT)}, {KC_U, 0}}));
        EXPECT_EQ(get_oneshot_mods(), 0);
        expect_clear(); idle_for(600);
    }
}

TEST_F(Model100Timeless, OneShotSymbolTypesUnderscoreThenReturnsToBaseComma) {
    TestDriver driver; capture(driver);
    auto osl = KeymapKey(0, 0, 2, OSL(3));
    auto comma = KeymapKey(0, 3, 7, KC_COMM);
    auto underscore = KeymapKey(3, 3, 7, KC_UNDS);
    set_keymap({osl, comma, underscore});
    tap_key(osl, 40);
    EXPECT_TRUE(layer_state_is(3));
    EXPECT_EQ(timeless_active_indicators(), TIMELESS_SYMBOL);
    tap_key(underscore, 40);
    EXPECT_FALSE(layer_state_is(3));
    EXPECT_EQ(timeless_active_indicators(), 0);
    tap_key(comma, 40);
    EXPECT_EQ(presses, (std::vector<Press>{{KC_MINS, MOD_BIT(KC_LSFT)}, {KC_COMM, 0}}));
    expect_clear();
}

TEST_F(Model100Timeless, OneShotSymbolCanBeUsedTwiceQuickly) {
    TestDriver driver; capture(driver);
    auto osl = KeymapKey(0, 0, 2, OSL(3));
    auto underscore = KeymapKey(3, 3, 7, KC_UNDS);
    set_keymap({osl, underscore});
    tap_key(osl, 30); tap_key(underscore, 30);
    tap_key(osl, 30); tap_key(underscore, 30);
    EXPECT_FALSE(layer_state_is(3));
    EXPECT_EQ(presses, (std::vector<Press>{{KC_MINS, MOD_BIT(KC_LSFT)}, {KC_MINS, MOD_BIT(KC_LSFT)}}));
    expect_clear();
}

TEST_F(Model100Timeless, HeldSymbolAllowsSeveralSymbolsAndClearsOnRelease) {
    TestDriver driver; capture(driver);
    auto left = KeymapKey(3, 4, 3, KC_LBRC), right = KeymapKey(3, 3, 3, KC_RBRC);
    auto backslash = KeymapKey(3, 1, 5, KC_BSLS);
    auto c = KeymapKey(0, 4, 3, KC_C);
    for (bool right_thumb : {false, true}) {
        auto osl = KeymapKey(DEF, right_thumb ? 7 : 0, right_thumb ? 6 : 2, OSL(SYM));
        set_keymap({osl, left, right, backslash, c}); presses.clear();
        press(osl, 300);
        EXPECT_EQ(timeless_active_indicators(), TIMELESS_SYMBOL);
        tap_key(left); tap_key(right); tap_key(backslash);
        EXPECT_TRUE(layer_state_is(3));
        release(osl);
        EXPECT_FALSE(layer_state_is(3));
        EXPECT_EQ(timeless_active_indicators(), 0);
        tap_key(c);
        EXPECT_EQ(presses, (std::vector<Press>{{KC_LBRC, 0}, {KC_RBRC, 0}, {KC_BSLS, 0}, {KC_C, 0}}));
        expect_clear(); idle_for(600);
    }
}

TEST_F(Model100Timeless, OneShotArrowTypesTwoCharactersThenReturnsToHomeRowMod) {
    TestDriver driver; capture(driver);
    auto arrow = KeymapKey(3, 2, 6, SYM_ARROW), l = KeymapKey(0, 2, 6, HRM_L);
    for (bool right_thumb : {false, true}) {
        auto osl = KeymapKey(DEF, right_thumb ? 7 : 0, right_thumb ? 6 : 2, OSL(SYM));
        set_keymap({osl, arrow, l}); presses.clear();
        tap_key(osl, 40);
        EXPECT_EQ(timeless_active_indicators(), TIMELESS_SYMBOL);
        tap_key(arrow, 40);
        EXPECT_FALSE(layer_state_is(3));
        EXPECT_EQ(timeless_active_indicators(), 0);
        tap_key(l, 40);
        EXPECT_EQ(presses, (std::vector<Press>{{KC_MINS, 0}, {KC_DOT, MOD_BIT(KC_LSFT)}, {KC_L, 0}}));
        expect_clear(); idle_for(600);
    }
}

TEST_F(Model100Timeless, ArrowConsumesOneShotShiftWithoutChangingLiteralText) {
    TestDriver driver; capture(driver);
    auto shift = KeymapKey(0, 0, 0, OSM(MOD_LSFT));
    auto osl = KeymapKey(0, 0, 2, OSL(3));
    auto arrow = KeymapKey(3, 2, 6, SYM_ARROW), u = KeymapKey(0, 4, 5, KC_U);
    set_keymap({shift, osl, arrow, u});
    tap_key(shift, 40); tap_key(osl, 40); tap_key(arrow, 40); tap_key(u);
    EXPECT_FALSE(layer_state_is(3));
    EXPECT_EQ(get_oneshot_mods(), 0);
    EXPECT_EQ(presses, (std::vector<Press>{{KC_MINS, 0}, {KC_DOT, MOD_BIT(KC_LSFT)}, {KC_U, 0}}));
    expect_clear();
}

TEST_F(Model100Timeless, ArrowRestoresHeldAltForTheNextShortcut) {
    TestDriver driver; capture(driver);
    auto alt = KeymapKey(DEF, 5, 2, HRM_S), osl = KeymapKey(0, 0, 2, OSL(3));
    auto arrow = KeymapKey(3, 2, 6, SYM_ARROW), u = KeymapKey(0, 4, 5, KC_U);
    auto transparent_alt = KeymapKey(SYM, 5, 2, KC_TRNS);
    set_keymap({alt, osl, arrow, u, transparent_alt});
    press(alt, 300); tap_key(osl, 40); tap_key(arrow, 40);
    EXPECT_EQ(get_mods(), MOD_BIT(KC_LALT));
    EXPECT_FALSE(layer_state_is(3));
    tap_key(u); release(alt);
    EXPECT_EQ(presses, (std::vector<Press>{{KC_MINS, 0}, {KC_DOT, MOD_BIT(KC_LSFT)}, {KC_U, MOD_BIT(KC_LALT)}}));
    expect_clear();
}

TEST_F(Model100Timeless, EitherPalmEnablesImmediateFnModifiersForArrows) {
    TestDriver driver; capture(driver);
    const uint16_t codes[] = {KC_LCTL, KC_LALT, KC_LGUI, KC_LSFT};
    for (bool right_palm : {false, true}) {
        for (uint8_t i = 0; i < 4; ++i) {
            auto fn = KeymapKey(DEF, right_palm ? 6 : 1, right_palm ? 7 : 3, MO(FUN));
            auto modifier = KeymapKey(FUN, 6 - i, 2, codes[i]);
            auto left = KeymapKey(FUN, 5, 6, KC_LEFT);
            set_keymap({fn, modifier, left}); presses.clear();
            press(fn, 1); press(modifier, 1); tap_key(left);
            release(modifier); release(fn);
            EXPECT_EQ(presses, (std::vector<Press>{{KC_LEFT, MOD_BIT(codes[i])}}));
            EXPECT_FALSE(layer_state_is(FUN));
            expect_clear(); idle_for(600);
        }
    }
}

TEST_F(Model100Timeless, FnCommandShiftSelectionDoesNotStickWhenPalmReleasedFirst) {
    TestDriver driver; capture(driver);
    auto fn = KeymapKey(DEF, 1, 3, MO(FUN));
    auto cmd = KeymapKey(FUN, 4, 2, KC_LGUI), shift = KeymapKey(FUN, 3, 2, KC_LSFT);
    auto right = KeymapKey(FUN, 2, 6, KC_RGHT);
    auto base_d = KeymapKey(DEF, 4, 2, HRM_D), base_f = KeymapKey(DEF, 3, 2, HRM_F);
    auto u = KeymapKey(DEF, 4, 5, KC_U);
    set_keymap({fn, cmd, shift, right, base_d, base_f, u});
    press(fn); press(cmd, 1); press(shift, 1); tap_key(right);
    release(fn); release(cmd); release(shift); tap_key(u);
    EXPECT_EQ(presses, (std::vector<Press>{{KC_RGHT, MOD_BIT(KC_LGUI) | MOD_BIT(KC_LSFT)}, {KC_U, 0}}));
    expect_clear();
}

TEST_F(Model100Timeless, NumberIndicatorPersistsUnderFnAndClearsWhenToggledOff) {
    TestDriver driver; capture(driver);
    auto num = KeymapKey(DEF, 0, 4, TG(NUM)), num_through = KeymapKey(NUM, 0, 4, KC_TRNS);
    auto fn = KeymapKey(DEF, 1, 3, MO(FUN)), fn_through = KeymapKey(NUM, 1, 3, KC_TRNS);
    set_keymap({num, num_through, fn, fn_through});
    tap_key(num);
    EXPECT_EQ(timeless_active_indicators(), TIMELESS_NUM);
    press(fn);
    EXPECT_TRUE(layer_state_is(FUN));
    EXPECT_EQ(timeless_active_indicators(), TIMELESS_NUM);
    release(fn); tap_key(num);
    EXPECT_EQ(timeless_active_indicators(), 0);
    EXPECT_TRUE(presses.empty()); expect_clear();
}

TEST_F(Model100Timeless, SymbolAndShiftIndicatorsClearTogetherAfterArrowMacro) {
    TestDriver driver; capture(driver);
    auto shift = KeymapKey(DEF, 0, 0, OSM(MOD_LSFT)), osl = KeymapKey(DEF, 0, 2, OSL(SYM));
    auto arrow = KeymapKey(SYM, 2, 6, SYM_ARROW);
    set_keymap({shift, osl, arrow});
    tap_key(shift, 40); tap_key(osl, 40);
    EXPECT_EQ(timeless_active_indicators(), TIMELESS_SYMBOL | TIMELESS_SHIFT);
    tap_key(arrow, 40);
    EXPECT_EQ(timeless_active_indicators(), 0);
    EXPECT_EQ(presses, (std::vector<Press>{{KC_MINS, 0}, {KC_DOT, MOD_BIT(KC_LSFT)}}));
    expect_clear();
}
