// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
// The menu's gamepad buttons, as the player chose them (menu_buttons.h): the launcher's keys, the port's own choice
// (built once as recomp-common's and once with RECOMP_MENU_BUTTON_DEFAULT="guide", as a port sets it), and what a game
// started by the launcher reads. Which buttons each choice opens the menu with: tests/menu_sdl_test.cpp (SDL 3).
#include "menu_buttons.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace recomp;

#define CHECK(what)                                                                     \
    do {                                                                                \
        if (!(what)) {                                                                  \
            fprintf(stderr, "%s:%d: not so: %s\n", __FILE__, __LINE__, #what);         \
            exit(1);                                                                    \
        }                                                                               \
    } while (0)

#ifndef EXPECTED_DEFAULT
#define EXPECTED_DEFAULT MenuButtons::GuideStick
#endif

static void SetEnvironment(const char *value) {
#ifdef _WIN32
    _putenv_s("RECOMP_MENU_BUTTON", value ? value : "");
#else
    if (value) {
        setenv("RECOMP_MENU_BUTTON", value, 1);
    } else {
        unsetenv("RECOMP_MENU_BUTTON");
    }
#endif
}

int main() {
    // (each choice by its key, and back)
    for (const MenuButtonsChoice &choice : kMenuButtonsChoices) {
        CHECK(ParseMenuButtons(choice.key, MenuButtons::Chord) == choice.value);
        CHECK(std::strcmp(MenuButtonsKey(choice.value), choice.key) == 0);
    }
    CHECK(ParseMenuButtons("guide-stick", MenuButtons::Chord) == MenuButtons::GuideStick);
    CHECK(ParseMenuButtons("guide", MenuButtons::Chord) == MenuButtons::Guide);
    CHECK(ParseMenuButtons("chord", MenuButtons::Guide) == MenuButtons::Chord);
    // (nothing, or a key not known: what is given instead)
    CHECK(ParseMenuButtons(nullptr, MenuButtons::Guide) == MenuButtons::Guide);
    CHECK(ParseMenuButtons("", MenuButtons::Chord) == MenuButtons::Chord);
    CHECK(ParseMenuButtons("Guide", MenuButtons::GuideStick) == MenuButtons::GuideStick);
    // (the port's own, and the game's with the launcher's word or without it)
    CHECK(DefaultMenuButtons() == EXPECTED_DEFAULT);
    SetEnvironment(nullptr);
    CHECK(MenuButtonsFromEnvironment() == EXPECTED_DEFAULT);
    SetEnvironment("chord");
    CHECK(MenuButtonsFromEnvironment() == MenuButtons::Chord);
    SetEnvironment("guide-stick");
    CHECK(MenuButtonsFromEnvironment() == MenuButtons::GuideStick);
    SetEnvironment("nonsense");
    CHECK(MenuButtonsFromEnvironment() == EXPECTED_DEFAULT);
    puts("menu_buttons_test: all as it should be");
    return 0;
}
