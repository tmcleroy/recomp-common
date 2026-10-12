// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
// The menu's rules, without a picture to compare: what rows there are, what the buttons do, and that
// drawing stays inside the picture at any size.
#include "menu.h"

#include <cstdio>
#include <cstdlib>
#include <map>

using namespace recomp;

#define CHECK(what)                                                    \
    do {                                                               \
        if (!(what)) {                                                 \
            fprintf(stderr, "%s:%d: not so: %s\n", __FILE__, __LINE__, #what); \
            exit(1);                                                   \
        }                                                              \
    } while (0)

namespace {
    std::map<size_t, int> g_values;

    MenuConfig Config() {
        MenuConfig c;
        c.groups = {
            {{MenuRow::Save}, {MenuRow::Load}, {MenuRow::Slot}},
            {{MenuRow::Pages}, {MenuRow::Speed}, {MenuRow::Action, 7, "Insert coin", "Puts a coin in."}},
            {{MenuRow::Action, 9, "Reset", "Starts again."}, {MenuRow::Quit}, {MenuRow::Resume}},
        };
        c.features = {
            {"Cheats", "Endless time", "The clock stands still."},
            {"Cheats", "Health", "Whose health is kept full.", {"Off", "Player 1", "Both"}},
            {"Picture", "Wide", "A wider picture."},
        };
        c.commands = {
            {"", "Start over", "Back to where the round began."},
            {"Training", "Begin", "Starts a session."},
        };
        c.value = [](size_t feature) { return g_values[feature]; };
        c.setValue = [](size_t feature, int value) { g_values[feature] = value; };
        c.speeds = {"Full", "1/2", "1/4"};
        c.speedDescription = "Left/Right: slow motion.";
        return c;
    }

    Menu::Chosen Press(Menu &menu, std::initializer_list<unsigned> presses) {
        Menu::Chosen chosen;
        for (unsigned press : presses) {
            chosen = menu.Buttons(press);
        }
        return chosen;
    }
} // namespace

int main() {
    // First page: Save, Load, Slot | Start over, Cheats, Picture, Training, Speed, Insert coin | Reset, Quit, Resume
    Menu menu;
    menu.Configure(Config());
    CHECK(Press(menu, {kMenuConfirm}).kind == Menu::Chosen::None); // (closed: nothing)
    menu.Open();
    CHECK(menu.IsOpen());
    CHECK(Press(menu, {kMenuConfirm}).kind == Menu::Chosen::Save);
    CHECK(Press(menu, {kMenuDown, kMenuConfirm}).kind == Menu::Chosen::Load);

    // The slot: 1-9, around, by Left/Right and Confirm
    Press(menu, {kMenuDown, kMenuLeft});
    CHECK(menu.Slot() == 9);
    Press(menu, {kMenuRight, kMenuConfirm});
    CHECK(menu.Slot() == 2);

    // A command of no category is on the first page
    Menu::Chosen chosen = Press(menu, {kMenuDown, kMenuConfirm});
    CHECK(chosen.kind == Menu::Chosen::Command && chosen.id == 0);

    // The Cheats page: an on/off feature changes by Confirm only, a choice by Left/Right (to its ends) and
    // Confirm (around)
    Press(menu, {kMenuDown, kMenuConfirm});
    Press(menu, {kMenuRight});
    CHECK(g_values[0] == 0);
    Press(menu, {kMenuConfirm});
    CHECK(g_values[0] == 1);
    Press(menu, {kMenuDown, kMenuRight, kMenuRight, kMenuRight});
    CHECK(g_values[1] == 2);
    Press(menu, {kMenuConfirm});
    CHECK(g_values[1] == 0);
    Press(menu, {kMenuLeft});
    CHECK(g_values[1] == 0);
    // Back (the row after the two features), and the first page is where it was left
    Press(menu, {kMenuDown, kMenuConfirm});
    // ... on the Cheats row: two down is Training, whose page has the command
    Press(menu, {kMenuDown, kMenuDown, kMenuConfirm});
    chosen = Press(menu, {kMenuConfirm});
    CHECK(chosen.kind == Menu::Chosen::Command && chosen.id == 1);
    Press(menu, {kMenuDown, kMenuLeft}); // (Left on a row with no values: the first page)

    // Speed: Left/Right to the ends, Confirm around
    Press(menu, {kMenuDown, kMenuRight, kMenuRight, kMenuRight});
    CHECK(menu.Speed() == 2);
    Press(menu, {kMenuConfirm});
    CHECK(menu.Speed() == 0);
    Press(menu, {kMenuLeft});
    CHECK(menu.Speed() == 0);

    // The host's rows hand back their ids
    chosen = Press(menu, {kMenuDown, kMenuConfirm});
    CHECK(chosen.kind == Menu::Chosen::Action && chosen.id == 7);
    chosen = Press(menu, {kMenuDown, kMenuConfirm});
    CHECK(chosen.kind == Menu::Chosen::Action && chosen.id == 9);
    CHECK(Press(menu, {kMenuDown, kMenuConfirm}).kind == Menu::Chosen::Quit);
    CHECK(menu.IsOpen());
    Press(menu, {kMenuDown, kMenuConfirm}); // Resume
    CHECK(!menu.IsOpen());

    // Opened again on the row it was closed on; Down from the last row is the first; Back closes
    menu.Open();
    CHECK(Press(menu, {kMenuDown, kMenuConfirm}).kind == Menu::Chosen::Save);
    Press(menu, {kMenuBack});
    CHECK(!menu.IsOpen());

    // With no speeds there is no Speed row; with nothing at all, nothing happens
    MenuConfig bare = Config();
    bare.speeds.clear();
    bare.groups = {{{MenuRow::Speed}, {MenuRow::Resume}}};
    Menu small;
    small.Configure(bare);
    small.Open();
    Press(small, {kMenuConfirm});
    CHECK(!small.IsOpen());
    Menu empty;
    empty.Open();
    CHECK(Press(empty, {kMenuConfirm}).kind == Menu::Chosen::None);

    // Drawing stays in the picture: pictures of many sizes with a guard either side (and the address
    // sanitizer the test is built with)
    for (int height : {1, 100, 224, 240, 448, 672, 720, 1080}) {
        for (int width : {1, 256, 320, 896, 1920}) {
            const size_t count = static_cast<size_t>(width) * height;
            std::vector<uint32_t> pixels(count + 2 * 64, 0x12345678);
            for (int page = 0; page < 2; page++) {
                Menu drawn;
                drawn.Configure(Config());
                drawn.Open();
                if (page) {
                    Press(drawn, {kMenuDown, kMenuDown, kMenuDown, kMenuDown, kMenuConfirm});
                }
                drawn.Note(page ? "" : "Nothing is kept in slot 1.");
                drawn.Draw(pixels.data() + 64, width, height);
            }
            for (size_t i = 0; i < 64; i++) {
                CHECK(pixels[i] == 0x12345678 && pixels[64 + count + i] == 0x12345678);
            }
            CHECK(pixels[64] != 0x12345678);
        }
    }
    puts("menu_test: all as it should be");
    return 0;
}
