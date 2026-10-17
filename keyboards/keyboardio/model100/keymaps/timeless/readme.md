# Model 100 timeless

The base layout uses QWERTY, native Chordal Hold, Flow Tap and Speculative Hold.
The home-row timing and modifiers are in `config.h` and `timeless.c`.

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
qmk test-c -t model100_timeless -j 4
```
