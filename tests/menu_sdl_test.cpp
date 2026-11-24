// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
// The menu's SDL controls (menu_sdl.h): its keys, and the mouse's events made into the picture's places.
#include "menu_sdl.h"

#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace recomp;

#define CHECK(what)                                                                     \
    do {                                                                                \
        if (!(what)) {                                                                  \
            fprintf(stderr, "%s:%d: not so: %s\n", __FILE__, __LINE__, #what);         \
            exit(1);                                                                    \
        }                                                                               \
    } while (0)

int main() {
    CHECK(sdl::MenuKey(SDL_SCANCODE_ESCAPE) && sdl::MenuKey(SDL_SCANCODE_F1) && sdl::MenuKey(SDL_SCANCODE_TAB));
    CHECK(!sdl::MenuKey(SDL_SCANCODE_TAB, false) && !sdl::MenuKey(SDL_SCANCODE_A));
    CHECK(sdl::MenuButton(SDL_GAMEPAD_BUTTON_GUIDE) && !sdl::MenuButton(SDL_GAMEPAD_BUTTON_SOUTH));
    // (the gamepad's buttons that open the menu by themselves, for each of the player's choices: Start and Back are
    // the chord's, whatever the choice)
    for (int b = 0; b < SDL_GAMEPAD_BUTTON_COUNT; b++) {
        const auto button = static_cast<SDL_GamepadButton>(b);
        const bool guide = button == SDL_GAMEPAD_BUTTON_GUIDE, stick = button == SDL_GAMEPAD_BUTTON_LEFT_STICK;
        CHECK(sdl::MenuButton(button, MenuButtons::GuideStick) == (guide || stick));
        CHECK(sdl::MenuButton(button, MenuButtons::Guide) == guide);
        CHECK(!sdl::MenuButton(button, MenuButtons::Chord));
    }
    CHECK(!sdl::MenuButton(SDL_GAMEPAD_BUTTON_INVALID, MenuButtons::GuideStick));
    // (this program has no RECOMP_MENU_BUTTON: recomp-common's own, the left stick's click too)
    CHECK(sdl::MenuButton(SDL_GAMEPAD_BUTTON_LEFT_STICK) == (DefaultMenuButtons() == MenuButtons::GuideStick));
    static bool keys[SDL_SCANCODE_COUNT] = {};
    keys[SDL_SCANCODE_DOWN] = keys[SDL_SCANCODE_BACKSPACE] = true;
    CHECK(sdl::MenuHeld(keys, nullptr, 0) == (kMenuDown | kMenuBack));

    MenuConfig config;
    config.groups = {{{MenuRow::Save}, {MenuRow::Load}}};
    Menu menu;
    menu.Configure(config);
    menu.Open();
    // (the picture is 320x240, shown at twice its size from 100, 50 of the window)
    std::vector<uint32_t> picture(320 * 240);
    menu.Draw(picture.data(), 320, 240);
    const sdl::MenuPlace place{100, 50, 640, 480, 320, 240};
    // (Load, somewhere down the middle of the window)
    bool found = false;
    for (float y = 50; y < 530 && !found; y += 1) {
        SDL_Event click{};
        click.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        click.button.button = SDL_BUTTON_LEFT;
        click.button.x = 420;
        click.button.y = y;
        Menu::Chosen chosen;
        Menu probe;
        probe.Configure(config);
        probe.Open();
        probe.Draw(picture.data(), 320, 240);
        CHECK(sdl::MouseEvent(click, nullptr, place, probe, chosen));
        found = chosen.kind == Menu::Chosen::Load;
    }
    CHECK(found);
    // (a touch the system made into a click: not the menu's)
    SDL_Event touch{};
    touch.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    touch.button.which = SDL_TOUCH_MOUSEID;
    touch.button.button = SDL_BUTTON_LEFT;
    Menu::Chosen chosen;
    CHECK(!sdl::MouseEvent(touch, nullptr, place, menu, chosen));
    // (the right button: Back, which closes the first page)
    SDL_Event right{};
    right.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    right.button.button = SDL_BUTTON_RIGHT;
    CHECK(sdl::MouseEvent(right, nullptr, place, menu, chosen) && !menu.IsOpen());
    puts("menu_sdl_test: all as it should be");
    return 0;
}
