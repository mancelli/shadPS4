<!--
SPDX-FileCopyrightText: 2026 mancelli/shadPS4 fork
SPDX-License-Identifier: GPL-2.0-or-later
-->

# Two gamepads: a trigger or button stays held until the game restarts

- Date: 2026-10-13. Fixed in 428ad3b (`src/input/input_handler.cpp`), in releases since
  2026-10-14.
- Checked with a stand-alone simulation of the function ([two-gamepads-sim.cpp](two-gamepads-sim.cpp)).
  A two-player check in the game after the fix is not recorded here.

## Report

Two players on one PC: sooner or later one player's block (R2 in UFC) stays pressed and cannot be
released until the game is restarted. With one gamepad it never happens. The bug was old and also
present in Shadlix.

## Cause [VERIFIED]

`UpdatePressedKeys()` keeps one list, `pressed_keys`, of everything held on all gamepads. The list
is kept sorted and searched with `std::lower_bound`, which only works on a sorted list. Two
different orderings were used:

- axis events (sticks, L2/R2 triggers) used `InputID`'s ordering: gamepad id, then type, then id;
- button-down events used an ordering of type and id **without the gamepad id**.

With one gamepad both orderings agree. With two, button inserts land in places that break the axis
ordering, so the list is no longer sorted. The binary search then misses an existing axis entry,
for example "R2 of player 2", and inserts a second one. When the trigger is released only one copy
goes back to 0; the other keeps the old "fully pressed" value, and the emulator keeps reading it.

## Fix

Button-down events use the same ordering as axes (`InputID`'s, gamepad id first). One line; the
comment at `input_handler.cpp:934-938` explains it.

## Simulation

[two-gamepads-sim.cpp](two-gamepads-sim.cpp) copies the list logic (including the noise filter
that ignores axis changes of ±1), sends 200,000 random button and axis events from one or two
gamepads, then releases every button and returns every axis to 0. A correct implementation leaves
nothing held. Output (`g++ -std=c++20 -O2 two-gamepads-sim.cpp -o sim && ./sim`):

```
held entries left after releasing everything, buttons/axes (0/0 = correct)
seed  old, 1 gamepad  old, 2 gamepads  fixed, 2 gamepads
   1          0/0              0/48               0/0
   2          0/0              0/52               0/0
   3          0/0              0/63               0/0
   4          0/0              0/48               0/0
   5          0/0              0/72               0/0
```

The stale entries are axes, which matches the report: in UFC the block is on a trigger.

## Related

If the emulator cannot match an SDL joystick ID to a gamepad, it falls back to index 0 (the first
player) and logs `Gamepad with joystick ID ... not found, returning 0 as fallback`
(`src/input/controller.cpp:709`). That is a different problem and does not cause stuck inputs.
