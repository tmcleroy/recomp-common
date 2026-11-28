// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
//
// A host with no game: sets the menu up as a port would, presses the buttons named on the command line and
// writes the picture as a PPM file. Shows everything a real host does except showing the picture in a window.
//
//   shot OUT.ppm [KEYS] [WIDTHxHEIGHT] [tabs|crt]
//     KEYS: u d l r (the directions), a (confirm), b (back), q e (the tab before, after), x (default), h (hotkey)
//     tabs: the tabbed menu (layout::Tabbed) in its window look; crt: in its CRT look (compact)
//   shot menu.ppm dddda 896x672
//   shot tabs.ppm e 1152x672 tabs
#include "i18n.h"
#include "menu.h"
#include "menu_layout.h"

#include <cstdio>
#include <cstdlib>
#include <map>
#include <vector>

using namespace recomp;

namespace {
    // What a port's own rows are called here
    enum { kInsertCoin = 1 };

    std::map<size_t, int> g_values = {{0, 1}}; // the features' values: a port keeps these in its settings

    MenuConfig Config() {
        MenuConfig config;
        config.groups = {
            {{MenuRow::Save}, {MenuRow::Load}, {MenuRow::Slot}},
            {{MenuRow::Pages}, {MenuRow::Speed}, {MenuRow::Action, kInsertCoin, "Insert coin", "Puts a coin in the first slot."}},
            {{MenuRow::ResetOptions}, {MenuRow::Reset}, {MenuRow::Quit}, {MenuRow::Resume}},
        };
        config.features = {
            {"Cheats", "Endless time", "The round's clock stands still."},
            {"Cheats", "Health", "Whose health is kept full.", {"Off", "Player 1", "Player 2", "Both"}},
            {"Picture", "Widescreen", "Shows more of the stage at each side (16:9)."},
        };
        config.commands = {
            {"", "Start over", "Back to where the round began."},
            {"Training", "Training mode", "Straight to a practice round."},
        };
        config.value = [](size_t feature) { return g_values[feature]; };
        config.setValue = [](size_t feature, int value) { g_values[feature] = value; };
        config.speeds = {"Full", "1/2", "1/4"};
        config.speedDescription = "Left/Right: slow motion.";
        config.confirmName = "a button";
        config.closeName = "Esc";
        return config;
    }
} // namespace

    // The tabbed menu as a port has it: the standard layout's pages, as tabs
    MenuConfig TabbedConfig() {
        MenuConfig config;
        using namespace recomp::layout;
        config.features = {
            {kGame, "Free play", "Start works without a coin."},
            {kGame, "Straight to the title", "Skips the board's start-up checks."},
            {kGame, "Run-ahead", "Takes the board's own lag away by running frames ahead.", {"Off", "1 frame", "2 frames"}},
            {kDisplay, "Widescreen", "The stage drawn wider than the board's 384 pixels, to fill a 16:9 screen."},
            {kDisplay, "Picture shape", "As an arcade monitor showed it (4:3), or each pixel square.", {"4:3", "Square pixels"}},
            {kDisplay, "Shader", "A filter over the picture.", {"Off", "Scanlines", "CRT aperture grille", "CRT slot mask"}},
            {kDisplay, "VSync", "Waits for the screen before showing each frame."},
            {"Arcade monitor", "Monitor", "The arcade monitor's mode.", {"Off", "15 kHz", "31 kHz"}},
            {kSound, "Custom soundtrack", "Music files played in place of the board's songs.", {"Off", "Arranged"}},
            {kTraining, "Gauge refills", "The super gauge fills again after each combo."},
            {kTraining, "Hit boxes", "Each fighter's hurt, attack, push and throw boxes."},
            {kDummy, "Guards", "What player 2 guards.", {"Never", "All", "After the first hit", "Random"}},
            {kCheats, "Infinite time", "The round's clock never runs out."},
            {kCheats, "No damage", "Hits land but take no health.", {"Off", "Player 1", "Player 2", "Both"}},
            {kStage, "Stage", "Where every fight is, whatever is played.", {"Game's own", "Random", "Feast of the Damned"}},
            {kVideo, "Replay length", "How much play is kept for Save replay.", {"Off", "10 seconds", "30 seconds", "60 seconds"}},
        };
        config.commands = {
            {kTraining, "Start training", "A fight with no clock that nobody loses."},
            {"Survival", "Start survival", "One life against every fighter in turn."},
            {kVideo, "Record video", "Records what is played and heard as an MP4."},
            {kSceneExport, "Save scene", "The picture taken apart, layer by layer."},
        };
        config.pages = {{"Arcade monitor", kDisplay, "A CRT at the board's own resolution."},
                        {"Survival", kModes, "One life against every fighter in turn.", false},
                        {kVideo, kGame, ""}, {kSceneExport, kGame, ""}};
        Rows rows;
        rows.actions = {{MenuRow::Action, ::kInsertCoin, "Insert coin", "Puts a coin in the first slot."}};
        Tabbed(config, rows);
        config.value = [](size_t feature) { return g_values[feature]; };
        config.setValue = [](size_t feature, int value) { g_values[feature] = value; };
        config.defaultValue = [](size_t feature) { return feature == 3 || feature == 6 ? 1 : 0; };
        Speeds(config);
        config.speedDescription = "Left/Right: slow motion, or faster than the board.";
        static bool muted = false;
        config.muted = [] { return muted; };
        config.setMuted = [](bool to) { muted = to; };
        config.confirmName = "A";
        config.closeName = "Esc";
        // (hotkeys: Run-ahead's is F3, Shader's the right stick)
        config.hotkey = [](size_t feature) { return feature == 2 ? std::string("F3") : feature == 5 ? std::string("Right stick") : std::string(); };
        config.hotkeyName = "H or Y";
        // (an icon: a port's comes from its PNG, icon.h's MenuIconOf; here two halves, green over blue, round-cornered)
        config.icon.width = config.icon.height = 64;
        for (int y = 0; y < 64; y++) {
            for (int x = 0; x < 64; x++) {
                const int cx = x < 14 ? 14 - x : x > 49 ? x - 49 : 0, cy = y < 14 ? 14 - y : y > 49 ? y - 49 : 0;
                const bool inside = cx * cx + cy * cy <= 14 * 14;
                config.icon.pixels.push_back(!inside ? 0 : y < 31 ? 0xFF6EB24Cu : y < 34 ? 0xFF5AB4D9u : 0xFFC9733Du);
            }
        }
        return config;
    }

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: shot OUT.ppm [KEYS] [WIDTHxHEIGHT]\n");
        return 2;
    }
    const char *keys = argc > 2 ? argv[2] : "";
    int width = 896, height = 672;
    if (argc > 3 && *argv[3] && sscanf(argv[3], "%dx%d", &width, &height) != 2) {
        fprintf(stderr, "shot: the size is WIDTHxHEIGHT\n");
        return 2;
    }

    // (in the language RECOMP_LANGUAGE names, as a game the launcher starts: i18n.h)
    recomp::i18n::SetLanguage(recomp::i18n::Resolve("", recomp::i18n::FromEnvironment()));
    const std::string look = argc > 4 ? argv[4] : "";
    Menu menu;
    if (look == "tabs" || look == "crt") {
        g_values = {{0, 1}, {1, 1}, {3, 1}, {5, 2}, {6, 1}, {10, 1}};
        menu.Configure(TabbedConfig());
        menu.SetCompact(look == "crt");
    } else {
        menu.Configure(Config());
    }
    menu.Open();
    for (const char *key = keys; *key; key++) {
        const unsigned button = *key == 'u' ? kMenuUp : *key == 'd' ? kMenuDown : *key == 'l' ? kMenuLeft : *key == 'r' ? kMenuRight
                                : *key == 'a' ? kMenuConfirm : *key == 'b' ? kMenuBack : *key == 'q' ? kMenuTabPrev
                                : *key == 'e' ? kMenuTabNext : *key == 'x' ? kMenuDefault : *key == 'h' ? kMenuHotkey : 0;
        const Menu::Chosen chosen = menu.Buttons(button);
        // A port carries out what was chosen and closes the menu if that went well
        switch (chosen.kind) {
        case Menu::Chosen::Save: printf("save a state in slot %d\n", menu.Slot()); break;
        case Menu::Chosen::Load: menu.Note("Nothing is kept in slot " + std::to_string(menu.Slot()) + "."); break;
        case Menu::Chosen::Action: printf("the port's own row %d\n", chosen.id); break;
        case Menu::Chosen::Command: printf("the game's command %d\n", chosen.id); break;
        case Menu::Chosen::Quit: printf("quit\n"); return 0;
        case Menu::Chosen::Reset: printf("reset the game\n"); break;
        case Menu::Chosen::ResetOptions: printf("every option back as it first was\n"); break;
        case Menu::Chosen::None: break;
        }
    }

    // The game's picture as it stood (here: a plain gradient), red in the low byte, with the menu over it
    std::vector<uint32_t> pixels(static_cast<size_t>(width) * height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const uint32_t r = 40 + 100 * x / width, g = 60 + 80 * y / height, b = 120;
            pixels[static_cast<size_t>(y) * width + x] = 0xFF000000u | (b << 16) | (g << 8) | r;
        }
    }
    if (menu.IsOpen()) {
        menu.Draw(pixels.data(), width, height);
    }
    FILE *out = fopen(argv[1], "wb");
    if (!out) {
        perror(argv[1]);
        return 1;
    }
    fprintf(out, "P6\n%d %d\n255\n", width, height);
    for (uint32_t pixel : pixels) {
        const unsigned char rgb[3] = {static_cast<unsigned char>(pixel), static_cast<unsigned char>(pixel >> 8), static_cast<unsigned char>(pixel >> 16)};
        fwrite(rgb, 1, 3, out);
    }
    fclose(out);
    return 0;
}
