// SPDX-FileCopyrightText: 2026 mancelli/shadPS4 fork
// SPDX-License-Identifier: GPL-2.0-or-later

// Stand-alone model of UpdatePressedKeys() from src/input/input_handler.cpp, used to prove the
// two-gamepad stuck-button bug (fixed in commit 428ad3b). See two-gamepads-stuck-buttons.md.
//
// Build and run:  g++ -std=c++20 -O2 two-gamepads-sim.cpp -o sim && ./sim
//
// The model keeps one sorted list of everything held on all gamepads, like the emulator. Axis
// events find their entry with an ordering that includes the gamepad id; button-down events used
// an ordering without it (old code). With two gamepads the list stops being sorted, lower_bound
// misses existing entries and duplicates appear. After every button is released and every axis
// is back at 0, a correct implementation leaves nothing held.

#include <algorithm>
#include <compare>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <random>
#include <tuple>

enum class InputType { Axis, KeyboardMouse, Controller, Count };

struct InputID {
    InputType type;
    unsigned sdl_id;
    unsigned char gamepad_id;
    bool operator==(const InputID& o) const {
        return type == o.type && sdl_id == o.sdl_id && gamepad_id == o.gamepad_id;
    }
    // Same order as the emulator's InputID: gamepad id first.
    auto operator<=>(const InputID& o) const {
        return std::tie(gamepad_id, type, sdl_id) <=> std::tie(o.gamepad_id, o.type, o.sdl_id);
    }
};

struct InputEvent {
    InputID input;
    bool active;
    signed char axis_value;
};

static std::list<std::pair<InputEvent, bool>> pressed_keys;
static bool g_fixed = false;

static bool Update(InputEvent event) {
    const InputID input = event.input;
    if (input.type == InputType::Axis) {
        auto it = std::lower_bound(pressed_keys.begin(), pressed_keys.end(), input,
                                   [](auto& e, InputID i) { return e.first.input < i; });
        if (it == pressed_keys.end() || it->first.input != input) {
            pressed_keys.insert(it, {event, false});
        } else {
            if (std::abs(it->first.axis_value - event.axis_value) <= 1) {
                return false;
            }
            it->first.axis_value = event.axis_value;
        }
        return true;
    }
    if (event.active) {
        // Old code: ordering without gamepad_id. Fixed code: the same ordering as axes.
        auto it = g_fixed
                      ? std::lower_bound(pressed_keys.begin(), pressed_keys.end(), input,
                                         [](auto& e, InputID i) { return e.first.input < i; })
                      : std::lower_bound(pressed_keys.begin(), pressed_keys.end(), input,
                                         [](auto& e, InputID i) {
                                             return std::tie(e.first.input.type, e.first.input.sdl_id) <
                                                    std::tie(i.type, i.sdl_id);
                                         });
        if (it == pressed_keys.end() || it->first.input != input) {
            pressed_keys.insert(it, {event, false});
            return true;
        }
        return false;
    }
    auto it = std::find_if(pressed_keys.begin(), pressed_keys.end(),
                           [&](auto& e) { return e.first.input == input; });
    if (it != pressed_keys.end()) {
        pressed_keys.erase(it);
        return true;
    }
    return false;
}

struct Left {
    int buttons; // button entries still in the list (button held)
    int axes;    // stick/trigger entries still away from rest (|value| > 1, the noise filter)
};

// Random presses on `gamepads` controllers, then release everything. Returns what is left behind.
static Left Run(bool fixed, unsigned seed, int gamepads) {
    g_fixed = fixed;
    pressed_keys.clear();
    std::mt19937 rng(seed);
    bool held[3][16]{};
    for (int step = 0; step < 200000; step++) {
        const int gp = 1 + static_cast<int>(rng() % gamepads);
        if (rng() % 2) {
            const int b = rng() % 16;
            held[gp][b] = !held[gp][b];
            Update({{InputType::Controller, static_cast<unsigned>(b), static_cast<unsigned char>(gp)},
                    held[gp][b], 0});
        } else {
            const int a = rng() % 6;
            int v = (rng() % 3 == 0) ? 0 : static_cast<int>(rng() % 128);
            if (a < 4) {
                v -= 64; // sticks are signed, triggers are not
            }
            Update({{InputType::Axis, static_cast<unsigned>(a), static_cast<unsigned char>(gp)}, true,
                    static_cast<signed char>(v)});
        }
    }
    for (int gp = 1; gp <= gamepads; gp++) {
        for (int b = 0; b < 16; b++) {
            if (held[gp][b]) {
                Update({{InputType::Controller, static_cast<unsigned>(b),
                         static_cast<unsigned char>(gp)},
                        false, 0});
            }
        }
        for (int a = 0; a < 6; a++) {
            Update({{InputType::Axis, static_cast<unsigned>(a), static_cast<unsigned char>(gp)}, true,
                    0});
        }
    }
    Left left{};
    for (auto& [e, f] : pressed_keys) {
        if (e.input.type == InputType::Controller) {
            left.buttons++;
        } else if (e.input.type == InputType::Axis && std::abs(e.axis_value) > 1) {
            left.axes++;
        }
    }
    return left;
}

int main() {
    std::printf("held entries left after releasing everything, buttons/axes (0/0 = correct)\n");
    std::printf("seed  old, 1 gamepad  old, 2 gamepads  fixed, 2 gamepads\n");
    for (unsigned s = 1; s <= 5; s++) {
        const Left a = Run(false, s, 1), b = Run(false, s, 2), c = Run(true, s, 2);
        std::printf("%4u  %9d/%-4d  %10d/%-4d  %12d/%-4d\n", s, a.buttons, a.axes, b.buttons,
                    b.axes, c.buttons, c.axes);
    }
}
