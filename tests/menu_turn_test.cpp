// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
// The menu turned with the picture (menu_turn.h): the turn for each choice, the pictures turned, and places mapped
// back through the turn as the pictures are turned.
#include "menu.h"
#include "menu_turn.h"

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
    CHECK(turn::Choices().size() == 5 && turn::Choices()[turn::kAuto] == "Auto" && turn::Choices()[turn::kUpright] == "Upright");
    CHECK(turn::Normal(-90) == 270 && turn::Normal(450) == 90 && turn::Normal(0) == 0);
    // (Auto: as the picture stands. A game stood up by a quarter turn: shown stood up, upright; shown as the board sends
    // it, the menu turned back by that quarter; the half turn of the other way, the other quarter)
    CHECK(turn::MenuTurn(turn::kAuto, 90, 90) == 0);
    CHECK(turn::MenuTurn(turn::kAuto, 0, 90) == 270);
    CHECK(turn::MenuTurn(turn::kAuto, 180, 90) == 90);
    CHECK(turn::MenuTurn(turn::kAuto, 0, 270) == 90);
    CHECK(turn::MenuTurn(turn::kAuto, 0, 0) == 0);
    CHECK(turn::MenuTurn(turn::kUpright, 0, 90) == 0);
    CHECK(turn::MenuTurn(turn::kTopRight, 0, 0) == 90 && turn::MenuTurn(turn::kUpsideDown, 90, 90) == 180 &&
          turn::MenuTurn(turn::kTopLeft, 0, 90) == 270);
    int tw = 0, th = 0;
    turn::TurnedSize(90, 4, 3, tw, th);
    CHECK(tw == 3 && th == 4);
    turn::TurnedSize(180, 4, 3, tw, th);
    CHECK(tw == 4 && th == 3);
    // (each pixel goes where Unturn says it came from: a 5 x 3 picture of distinct pixels turned every way)
    const int w = 5, h = 3;
    std::vector<uint32_t> in(w * h);
    for (int i = 0; i < w * h; i++) {
        in[i] = static_cast<uint32_t>(i + 1);
    }
    for (int degrees = 0; degrees < 360; degrees += 90) {
        turn::TurnedSize(degrees, w, h, tw, th);
        std::vector<uint32_t> out(tw * th, 0);
        turn::TurnPixels(in.data(), w, h, degrees, out.data());
        for (int y = 0; y < th; y++) {
            for (int x = 0; x < tw; x++) {
                float fx = 0, fy = 0;
                turn::Unturn(degrees, (x + 0.5f) / tw, (y + 0.5f) / th, fx, fy);
                const int sx = static_cast<int>(std::floor(fx * w)), sy = static_cast<int>(std::floor(fy * h));
                CHECK(sx >= 0 && sx < w && sy >= 0 && sy < h);
                CHECK(out[y * tw + x] == in[sy * w + sx]);
            }
        }
        // (and a quarter turn and its other three make a whole one)
        std::vector<uint32_t> back(w * h, 0);
        int bw = 0, bh = 0;
        turn::TurnedSize(360 - degrees, tw, th, bw, bh);
        CHECK(bw == w && bh == h);
        turn::TurnPixels(out.data(), tw, th, 360 - degrees, back.data());
        CHECK(back == in);
    }
    // (the menu drawn on its own picture, the turned screen's shape, and turned onto the screen's: its rows where the
    // mouse finds them through the turn)
    MenuConfig config;
    config.groups = {{{MenuRow::Save}, {MenuRow::Load}}};
    for (int degrees = 0; degrees < 360; degrees += 90) {
        Menu menu;
        menu.Configure(config);
        menu.Open();
        const int sw = 480, sh = 360; // (the screen's picture)
        int mw = 0, mh = 0;
        turn::TurnedSize(degrees, sw, sh, mw, mh);
        std::vector<uint32_t> own(mw * mh, 0xFF000000u), screen(sw * sh);
        menu.Draw(own.data(), mw, mh);
        turn::TurnPixels(own.data(), mw, mh, degrees, screen.data());
        bool found = false;
        for (int i = 0; i < 400 && !found; i++) {
            // (along the menu's own middle, from its top down, as places on the screen)
            const float fy = i / 400.0f;
            float u = 0.5f, v = fy;
            switch (degrees) {
            case 90: u = 1 - fy; v = 0.5f; break;
            case 180: u = 0.5f; v = 1 - fy; break;
            case 270: u = fy; v = 0.5f; break;
            default: break;
            }
            float x = 0, y = 0;
            turn::Unturn(degrees, u, v, x, y);
            Menu probe;
            probe.Configure(config);
            probe.Open();
            probe.Draw(own.data(), mw, mh);
            found = probe.Mouse(MenuMouse::Click, x * mw, y * mh).kind == Menu::Chosen::Load;
        }
        CHECK(found);
    }
    puts("menu_turn_test: all as it should be");
    return 0;
}
