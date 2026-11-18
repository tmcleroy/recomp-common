// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
// The standard layout (menu_layout.h): the first page's groups, the pages put where they belong, the speeds.
#include "menu_layout.h"

#include <cstdio>
#include <cstdlib>

using namespace recomp;

#define CHECK(what)                                                                     \
    do {                                                                                \
        if (!(what)) {                                                                  \
            fprintf(stderr, "%s:%d: not so: %s\n", __FILE__, __LINE__, #what);         \
            exit(1);                                                                    \
        }                                                                               \
    } while (0)

namespace {
    const MenuPage *Find(const MenuConfig &c, const char *name) {
        for (const MenuPage &page : c.pages) {
            if (page.name == name) {
                return &page;
            }
        }
        return nullptr;
    }
} // namespace

int main() {
    MenuConfig c;
    c.features = {{layout::kCheats, "Infinite time", ""}, {layout::kTraining, "Hit boxes", ""}, {layout::kDummy, "Guards", ""},
                  {layout::kDisplay, "Shader", "", {"Off", "CRT"}}};
    // (a port's own page of the same name keeps its words, and is moved where it belongs)
    c.pages = {{layout::kCheats, "", "Mine."}, {layout::kTraining, "", "My training."}};
    layout::Rows rows;
    rows.heads = {layout::kStage};
    rows.actions = {{MenuRow::Action, 1, layout::kInsertCoin, "A coin."}};
    layout::Standard(c, rows);
    CHECK(c.groups.size() == 5);
    CHECK(c.groups[0].size() == 4 && c.groups[0][1].title == layout::kDisplay && c.groups[0][3].title == layout::kStage);
    CHECK(c.groups[1][0].title == layout::kModes);
    CHECK(c.groups[2][0].kind == MenuRow::Save);
    CHECK(c.groups[3][0].kind == MenuRow::Pages && c.groups[3][1].title == layout::kInsertCoin);
    CHECK(c.groups[4][0].kind == MenuRow::ResetOptions && c.groups[4][1].kind == MenuRow::Reset && c.groups[4][2].kind == MenuRow::Quit &&
          c.groups[4][3].kind == MenuRow::Resume);
    CHECK(Find(c, layout::kCheats)->parent == layout::kGame && Find(c, layout::kCheats)->description == "Mine.");
    CHECK(Find(c, layout::kTraining)->parent == layout::kModes && Find(c, layout::kTraining)->description == "My training.");
    CHECK(Find(c, layout::kDummy)->parent == layout::kTraining);
    CHECK(Find(c, layout::kGame)->rows.size() == 2);
    // (no states: three groups of rows and the head)
    layout::Rows none;
    none.states = false;
    layout::Standard(c, none);
    CHECK(c.groups.size() == 4);
    // The speeds: full in the middle, the menu starting there; none faster when the port can't
    layout::Speeds(c);
    CHECK(c.speeds.size() == 13 && c.speeds[static_cast<size_t>(c.normalSpeed)] == "Full");
    layout::Speeds(c, false);
    CHECK(c.speeds.size() == 7 && c.speeds.back() == "Full");
    CHECK(layout::SpeedOf(0).ticks == 8 && layout::SpeedOf(99).frames == 8);
    // And a menu made from it draws
    c.muted = [] { return false; };
    c.setMuted = [](bool) {};
    Menu menu;
    menu.Configure(c);
    menu.Open();
    std::vector<uint32_t> picture(640 * 480);
    menu.Draw(picture.data(), 640, 480);
    puts("layout_test: all as it should be");
    return 0;
}
