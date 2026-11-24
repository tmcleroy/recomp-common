// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
// The player's own shader presets (src/own_shaders.h): the shared folder, finding the presets in it and in a game's
// own, the list shown, and the choice kept by name.
#include "own_shaders.h"

#include <cstdio>
#include <fstream>

namespace fs = std::filesystem;
using namespace recomp::shaders;

static int failures = 0;
#define CHECK(x)                                                                 \
    do {                                                                         \
        if (!(x)) {                                                              \
            std::fprintf(stderr, "%s:%d: FAILED: %s\n", __FILE__, __LINE__, #x); \
            failures++;                                                          \
        }                                                                        \
    } while (0)

static void touch(const fs::path &file) {
    fs::create_directories(file.parent_path());
    std::ofstream(file) << "shaders = 1\n";
}

static void unset(const char *name) {
#ifdef _WIN32
    _putenv_s(name, "");
#else
    unsetenv(name);
#endif
}

int main() {
    recomp::i18n::SetLanguage("en");
    unset(kEnvironment);
    const fs::path root = fs::temp_directory_path() / "recomp-own-shaders-test";
    fs::remove_all(root);
    const fs::path game = root / "Some Game", launcher = root / "Launcher";

    // Where the shared folder is: the launcher's, beside the game's folder, unless the launcher said
    CHECK(SharedFolder(game, "Launcher") == root / "Launcher" / "shaders");
    CHECK(SharedFolder(game.string() + "/", "Launcher") == root / "Launcher" / "shaders");
    CHECK(SharedFolder(fs::path(), "Launcher").empty());
    CHECK(LauncherFolder(launcher) == launcher / "shaders");
    CHECK(Prepare(LauncherFolder(launcher)) && fs::is_directory(launcher / "shaders"));
    CHECK(SharedFolder(root / "Elsewhere" / "Game", "Other") == launcher / "shaders"); // (RECOMP_SHADERS, as Prepare set it)
    unset(kEnvironment);

    // None yet
    CHECK(Find(GameFolder(game), LauncherFolder(launcher)).empty());
    std::string why;
    CHECK(Pick({}, "", why) == nullptr && !why.empty());

    // A pack dropped in whole (its passes beside it, which aren't presets), another preset, a file that isn't one
    touch(launcher / "shaders" / "crt-pack" / "newpixie-crt.slangp");
    touch(launcher / "shaders" / "crt-pack" / "shaders" / "newpixie" / "blur.slang");
    touch(launcher / "shaders" / "Zebra.SLANGP");
    touch(launcher / "shaders" / "a-scanlines.slangp");
    touch(launcher / "shaders" / "notes.txt");
    std::vector<Preset> found = Find(GameFolder(game), LauncherFolder(launcher));
    CHECK(found.size() == 3);
    if (found.size() == 3) {
        CHECK(found[0].name == "a-scanlines.slangp" && found[1].name == "crt-pack/newpixie-crt.slangp" && found[2].name == "Zebra.SLANGP");
        CHECK(!found[0].game_own && found[1].file == launcher / "shaders" / "crt-pack" / "newpixie-crt.slangp");
        CHECK(Label(found[1]) == "crt-pack/newpixie-crt" && Label(found[2]) == "Zebra");
    }
    // (none chosen: the first)
    const Preset *picked = Pick(found, "", why);
    CHECK(picked && picked->name == "a-scanlines.slangp" && why.empty());

    // The game's own folder goes first, and its preset of a shared one's name is used in its place
    touch(game / "shaders" / "crt-pack" / "newpixie-crt.slangp");
    touch(game / "shaders" / "mine.slangp");
    found = Find(GameFolder(game), LauncherFolder(launcher));
    CHECK(found.size() == 4);
    if (found.size() == 4) {
        CHECK(found[0].name == "crt-pack/newpixie-crt.slangp" && found[0].game_own && found[0].file == game / "shaders" / "crt-pack" / "newpixie-crt.slangp");
        CHECK(found[1].name == "mine.slangp" && found[1].game_own);
        CHECK(found[2].name == "a-scanlines.slangp" && !found[2].game_own);
        CHECK(Label(found[1]) == "mine (this game's)");
    }
    picked = Pick(found, "", why);
    CHECK(picked && picked->game_own && picked->name == "crt-pack/newpixie-crt.slangp");

    // (a launcher of one game: its folder is the game's, so the shared folder is the game's own, its presets shared)
    const std::vector<Preset> one = Find(GameFolder(launcher), SharedFolder(launcher, "Launcher"));
    CHECK(one.size() == 3 && !one[0].game_own);

    // The choice, kept by name in settings.ini: found again however the list has changed around it
    recomp::Ini ini(root / "Some Game" / "settings.ini");
    ini.load();
    CHECK(Chosen(ini).empty());
    Choose(ini, "Zebra.SLANGP");
    CHECK(ini.save());
    recomp::Ini again(root / "Some Game" / "settings.ini");
    again.load();
    CHECK(Chosen(again) == "Zebra.SLANGP");
    touch(launcher / "shaders" / "b-before-zebra.slangp");
    found = Find(GameFolder(game), LauncherFolder(launcher));
    picked = Pick(found, Chosen(again), why);
    CHECK(picked && picked->name == "Zebra.SLANGP" && picked->file == launcher / "shaders" / "Zebra.SLANGP");

    // The list the launcher and the menu show
    Choices list = MakeChoices(found, Chosen(again));
    CHECK(list.labels.size() == found.size() + 1 && list.labels[0] == kFirstFound && list.names[0].empty());
    CHECK(list.NameAt(list.IndexOf("Zebra.SLANGP")) == "Zebra.SLANGP" && list.IndexOf("Zebra.SLANGP") == static_cast<int>(found.size()));
    CHECK(list.IndexOf("") == 0 && list.NameAt(0).empty() && list.NameAt(99).empty());

    // A chosen one taken away: not drawn (and why), and still in the list, at the end, until another is chosen
    fs::remove(launcher / "shaders" / "Zebra.SLANGP");
    found = Find(GameFolder(game), LauncherFolder(launcher));
    CHECK(Pick(found, "Zebra.SLANGP", why) == nullptr && why.find("Zebra.SLANGP") != std::string::npos);
    list = MakeChoices(found, "Zebra.SLANGP");
    CHECK(list.labels.size() == found.size() + 2 && list.labels.back() == "Zebra.SLANGP (not found)");
    CHECK(list.IndexOf("Zebra.SLANGP") == static_cast<int>(found.size()) + 1);

    // (in the player's language: the first choice as it is drawn, Tr; the others made so)
    recomp::i18n::SetLanguage("es");
    CHECK(std::string(recomp::Tr(MakeChoices({}, "").labels[0].c_str())) != kFirstFound);
    CHECK(MakeChoices({}, "x.slangp").labels[1] == "x.slangp (no encontrado)");
    recomp::i18n::SetLanguage("en");

    fs::remove_all(root);
    if (failures) {
        return 1;
    }
    std::puts("own_shaders_test: all as it should be");
    return 0;
}
