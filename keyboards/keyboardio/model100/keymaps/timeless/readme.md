# Model 100 timeless

The base layout uses QWERTY, native Chordal Hold, Flow Tap and Speculative Hold.
The home-row timing and modifiers are in `config.h` and `timeless.c`.

## Layer controls

- Both palm keys hold Fn. Both symbol thumbs provide one-shot Sym.
- Both outer thumb keys are one-shot Shift in the base and Num layers.
- In Fn and Sym, both outer thumbs lock or unlock the current layer. The Caps
  position also does this in both layers; G remains an extra Fn lock key.
- Fn+F is a plain Shift, including while Fn is locked, for arrow selection.
- Num keeps its toggle and never exits automatically.

## Automatic unlock

Only locked Fn and Sym use these rules. Holding either palm preserves ordinary
Fn behavior; holding either symbol thumb preserves ordinary symbol behavior.
The exit key is processed by the resulting layer, including native tap/hold
handling, rather than discarded. Num remains on if it was already on.

| Locked layer | Stays active | Unlocks and processes the same key in the resulting layer |
| --- | --- | --- |
| Fn | Arrows, word/line/page movement, modifiers, undo/cut/copy/paste, Delete, Fn backslash/pipe, layer and lighting controls | Ordinary letters/punctuation, number row, Space/Enter/Tab/Esc |
| Sym | Symbols, the `->` macro, Backspace, modifiers, layer and lighting controls | Digits, Space/Enter/Tab/Esc, any ordinary letters inherited from lower layers |

Locked Fn therefore differs from held Fn at the number row (digits instead of
F1–F10), the Space thumb (Space instead of Enter), and the backtick position
(backtick instead of Tab). F11/F12 remain available while locked. Fn+Backspace
continues to send Delete and keep the lock.

Keys with an assigned layer action still perform it: HJKL remain arrows, ASDF
remain modifiers, N/M remain Page Up/Down, and A in Sym remains `!`. To start a
word with one of these letters, unlock manually first.

If both Fn and Sym are locked, an exit key can unlock both, from the top down.
Manual lock keys only unlock the top layer. Keys held before an automatic exit
keep their original release action, so arrows and modifiers do not stick.

Purple on the outer thumbs and Caps position indicates a layer lock. With no
lock, the outer thumbs show yellow for pending one-shot Shift. RGB must be on
for these indicators to be visible.

No combos are enabled.

## Build and test

```sh
qmk compile -kb keyboardio/model100 -km timeless
qmk test-c -t model100_timeless -j 4
```
