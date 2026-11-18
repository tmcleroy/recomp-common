// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
// The arcade monitor's choice of mode (arcade_sdl.h), on made-up lists of modes: no screen needed.
#include "../src/arcade_sdl.h"

#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace recomp::sdl;

#define CHECK(x)                                                                                                       \
    do {                                                                                                               \
        if (!(x)) {                                                                                                    \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x);                                                    \
            exit(1);                                                                                                   \
        }                                                                                                              \
    } while (0)

namespace {
    SDL_DisplayMode Mode(int w, int h, float rate, float density = 1.0f) {
        SDL_DisplayMode mode{};
        mode.w = w;
        mode.h = h;
        mode.refresh_rate = rate;
        mode.pixel_density = density;
        return mode;
    }
    int Pick(std::vector<SDL_DisplayMode> modes, ArcadeMode want, const ArcadeBoard &board) {
        std::vector<const SDL_DisplayMode *> list;
        for (const SDL_DisplayMode &mode : modes) {
            list.push_back(&mode);
        }
        return ArcadeModeIndex(list.data(), static_cast<int>(list.size()), want, board);
    }
} // namespace

int main() {
    const ArcadeBoard cps2{384, 224, 59.64}, naomi{640, 480, 59.94}, cave{320, 240, 57.55};
    // (the board's own: exactly its size, the refresh nearest; never another size, nor a rate 5 Hz off)
    CHECK(Pick({Mode(1920, 1080, 60), Mode(384, 224, 60), Mode(384, 224, 59.6f)}, ArcadeMode::Board, cps2) == 2);
    CHECK(Pick({Mode(384, 240, 59.64f), Mode(400, 224, 59.64f), Mode(384, 224, 50)}, ArcadeMode::Board, cps2) == -1);
    CHECK(Pick({Mode(192, 112, 59.6f, 2.0f)}, ArcadeMode::Board, cps2) == 0); // (points times density)
    CHECK(Pick({Mode(384, 224, 59.6f)}, ArcadeMode::Off, cps2) == -1);
    // (super: 1920 columns or more of its lines, the widest at the nearest refresh)
    CHECK(Pick({Mode(1280, 224, 59.6f), Mode(1920, 224, 59.6f), Mode(2560, 224, 59.6f), Mode(3840, 224, 50)}, ArcadeMode::Super, cps2) == 2);
    CHECK(Pick({Mode(2560, 224, 61), Mode(1920, 224, 59.64f)}, ArcadeMode::Super, cps2) == 1);
    // (224 before 240, wherever 240 comes in the list; 240 where there is no 224)
    CHECK(Pick({Mode(2560, 240, 59.64f), Mode(2560, 224, 60)}, ArcadeMode::Super, cps2) == 1);
    CHECK(Pick({Mode(2560, 224, 60), Mode(2560, 240, 59.64f)}, ArcadeMode::Super, cps2) == 0);
    CHECK(Pick({Mode(640, 480, 60), Mode(2560, 240, 59.9f)}, ArcadeMode::Super, cps2) == 1);
    // (a board of 240 lines or more: its own lines alone)
    CHECK(Pick({Mode(2560, 240, 57.5f)}, ArcadeMode::Super, cave) == 0);
    CHECK(Pick({Mode(2560, 224, 57.5f)}, ArcadeMode::Super, cave) == -1);
    CHECK(Pick({Mode(320, 240, 57.55f), Mode(320, 240, 60)}, ArcadeMode::Board, cave) == 0);
    CHECK(Pick({Mode(2560, 240, 59.94f), Mode(2560, 480, 59.94f)}, ArcadeMode::Super, naomi) == 1);
    CHECK(Pick({Mode(640, 480, 59.94f)}, ArcadeMode::Board, naomi) == 0);
    // (the picture on 240 lines: its 224 one for one in the middle; on its own lines, or the desktop's, as it was)
    const SDL_FRect fill{0, 0, 2560, 240};
    const SDL_FRect mid = ArcadeLines(fill, 240, 224);
    CHECK(mid.y == 8 && mid.h == 224 && mid.w == 2560);
    const SDL_FRect same = ArcadeLines({0, 0, 2560, 224}, 224, 224);
    CHECK(same.y == 0 && same.h == 224);
    CHECK(ArcadeLines({0, 0, 1920, 1080}, 1080, 224).h == 1080);
    // (the choices' words)
    CHECK(ArcadeBoardName(cps2) == "384 x 224");
    CHECK(ArcadeDescription(cps2).find("59.64 Hz") != std::string::npos && ArcadeDescription(cps2).find("240 lines") != std::string::npos);
    CHECK(ArcadeDescription(cave).find("240 lines (") == std::string::npos);
    puts("arcade_sdl_test: all as it should be");
    return 0;
}
