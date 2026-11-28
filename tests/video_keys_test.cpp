// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
// A game that changes a Display switch in its menu while the launcher runs (launcher/src/video_keys.h).
#include "../launcher/src/video_keys.h"

#include <cstdio>
#include <fstream>

static int failures = 0;
#define CHECK(x)                                                     \
    do {                                                             \
        if (!(x)) {                                                  \
            std::printf("%s:%d: failed: %s\n", __FILE__, __LINE__, #x); \
            failures++;                                              \
        }                                                            \
    } while (0)

int main() {
    const std::string path = "recomp_common_video_keys_test.ini";
    {
        std::ofstream file(path);
        file << "[video]\nwidescreen=0\nfullscreen=1\n\n[video.a]\nown=0\n\n[launcher]\nlanguage=pt\n";
    }
    // the launcher's copy, loaded once
    recomp::Ini prefs(path);
    prefs.load();
    const std::vector<launcher::Toggle> display = {{"fullscreen", "F", "", false, nullptr, true},
                                                   {"widescreen", "W", "", false, nullptr, true},
                                                   {"own", "O", "", false, nullptr, false}};
    // (the game's menu: its own copy of the file, as the game's KeepFirstPage writes it)
    recomp::Ini game(path);
    game.load();
    game.set("video", "widescreen", "1");
    game.set("video.a", "own", "1");
    game.save();
    // the launcher meanwhile changes something else, in memory only
    prefs.set("launcher", "language", "es");
    CHECK(prefs.get("video", "widescreen") == "0");
    CHECK(launcher::reread_switches(prefs, display, "video.a"));
    CHECK(prefs.get("video", "widescreen") == "1" && prefs.get("video", "fullscreen") == "1" && prefs.get("video.a", "own") == "1");
    CHECK(prefs.get("launcher", "language") == "es");  // (not thrown away)
    CHECK(!launcher::reread_switches(prefs, display, "video.a"));
    // the launcher's next save keeps the game's value
    prefs.save();
    recomp::Ini again(path);
    again.load();
    CHECK(again.get("video", "widescreen") == "1" && again.get("launcher", "language") == "es");
    std::remove(path.c_str());
    if (failures == 0) {
        std::printf("video_keys_test: all as it should be\n");
    }
    return failures ? 1 : 0;
}
