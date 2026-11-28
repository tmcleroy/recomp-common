// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
// The menu's SDL controls (menu_sdl.h): its keys, and the mouse's events made into the picture's places.
#include "hotkey.h"
#include "menu_sdl.h"

#include <cmath>
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
    // (the menu turned a quarter for a monitor on its side: its 240 x 320 picture turned onto the window's 640 x 480
    // from 100, 50; its rows run across the window, top at the right, and a click there is mapped back through the turn)
    {
        std::vector<uint32_t> own(240 * 320);
        const sdl::MenuPlace turned{100, 50, 640, 480, 240, 320, 90};
        bool hit = false;
        for (float x = 740; x > 100 && !hit; x -= 1) {
            SDL_Event click{};
            click.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
            click.button.button = SDL_BUTTON_LEFT;
            click.button.x = x;
            click.button.y = 290;
            Menu::Chosen chosen;
            Menu probe;
            probe.Configure(config);
            probe.Open();
            probe.Draw(own.data(), 240, 320);
            CHECK(sdl::MouseEvent(click, nullptr, turned, probe, chosen));
            hit = chosen.kind == Menu::Chosen::Load;
        }
        CHECK(hit);
        // (a corner of the window back to the menu's picture: the window's top right is the menu's top left)
        float mx = -1, my = -1;
        turned.ToMenu(740, 50, mx, my);
        CHECK(std::fabs(mx) < 0.01f && std::fabs(my) < 0.01f);
    }
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
    {
        // Hotkeys: kept as a line and read back; asked for, refused, given, taken away; stepping in the game
        Hotkeys keys;
        keys.Set("run_ahead", {SDL_SCANCODE_F3, SDL_GAMEPAD_BUTTON_INVALID});
        keys.Set("shader", {SDL_SCANCODE_UNKNOWN, SDL_GAMEPAD_BUTTON_RIGHT_STICK});
        Hotkeys back;
        back.Load(keys.Save() + ",nothing:key:NoSuchKey,broken");
        CHECK(back.Save() == keys.Save() && back.Of("run_ahead").Name() == "F3" && back.Of("shader").Name() == "Right stick");
        CHECK(back.Find({SDL_SCANCODE_F3, SDL_GAMEPAD_BUTTON_INVALID}) == "run_ahead");
        back.Set("other", {SDL_SCANCODE_F3, SDL_GAMEPAD_BUTTON_INVALID}); // (one key, one feature)
        CHECK(back.Find({SDL_SCANCODE_F3, SDL_GAMEPAD_BUTTON_INVALID}) == "other" && back.Of("run_ahead").Empty());

        int value = 0;
        MenuConfig c;
        c.tabs = {{"Game", {"Game"}}};
        c.features = {{"Game", "Run-ahead", "", {"Off", "1 frame", "2 frames"}}};
        c.value = [&](size_t) { return value; };
        c.setValue = [&](size_t, int to) { value = to; };
        Hotkeys mine;
        c.hotkey = [&](size_t) { return mine.Of("run_ahead").Name(); };
        const std::vector<std::string> ids = {"run_ahead"};
        Menu m;
        m.Configure(c);
        m.Open();
        m.Buttons(kMenuHotkey);
        CHECK(m.Listening() == 0);
        SDL_Event key{};
        key.type = SDL_EVENT_KEY_DOWN;
        key.key.scancode = SDL_SCANCODE_W;
        bool changed = false;
        const auto taken = [](Hotkey h) { return h.key == SDL_SCANCODE_W ? std::string("That key plays the game.") : std::string(); };
        CHECK(sdl::HotkeyListen(key, m, mine, ids, taken, changed) && !changed && m.Listening() == 0);
        key.key.scancode = SDL_SCANCODE_DOWN; // (the menu's)
        CHECK(sdl::HotkeyListen(key, m, mine, ids, taken, changed) && !changed && m.Listening() == 0);
        key.key.scancode = SDL_SCANCODE_F3;
        CHECK(sdl::HotkeyListen(key, m, mine, ids, taken, changed) && changed && m.Listening() < 0 && mine.Of("run_ahead").Name() == "F3");
        CHECK(!sdl::HotkeyListen(key, m, mine, ids, taken, changed)); // (not asking now)
        m.Close();
        CHECK(sdl::HotkeyPress(key, m, mine, ids) == "Run-ahead: 1 frame" && value == 1);
        key.key.mod = SDL_KMOD_LSHIFT;
        CHECK(sdl::HotkeyPress(key, m, mine, ids) == "Run-ahead: Off");
        key.key.scancode = SDL_SCANCODE_F4;
        CHECK(sdl::HotkeyPress(key, m, mine, ids).empty());
        m.Open();
        m.Buttons(kMenuHotkey);
        key.key.scancode = SDL_SCANCODE_BACKSPACE;
        changed = false;
        CHECK(sdl::HotkeyListen(key, m, mine, ids, taken, changed) && changed && mine.Of("run_ahead").Empty());
    }
    puts("menu_sdl_test: all as it should be");
    return 0;
}
