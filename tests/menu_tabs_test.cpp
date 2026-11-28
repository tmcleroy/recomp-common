// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
// The tabbed menu's rules (MenuConfig::tabs, layout::Tabbed): which tabs there are, the three places the buttons go
// through (the command row, the tabs, the rows), the second press Reset and Quit want, Default, Back, the mouse, and
// that both looks stay inside the picture at any size.
#include "menu.h"
#include "menu_layout.h"

#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

using namespace recomp;

#define CHECK(what)                                                                     \
    do {                                                                                \
        if (!(what)) {                                                                  \
            fprintf(stderr, "%s:%d: not so: %s\n", __FILE__, __LINE__, #what);          \
            exit(1);                                                                    \
        }                                                                               \
    } while (0)

namespace {
    std::map<size_t, int> g_values;
    bool g_muted = false;
    enum { kCoin = 3 };

    // Game (Speed, Mute, Free play, Insert coin, Weird >, Reset all options) · Display (Wide, Shader, Arcade monitor >) ·
    // Training (Begin, Refills, Dummy >) · Cheats (Endless time) ; no Sound, Online, Modes, Stage or Capture
    MenuConfig Config() {
        using namespace recomp::layout;
        MenuConfig c;
        c.features = {
            {kGame, "Free play", "Start works without a coin."},
            {kDisplay, "Wide", "A wider picture."},
            {kDisplay, "Shader", "A filter.", {"Off", "Scanlines", "CRT"}},
            {"Arcade monitor", "Monitor", "The monitor's mode.", {"Off", "15 kHz"}},
            {kTraining, "Refills", "The gauge fills again."},
            {kDummy, "Guards", "What player 2 guards.", {"Never", "All"}},
            {kCheats, "Endless time", "The clock stands still."},
            {"Weird", "Odd one", "On no page a tab has."},
        };
        c.commands = {{kTraining, "Begin", "Starts a session."}};
        c.pages = {{"Arcade monitor", kDisplay, ""}};
        Rows rows;
        rows.actions = {{MenuRow::Action, kCoin, kInsertCoin, "Puts a coin in."}};
        Tabbed(c, rows);
        c.value = [](size_t feature) { return g_values[feature]; };
        c.setValue = [](size_t feature, int value) { g_values[feature] = value; };
        c.defaultValue = [](size_t feature) { return feature == 1 ? 1 : 0; }; // (Wide is on by default)
        Speeds(c);
        c.muted = [] { return g_muted; };
        c.setMuted = [](bool to) { g_muted = to; };
        return c;
    }

    Menu::Chosen Press(Menu &menu, std::initializer_list<unsigned> presses) {
        Menu::Chosen chosen;
        for (unsigned press : presses) {
            chosen = menu.Buttons(press);
        }
        return chosen;
    }

    const std::string &TabName(const Menu &menu, const MenuConfig &config) {
        return config.tabs[static_cast<size_t>(menu.Tab())].name;
    }
} // namespace

int main() {
    const MenuConfig config = Config();
    CHECK(config.tabs.size() == 9);
    CHECK(config.bar.size() == 5); // Save, Load, Slot, Reset, Quit
    {
        // The tabs with something on them, around; opened on the first tab's first row
        g_values = {{1, 1}};
        Menu menu;
        menu.Configure(Config());
        menu.Open();
        CHECK(menu.Tabbed() && TabName(menu, config) == "Game");
        CHECK(Press(menu, {kMenuTabNext}).kind == Menu::Chosen::None && TabName(menu, config) == "Display");
        Press(menu, {kMenuTabNext});
        CHECK(TabName(menu, config) == "Training"); // (Sound and Online have nothing)
        Press(menu, {kMenuTabNext, kMenuTabNext});
        CHECK(TabName(menu, config) == "Game"); // (past Cheats, around)
        Press(menu, {kMenuTabPrev});
        CHECK(TabName(menu, config) == "Cheats");

        // Each tab keeps the row it was left on
        Press(menu, {kMenuTabNext}); // (Game: Speed, Mute, Free play, Insert coin, Weird, Reset all options)
        Press(menu, {kMenuDown, kMenuDown, kMenuConfirm});
        CHECK(g_values[0] == 1); // (Free play on)
        Press(menu, {kMenuTabNext, kMenuTabPrev, kMenuConfirm});
        CHECK(g_values[0] == 0); // (back on Free play: off again)

        // A switch: Left off, Right on (and Confirm, as before, the other way)
        Press(menu, {kMenuRight});
        CHECK(g_values[0] == 1);
        Press(menu, {kMenuRight});
        CHECK(g_values[0] == 1);
        Press(menu, {kMenuLeft});
        CHECK(g_values[0] == 0);

        // Default: the option chosen back to its own default
        Press(menu, {kMenuTabNext, kMenuConfirm}); // (Display: Wide off)
        CHECK(g_values[1] == 0);
        Press(menu, {kMenuDefault});
        CHECK(g_values[1] == 1);
        Press(menu, {kMenuDown, kMenuRight, kMenuRight});
        CHECK(g_values[2] == 2);
        Press(menu, {kMenuDefault});
        CHECK(g_values[2] == 0);

        // A page under a tab's: opened in the tab's place; Back (or Left) comes out of it, onto its row
        Press(menu, {kMenuDown, kMenuConfirm}); // (Arcade monitor)
        Press(menu, {kMenuRight});
        CHECK(g_values[3] == 1);
        Press(menu, {kMenuBack});
        CHECK(menu.IsOpen());
        Press(menu, {kMenuUp, kMenuUp, kMenuConfirm}); // (from Arcade monitor up to Wide)
        CHECK(g_values[1] == 0);
        Press(menu, {kMenuConfirm});

        // Up from the top row: the tabs (Left/Right change tab), then the command row (Left/Right go through it)
        Press(menu, {kMenuUp}); // (from the top row: on the tabs)
        Press(menu, {kMenuRight});
        CHECK(TabName(menu, config) == "Training");
        Press(menu, {kMenuLeft});
        CHECK(TabName(menu, config) == "Display");
        Press(menu, {kMenuUp}); // (the command row, on Save state)
        CHECK(Press(menu, {kMenuConfirm}).kind == Menu::Chosen::Save);
        CHECK(Press(menu, {kMenuRight, kMenuConfirm}).kind == Menu::Chosen::Load);
        Press(menu, {kMenuRight, kMenuConfirm, kMenuConfirm}); // (Slot: 1, 2, 3)
        CHECK(menu.Slot() == 3);
        // Reset game and Quit game: a second press, straight after
        CHECK(Press(menu, {kMenuRight, kMenuConfirm}).kind == Menu::Chosen::None);
        CHECK(menu.Buttons(kMenuConfirm).kind == Menu::Chosen::Reset);
        CHECK(Press(menu, {kMenuRight, kMenuConfirm}).kind == Menu::Chosen::None);
        Press(menu, {kMenuLeft, kMenuRight}); // (moved away and back: asked again)
        CHECK(menu.Buttons(kMenuConfirm).kind == Menu::Chosen::None);
        CHECK(menu.Buttons(kMenuConfirm).kind == Menu::Chosen::Quit);
        // Resume, at the right; Back, at the left, closes where there is no page to go back to
        Press(menu, {kMenuRight, kMenuRight, kMenuConfirm});
        CHECK(!menu.IsOpen());
        menu.Open();
        CHECK(TabName(menu, config) == "Display"); // (as it was left)
        Press(menu, {kMenuUp});                      // (the command row, where it was: Resume)
        Press(menu, {kMenuLeft, kMenuLeft, kMenuLeft, kMenuLeft, kMenuLeft, kMenuLeft, kMenuLeft, kMenuConfirm});
        CHECK(!menu.IsOpen());
        menu.Open();
        Press(menu, {kMenuDown, kMenuDown}); // (the tabs, then the rows)
        Press(menu, {kMenuBack});
        CHECK(!menu.IsOpen());

        // Down at the bottom: the top row again (not into the command row)
        menu.Open();
        Press(menu, {kMenuTabNext}); // (Training: Begin, Refills, Dummy)
        Press(menu, {kMenuDown, kMenuDown, kMenuDown});
        CHECK(Press(menu, {kMenuConfirm}).kind == Menu::Chosen::Command);
    }
    {
        // The Game tab: the port's rows, a page no tab has, and Reset all options, which asks first
        g_values = {{1, 1}, {7, 1}};
        Menu menu;
        menu.Configure(Config());
        menu.Open();
        CHECK(Press(menu, {kMenuDown, kMenuDown, kMenuDown, kMenuConfirm}).kind == Menu::Chosen::Action);
        Press(menu, {kMenuDown, kMenuConfirm, kMenuConfirm}); // (into Weird: Odd one off)
        CHECK(g_values[7] == 0);
        Press(menu, {kMenuBack, kMenuDown, kMenuConfirm}); // (Reset all options: No first)
        CHECK(Press(menu, {kMenuConfirm}).kind == Menu::Chosen::None && menu.IsOpen());
        Press(menu, {kMenuConfirm});
        CHECK(Press(menu, {kMenuDown, kMenuConfirm}).kind == Menu::Chosen::ResetOptions);
    }
    {
        // The mouse, where the menu was last drawn: Resume at the command row's right; the wheel through the rows
        for (const bool crt : {false, true}) {
            g_values = {};
            Menu menu;
            menu.Configure(Config());
            menu.SetCompact(crt);
            menu.Open();
            const int width = crt ? 384 : 1152, height = crt ? 224 : 672;
            std::vector<uint32_t> picture(static_cast<size_t>(width) * height);
            menu.Draw(picture.data(), width, height);
            for (int y = 0; y < height; y += 7) {
                for (int x = 0; x < width; x += 7) {
                    menu.Mouse(MenuMouse::Move, static_cast<float>(x), static_cast<float>(y));
                }
            }
            menu.Mouse(MenuMouse::WheelDown, width / 4.0f, height / 2.0f);
            menu.Draw(picture.data(), width, height);
            // (from the right, along the command row's middle: the first thing there is Resume)
            const float barY = crt ? 8 + 7.0f : 24 + 30.0f;
            for (int x = width - 1; x > width / 2 && menu.IsOpen(); x--) {
                menu.Mouse(MenuMouse::Click, static_cast<float>(x), barY);
            }
            CHECK(!menu.IsOpen());
            // (the right button: Back, so here it closes)
            menu.Open();
            menu.Mouse(MenuMouse::Back, 0, 0);
            CHECK(!menu.IsOpen());
        }
    }
    {
        // Both looks inside the picture at any size, on every tab and page (the sanitizers see a pixel outside)
        for (const bool crt : {false, true}) {
            for (const auto &size : std::vector<std::pair<int, int>>{{384, 224}, {320, 240}, {1152, 672}, {896, 672}, {224, 384}, {2560, 1440}, {96, 64}, {1, 1}}) {
                Menu menu;
                menu.Configure(Config());
                menu.SetCompact(crt);
                menu.Open();
                std::vector<uint32_t> picture(static_cast<size_t>(size.first) * size.second);
                for (int tab = 0; tab < 5; tab++) {
                    for (int row = 0; row < 8; row++) {
                        menu.Draw(picture.data(), size.first, size.second);
                        Press(menu, {kMenuDown});
                    }
                    Press(menu, {kMenuConfirm});
                    menu.Draw(picture.data(), size.first, size.second);
                    Press(menu, {kMenuBack, kMenuTabNext});
                    if (!menu.IsOpen()) {
                        menu.Open();
                    }
                }
                Press(menu, {kMenuUp, kMenuUp, kMenuUp});
                menu.Draw(picture.data(), size.first, size.second);
            }
        }
    }
    {
        // No tabs: the page menu, as it always was
        MenuConfig plain;
        plain.groups = {{{MenuRow::Quit}, {MenuRow::Resume}}};
        Menu menu;
        menu.Configure(plain);
        menu.Open();
        CHECK(!menu.Tabbed());
        CHECK(menu.Buttons(kMenuConfirm).kind == Menu::Chosen::Quit);
        CHECK(menu.Buttons(kMenuTabNext).kind == Menu::Chosen::None && menu.IsOpen());
    }
    puts("menu_tabs_test: all as it should be");
    {
        // Hotkeys: Hotkey on a feature's row asks for one; while asked the buttons are not the menu's, nor the one
        // pressed just after; Step goes around the choices offered, and flips a switch
        g_values = {};
        Menu menu;
        MenuConfig with = Config();
        menu.Configure(Config());
        menu.Open();
        Press(menu, {kMenuTabNext, kMenuDown});
        CHECK(Press(menu, {kMenuHotkey}).kind == Menu::Chosen::None && menu.Listening() < 0); // (no MenuConfig::hotkey)
        with.hotkey = [](size_t feature) { return feature == 2 ? std::string("F3") : std::string(); };
        menu.Configure(with);
        Press(menu, {kMenuHotkey});
        CHECK(menu.Listening() == 2); // Shader
        Press(menu, {kMenuDown, kMenuBack});
        CHECK(menu.IsOpen() && menu.Listening() == 2);
        menu.StopListening();
        Press(menu, {kMenuBack}); // (the key just given: not the menu's)
        CHECK(menu.IsOpen() && menu.Listening() < 0);
        std::vector<uint32_t> pixels(1152 * 672);
        Press(menu, {kMenuHotkey});
        menu.Draw(pixels.data(), 1152, 672); // (the question drawn)
        menu.StopListening();
        menu.Draw(pixels.data(), 1152, 672); // (the row's key drawn)
        menu.Close();
        CHECK(menu.Step(2, 1) == "Shader: Scanlines" && g_values[2] == 1);
        CHECK(menu.Step(2, 1) == "Shader: CRT");
        CHECK(menu.Step(2, 1) == "Shader: Off" && g_values[2] == 0);
        CHECK(menu.Step(2, -1) == "Shader: CRT");
        CHECK(menu.Step(1, 1) == "Wide: On" && menu.Step(1, -1) == "Wide: Off");
        with.offered = [](size_t, int choice) { return choice != 1; }; // (Scanlines not offered)
        menu.Configure(with);
        g_values[2] = 0;
        CHECK(menu.Step(2, 1) == "Shader: CRT");
    }
    return 0;
}
