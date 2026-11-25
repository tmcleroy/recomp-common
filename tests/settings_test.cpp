// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
// The settings rules a launcher and its game share: presets, values, choices, exclusive features, and that
// a file keeps everything nobody asked to change.
#include "settings.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

using namespace recomp;

#define CHECK(what)                                                    \
    do {                                                               \
        if (!(what)) {                                                 \
            fprintf(stderr, "%s:%d: not so: %s\n", __FILE__, __LINE__, #what); \
            exit(1);                                                   \
        }                                                              \
    } while (0)

namespace {

    std::vector<FeatureInfo> Features() {
        return {
            {"free_play", "Game", "Free play", "Start works without a coin.", kFeatureEnhanced, {}},
            {"easy", "Difficulty", "Easy", "", kFeatureExclusive, {}},
            {"hard", "Difficulty", "Hard", "", kFeatureExclusive, {}},
            {"health", "Cheats", "Health", "Whose is kept full.", 0, {"Off", "Player 1", "Both"}},
            {"training", "Training", "Training", "This run only.", kFeatureSession, {}},
            {"wide", "Picture", "Wide", "", kFeatureEnhanced | kFeatureExclusive, {}},
        };
    }

    std::filesystem::path Write(const char *name, const std::string &text) {
        const std::filesystem::path path = std::filesystem::temp_directory_path() / name;
        std::ofstream(path, std::ios::binary) << text;
        return path;
    }

    std::string Read(const std::filesystem::path &path) {
        std::ifstream file(path, std::ios::binary);
        std::stringstream text;
        text << file.rdbuf();
        return text.str();
    }

    Ini Loaded(const std::string &text) {
        Ini ini(Write("recomp_common_settings_test.ini", text));
        ini.load();
        return ini;
    }

} // namespace

int main() {
    const std::vector<FeatureInfo> features = Features();

    // The list as text and back; session features are not in it; choices are a sixth field
    const std::string text = FeatureListText(features);
    CHECK(text.find("training") == std::string::npos);
    CHECK(text.find("health\tCheats\t0\tHealth\tWhose is kept full.\tOff|Player 1|Both\n") != std::string::npos);
    CHECK(text.find("free_play\tGame\t2\tFree play\tStart works without a coin.\n") != std::string::npos);
    const std::vector<FeatureInfo> parsed = ParseFeatureList(text);
    CHECK(parsed.size() == 5);
    CHECK(parsed[3].name == "health" && parsed[3].choices.size() == 3 && parsed[3].choices[2] == "Both");
    CHECK(parsed[0].choices.empty() && parsed[0].flags == kFeatureEnhanced);
    // (a list from a game that knows nothing of choices, and lines that are not features)
    CHECK(ParseFeatureList("a\tB\t4\tA\tAn a.\r\nnot a feature\n\n").size() == 1);

    // No file: pure, everything off
    {
        Ini ini("/nonexistent/recomp-common/settings.ini");
        ini.load();
        CHECK(Preset(ini) == "pure");
        for (int value : FeatureValues(ini, features)) {
            CHECK(value == 0);
        }
    }
    // Enhanced: the enhanced ones; a value in the file wins; a choice by number; a choice it has not is off
    {
        Ini ini = Loaded("[features]\npreset = enhanced\nhealth = 2\neasy=on\n");
        const std::vector<int> v = FeatureValues(ini, features);
        CHECK(v[0] == 1 && v[1] == 1 && v[2] == 0 && v[3] == 2 && v[4] == 0 && v[5] == 1);
        ini = Loaded("[features]\npreset=enhanced\nfree_play=off\nhealth=7\ntraining=1\nwide=no\n");
        const std::vector<int> w = FeatureValues(ini, features);
        CHECK(w[0] == 0 && w[3] == 0 && w[4] == 0 && w[5] == 0);
        // (on or off features are never more than 1; of two exclusive ones on, the first is)
        ini = Loaded("[features]\npreset=custom\nfree_play=5\neasy=1\nhard=1\n");
        const std::vector<int> x = FeatureValues(ini, features);
        CHECK(x[0] == 1 && x[1] == 1 && x[2] == 0);
    }
    // Changing one from a preset: the others are written as they were, a choice keeps its number, the
    // preset becomes custom, and the rest of the file is as it was
    {
        const std::filesystem::path path =
            Write("recomp_common_settings_test.ini", "; mine\n[video]\nfullscreen=1\n\n[features]\npreset=enhanced\nhealth = 2\n\n[keyboard1]\nup = W\n");
        Ini ini(path);
        ini.load();
        SetFeature(ini, features, 2, 1);
        CHECK(ini.save());
        const std::string after = Read(path);
        CHECK(after.find("; mine\n[video]\nfullscreen=1\n") == 0);
        CHECK(after.find("[keyboard1]\nup = W\n") != std::string::npos);
        CHECK(after.find("health = 2\n") != std::string::npos); // (not rewritten, not made 1)
        CHECK(after.find("training") == std::string::npos);
        Ini again(path);
        again.load();
        CHECK(Preset(again) == "custom");
        const std::vector<int> v = FeatureValues(again, features);
        CHECK(v[0] == 1 && v[1] == 0 && v[2] == 1 && v[3] == 2 && v[5] == 1);
        // An exclusive one on: its category's other goes off, another category's stays
        SetFeature(again, features, 1, 1);
        const std::vector<int> w = FeatureValues(again, features);
        CHECK(w[1] == 1 && w[2] == 0 && w[5] == 1);
        // A choice
        SetFeature(again, features, 3, 1);
        CHECK(FeatureValues(again, features)[3] == 1);
        CHECK(again.get("features", "health") == "1");
        // A session feature is never written
        SetFeature(again, features, 4, 1);
        CHECK(again.get("features", "training").empty());
        // Back to a preset: the features follow it again
        SetPreset(again, features, "pure");
        for (int value : FeatureValues(again, features)) {
            CHECK(value == 0);
        }
        SetPreset(again, features, "enhanced");
        CHECK(FeatureValues(again, features)[0] == 1 && FeatureValues(again, features)[3] == 0);
        CHECK(again.get("video", "fullscreen") == "1" && again.get("keyboard1", "up") == "W");
    }
    // A file with no section yet
    {
        Ini ini = Loaded("");
        SetFeature(ini, features, 0, 1);
        CHECK(Preset(ini) == "custom" && FeatureValues(ini, features)[0] == 1);
    }
    // The choices a kFeatureMenuChoices feature's menu offers: all until narrowed, off always, the rest as [menu] says
    {
        const FeatureInfo shader = {"shader", "Display", "Shader", "", kFeatureMenuChoices, {"Off", "CRT", "Scanlines", "xBRZ"}};
        Ini ini = Loaded("[features]\nshader=2\n");
        CHECK((MenuChoices(ini, shader) == std::vector<bool>{true, true, true, true}));
        SetMenuChoices(ini, shader, {false, true, false, true});
        CHECK(ini.get("menu", "shader") == "1,3" && ini.get("features", "shader") == "2");
        CHECK((MenuChoices(ini, shader) == std::vector<bool>{true, true, false, true}));
        SetMenuChoices(ini, shader, {true, false, false, false});
        CHECK(ini.get("menu", "shader") == "0" && (MenuChoices(ini, shader) == std::vector<bool>{true, false, false, false}));
        SetMenuChoices(ini, shader, {true, true, true, true});
        CHECK(ini.get("menu", "shader").empty() && (MenuChoices(ini, shader) == std::vector<bool>{true, true, true, true}));
        // (numbers it has not, and a feature that is not one: every choice)
        CHECK((MenuChoices(Loaded("[menu]\nshader=9,x,2\n"), shader) == std::vector<bool>{true, false, true, false}));
        CHECK((MenuChoices(Loaded("[menu]\nhealth=1\n"), features[3]) == std::vector<bool>{true, true, true}));
    }
    // A choice kept by name (FeatureInfo::ids): a list put in another order (a choice moved from the end to the middle)
    // keeps every saved choice's meaning, and an older build that knows only the numbers reads what a newer wrote
    {
        // (as an older build had it: by number, 13 the one added last; and now, moved up, with names)
        const std::vector<std::string> oldOrder = {"Off", "CRT", "CRT Lottes", "CRT Royale", "CRT Guest", "CRT Light",
                                                   "Scanlines: light", "Scanlines: strong", "Scanlines: OSSC", "Scanlines: dark",
                                                   "xBRZ", "ScaleFX", "Your own", "CRT Newpixie"};
        const std::vector<std::string> oldIds = {"off", "crt", "crt-lottes", "crt-royale", "crt-guest", "crt-light",
                                                 "scanlines-light", "scanlines-strong", "scanlines-ossc", "scanlines-dark",
                                                 "xbrz", "scalefx", "your-own", "crt-newpixie"};
        const std::vector<FeatureInfo> before = {{"shader", "Display", "Shader", "", kFeatureMenuChoices, oldOrder}};
        FeatureInfo shader = {"shader", "Display", "Shader", "", kFeatureMenuChoices, {}};
        shader.ids = {"off", "crt", "crt-lottes", "crt-royale", "crt-guest", "crt-light", "crt-newpixie", "scanlines-light",
                      "scanlines-strong", "scanlines-ossc", "scanlines-dark", "xbrz", "scalefx", "your-own", "made-later"};
        for (const std::string &id : shader.ids) {
            const auto at = std::find(oldIds.begin(), oldIds.end(), id);
            shader.choices.push_back(at == oldIds.end() ? "Made later" : oldOrder[static_cast<size_t>(at - oldIds.begin())]);
        }
        shader.numbered = oldIds;
        const std::vector<FeatureInfo> now = {shader};
        auto choiceName = [](const std::vector<FeatureInfo> &list, const Ini &ini) {
            return list[0].choices[static_cast<size_t>(FeatureValues(ini, list)[0])];
        };
        // Every old number reads as the same choice, and is made over once into both keys
        for (int number = 0; number < 14; number++) {
            Ini ini = Loaded("[features]\npreset=custom\nshader=" + std::to_string(number) + "\n");
            CHECK(choiceName(before, ini) == oldOrder[static_cast<size_t>(number)]);
            CHECK(choiceName(now, ini) == oldOrder[static_cast<size_t>(number)]);
            CHECK(MigrateFeatures(ini, now));
            CHECK(ini.get("features", "shader_id") == oldIds[static_cast<size_t>(number)]);
            CHECK(ini.get("features", "shader") == std::to_string(number));
            CHECK(choiceName(now, ini) == oldOrder[static_cast<size_t>(number)] && choiceName(before, ini) == oldOrder[static_cast<size_t>(number)]);
            CHECK(!MigrateFeatures(ini, now)); // (once)
        }
        // (off as words, and a number past the old list: off)
        CHECK(FeatureValues(Loaded("[features]\nshader=off\n"), now)[0] == 0 && FeatureValues(Loaded("[features]\nshader=40\n"), now)[0] == 0);
        // Written by a newer build, every choice: name_id, and the old number beside it, which the older build reads as
        // the same choice; one with no old number, none (the older build: as the preset has it, off)
        for (int choice = 0; choice < static_cast<int>(shader.choices.size()); choice++) {
            Ini ini = Loaded("[features]\npreset=pure\n");
            SetFeature(ini, now, 0, choice);
            CHECK(ini.get("features", "shader_id") == shader.ids[static_cast<size_t>(choice)]);
            CHECK(FeatureValues(ini, now)[0] == choice);
            if (shader.ids[static_cast<size_t>(choice)] == "made-later") {
                CHECK(ini.get("features", "shader").empty() && FeatureValues(ini, before)[0] == 0);
            } else {
                CHECK(choiceName(before, ini) == shader.choices[static_cast<size_t>(choice)]);
            }
        }
        // name_id wins over a number that disagrees (an older build's change after a newer's), and is made to agree
        {
            Ini ini = Loaded("[features]\npreset=custom\nshader_id=crt-newpixie\nshader=6\n");
            CHECK(choiceName(now, ini) == "CRT Newpixie");
            CHECK(MigrateFeatures(ini, now) && ini.get("features", "shader") == "13");
        }
        // An id this build does not know (a newer build's): its number; left as it is
        {
            Ini ini = Loaded("[features]\npreset=custom\nshader_id=crt-future\nshader=2\n");
            CHECK(choiceName(now, ini) == "CRT Lottes");
            CHECK(!MigrateFeatures(ini, now) && ini.get("features", "shader_id") == "crt-future");
        }
        // A preset takes both out
        {
            Ini ini = Loaded("[features]\npreset=custom\nshader_id=xbrz\nshader=10\n");
            SetPreset(ini, now, "enhanced");
            CHECK(ini.get("features", "shader_id").empty() && ini.get("features", "shader").empty());
        }
        // The favourites: old numbers read as the same choices and made over; written by name and by old number, and
        // the older build offers the same ones
        {
            Ini ini = Loaded("[menu]\nshader=1,6,12,13\n");
            const std::vector<bool> offered = MenuChoices(ini, shader);
            CHECK(offered[0] && offered[1] && offered[6] && offered[7] && offered[13] && !offered[2] && !offered[12] && !offered[14]);
            CHECK(MigrateFeatures(ini, now));
            CHECK(ini.get("menu", "shader_id") == "crt,crt-newpixie,scanlines-light,your-own");
            CHECK(ini.get("menu", "shader") == "1,6,12,13");
            CHECK(MenuChoices(ini, shader) == offered && !MigrateFeatures(ini, now));
            std::vector<bool> mine(shader.choices.size(), false);
            mine[2] = mine[6] = mine[14] = true;
            SetMenuChoices(ini, shader, mine);
            CHECK(ini.get("menu", "shader_id") == "crt-lottes,crt-newpixie,made-later" && ini.get("menu", "shader") == "2,13");
            CHECK((MenuChoices(ini, before[0]) == std::vector<bool>{true, false, true, false, false, false, false, false, false,
                                                                    false, false, false, false, true}));
            CHECK(MenuChoices(ini, shader) == (std::vector<bool>{true, false, true, false, false, false, true, false, false,
                                                                 false, false, false, false, false, true}));
            // (only one with no old number: the older build offers off alone; every one: neither key)
            std::vector<bool> later(shader.choices.size(), false);
            later[14] = true;
            SetMenuChoices(ini, shader, later);
            CHECK(ini.get("menu", "shader_id") == "made-later" && ini.get("menu", "shader") == "0");
            SetMenuChoices(ini, shader, std::vector<bool>(shader.choices.size(), true));
            CHECK(ini.get("menu", "shader_id").empty() && ini.get("menu", "shader").empty());
        }
        // The list as text carries the ids and the old order; a reader that knows nothing of them sees the choices
        {
            const std::vector<FeatureInfo> back = ParseFeatureList(FeatureListText(now));
            CHECK(back.size() == 1 && back[0].choices == shader.choices && back[0].ids == shader.ids && back[0].numbered == shader.numbered);
            FeatureInfo gap = shader;
            gap.numbered[3] = "";
            CHECK(ParseFeatureList(FeatureListText({gap}))[0].numbered == gap.numbered);
        }
    }
    // Chosen: a value of the file's own, not the preset's (a collection's launcher gives a game new to it the shared values)
    {
        Ini ini = Loaded("[features]\npreset=enhanced\n");
        CHECK(!FeatureChosen(ini, features[0]) && FeatureValues(ini, features)[0] == 1);
        SetFeature(ini, features, 3, 2);
        CHECK(FeatureChosen(ini, features[3]) && FeatureChosen(ini, features[0])); // (custom: the others written down)
        CHECK(!FeatureChosen(ini, features[4]));                                   // (a session feature never is)
        SetPreset(ini, features, "pure");
        CHECK(!FeatureChosen(ini, features[3]));
    }
    std::error_code error;
    std::filesystem::remove(std::filesystem::temp_directory_path() / "recomp_common_settings_test.ini", error);
    printf("settings: ok\n");
    return 0;
}
