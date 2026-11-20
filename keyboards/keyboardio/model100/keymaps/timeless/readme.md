# Model 100 timeless

The base layout uses QWERTY, native Chordal Hold, Flow Tap and Speculative Hold.
The home-row timing and modifiers are in `config.h` and `timeless.c`.

## Home-row modifiers

All eight home-row modifiers use `PERMISSIVE_HOLD` with Chordal Hold, following
Getreuer's guide and the source layout's balanced hold behavior. Within the
tapping term, holding D and tapping U sends Command+U when U is released.
Releasing D before U instead types `du`, protecting overlapping letter presses.
Chordal Hold also resolves same-hand letter pairs as taps within the tapping
term. Outside Flow Tap, holding a key past the tapping term selects its modifier.

The native tap-hold features follow [Getreuer's approach](https://github.com/getreuer/qmk-keymap/blob/f830174ba88dae5f649f16950f3a139021c9a5ca/config_getreuer.h).
The user's original 280 ms tapping term and 220 ms Flow Tap window apply to
all eight home-row keys, including both Shift keys:

| Keys | Modifier | Tapping term | Flow Tap |
| --- | --- | --- | --- |
| A / ; | Ctrl | 280 ms | 220 ms |
| S / L | Alt | 280 ms | 220 ms |
| D / K | Command | 280 ms | 220 ms |
| F / J | Shift | 280 ms | 220 ms |

Native QMK Flow Tap applies after a letter (including a tapped home-row key),
Space, comma, period, semicolon or slash. It does not apply after a number or
while Ctrl, Command or left Alt is active. Within its window, it sends the
letter right away even if the key stays held. Outside that window, Permissive
Hold and Chordal Hold still decide taps and holds.

A wider Flow Tap window protects overlapping letters, but can also turn an
intended home-row modifier into a letter soon after typing. Fn+ASDF supplies
plain Ctrl, Alt, Command and Shift for shortcuts during that window.

Speculative Hold uses Getreuer's separate 200 ms typing guard and allows only
one speculative modifier. Quick Tap uses 175 ms for both hands.
These values differ from Getreuer's personal values.
Key positions and selection-repeat timing do not change.

This branch retains the QMK tap-hold fix in `quantum/action_tapping.c`: queued
key releases must not become the next pending tap-hold press. With
`HOLD_ON_OTHER_KEY_PRESS` enabled, holding D, pressing F, releasing D, then
pressing U could leave Command stuck. Keep this fix when updating QMK until
upstream includes the same correction. Its core regression test still runs
with that option enabled, even though this keymap now uses Permissive Hold.
The queue also retries an event once Chordal Hold clears the pending key. This
keeps a new physical release from overtaking its queued press and leaving a
letter held. Both fixes have regression tests; the input-sequence test also
checks that every key and modifier clears after all physical keys are released.

A third fix checks Flow Tap when a released tap-hold key is pressed again
after an intervening key. Previously, `F M F` could skip Flow Tap on the last F
while the first tap was still within its tapping term. Tapping Space before
releasing that final F then sent Shift+Space instead of F followed by Space.
In Dubeolsik, that can turn `를` into `르`. The fix keeps Quick Tap's existing
behavior when no other key intervenes and still respects Flow Tap eligibility
and its time limit. Native QMK and Model 100 tests cover this case, its release
orders and shortcuts where Flow Tap must stay off. The Korean tests check the
HID key sequences for 을/를/들; they do not run the host's Korean input method.

## Layer controls

- Both palm keys hold Fn. Both symbol thumbs provide one-shot Sym.
- Both outer thumb keys are one-shot Shift in the base and Num layers.
- In Fn and Sym, both outer thumbs lock or unlock the current layer. The Caps
  position also does this in both layers; G remains an extra Fn lock key.
- Fn+F is a plain Shift, including while Fn is locked, for arrow selection.
- Fn+E selects a word; Fn+R selects a line with macOS editing shortcuts.
- Num keeps its toggle and never exits automatically.

## Automatic unlock

Only locked Fn and Sym use these rules. Holding either palm preserves ordinary
Fn behavior; holding either symbol thumb preserves ordinary symbol behavior.
The exit key is processed by the resulting layer, including native tap/hold
handling, rather than discarded. Num remains on if it was already on.

| Locked layer | Stays active | Unlocks and processes the same key in the resulting layer |
| --- | --- | --- |
| Fn | Arrows, word/line/page movement, word/line selection, modifiers, undo/cut/copy/paste, Delete, Fn backslash/pipe, layer and lighting controls | Ordinary letters/punctuation, number row, Space/Enter/Tab/Esc |
| Sym | Symbols, the `->` macro, Backspace, modifiers, layer and lighting controls | Digits, Space/Enter/Tab/Esc, any ordinary letters inherited from lower layers |

Locked Fn therefore differs from held Fn at the number row (digits instead of
F1–F10), the Space thumb (Space instead of Enter), and the backtick position
(backtick instead of Tab). F11/F12 remain available while locked. Fn+Backspace
continues to send Delete and keep the lock.

Keys with an assigned layer action still perform it: HJKL remain arrows, ASDF
remain modifiers, E/R select words/lines, N/M remain Page Up/Down, and A in Sym remains `!`. To start a
word with one of these letters, unlock manually first.

If both Fn and Sym are locked, an exit key can unlock both, from the top down.
Manual lock keys only unlock the top layer. Keys held before an automatic exit
keep their original release action, so arrows and modifiers do not stick.

Purple on the outer thumbs and Caps position indicates a layer lock. With no
lock, the outer thumbs show yellow for pending one-shot Shift. RGB must be on
for these indicators to be visible.

No combos are enabled.

## Word and line selection (macOS)

- Fn+E selects the word at the cursor. Repeated taps extend by one word.
- Holding E extends by one word after 400 ms, then every 150 ms. Tune
  `SELECT_WORD_REPEAT_DELAY` and `SELECT_WORD_REPEAT_INTERVAL` in `config.h`.
  Releasing E, pressing any other key, or leaving Fn stops the repeat.
  After an interruption it stays stopped until E is pressed again.
- Fn+R selects the current line. Repeated taps extend to the next line's end.
  Holding R does not repeat.
- Both work with held or locked Fn. They keep Fn locked.
- Each selection step finishes its hotkeys immediately, including hold repeats.
  This lets E/R overlap with the next Copy/Cut/Paste key without a stuck arrow
  or selection modifier. Physically held modifiers are restored afterwards.
- These two keys have fixed meanings even with Shift held. A pending one-shot
  modifier is consumed; Caps Word ends. Fn+E/R do not type E/R or auto-unlock.
- Other typing, navigation, or editing starts the next selection afresh.
  Modifier and layer keys do not reset it. A gap of `SELECT_WORD_TIMEOUT`
  (5000 ms) between selection taps also resets it; this does not deselect text.
- The keyboard cannot see mouse selections. After moving the cursor with a
  mouse, wait five seconds or use an arrow before starting a new selection.
- Word boundaries and wrapped-line behavior depend on the application. These
  are standard Mac text-editing shortcuts, not Vim normal-mode commands.

`selection.c` adapts the Mac forward-selection sequences from
[Getreuer's Select Word](https://getreuer.info/posts/keyboards/select-word/),
pinned to `getreuer/qmk-modules` commit
`8c55ac1c5d547d1ff324ae2834b26f2075222c97`. It keeps only the two requested actions
and emits complete taps for each step, including timed word repeats. The source is included in
the keymap so a normal build needs no extra module checkout. Its Apache-2.0
license is in `LICENSE.select-word`.

## Build and test

```sh
qmk compile -kb keyboardio/model100 -km timeless
qmk test-c -t model100_timeless -t 'tap_hold_configurations/chordal_hold/*' \
  -t tap_hold_configurations/flow_tap -t tap_hold_configurations/quick_tap \
  -t tap_hold_configurations/speculative_hold/flow_tap -j 4
```
