// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
// Which gamepad is which player's (launcher/src/bindings.h: [controllers], AssignPads). Needs SDL 3's headers and
// library, no gamepad.
#include "../launcher/src/bindings.h"

#include <cstdio>
#include <unistd.h>

static int failures = 0;
#define CHECK(x)                                                                                                       \
    do {                                                                                                               \
        if (!(x)) {                                                                                                    \
            std::fprintf(stderr, "pad_players_test: %s:%d: %s\n", __FILE__, __LINE__, #x);                           \
            failures++;                                                                                                \
        }                                                                                                              \
    } while (0)

using namespace recomp;

static bool Is(const int out[kBindMaxPlayers], int a, int b, int c, int d) {
    return out[0] == a && out[1] == b && out[2] == c && out[3] == d;
}

int main() {
    int out[kBindMaxPlayers];
    PadChoices none;
    // (nothing chosen: in the order connected, as it always was; those beyond the players play nothing)
    AssignPads(none, {"deck", "pad", "pad-2"}, 2, out);
    CHECK(Is(out, 0, 1, -1, -1));
    AssignPads(none, {}, 2, out);
    CHECK(Is(out, -1, -1, -1, -1));

    // (the Steam Deck's own controls not used: the two gamepads plugged into it are players 1 and 2)
    PadChoices deckOff;
    deckOff.player["deck"] = kPadNobody;
    AssignPads(deckOff, {"deck", "pad", "pad-2"}, 2, out);
    CHECK(Is(out, 1, 2, -1, -1));

    // (chosen ones first; the automatic one gets the player left; swapped from the order connected)
    PadChoices swapped;
    swapped.player["pad-2"] = 0;
    swapped.player["pad"] = 1;
    AssignPads(swapped, {"deck", "pad", "pad-2"}, 2, out);
    CHECK(Is(out, 2, 1, -1, -1));
    AssignPads(swapped, {"deck", "pad", "pad-2"}, 4, out);
    CHECK(Is(out, 2, 1, 0, -1));

    // (a chosen gamepad not connected: its player gets an automatic one)
    PadChoices gone;
    gone.player["away"] = 0;
    AssignPads(gone, {"deck", "pad"}, 2, out);
    CHECK(Is(out, 0, 1, -1, -1));

    // (player 3 chosen in a two-player game: as automatic; two chosen for one player: the first has it)
    PadChoices odd;
    odd.player["pad"] = 2;
    odd.player["deck"] = 1;
    odd.player["pad-2"] = 1;
    AssignPads(odd, {"deck", "pad", "pad-2"}, 2, out);
    CHECK(Is(out, 1, 0, -1, -1));

    // (an empty seat: takes its automatic place and plays nothing, the players after it keep theirs)
    AssignPads(none, {"", "pad"}, 2, out);
    CHECK(Is(out, 0, 1, -1, -1));
    AssignPads(deckOff, {"deck", "", "pad-2"}, 2, out);
    CHECK(Is(out, 1, 2, -1, -1));

    // (as text, and from a settings.ini)
    CHECK(PadChoiceText(kPadNobody) == "none" && PadChoiceText(kPadAuto) == "auto" && PadChoiceText(1) == "2");
    CHECK(PadChoiceFromText("none") == kPadNobody && PadChoiceFromText("auto") == kPadAuto && PadChoiceFromText("4") == 3);
    CHECK(PadChoiceFromText("5") == kPadAuto && PadChoiceFromText("") == kPadAuto);
    char path[] = "/tmp/pad_players_testXXXXXX";
    const int fd = mkstemp(path);
    CHECK(fd >= 0);
    const char text[] = "[gamepad1]\nlp = x\n[controllers]\n030000005e04-2 = 1\ndeck = none\n other = auto \n[after]\nx = 2\n";
    CHECK(write(fd, text, sizeof text - 1) == static_cast<ssize_t>(sizeof text - 1));
    close(fd);
    PadChoices read = LoadPadChoices(path);
    unlink(path);
    CHECK(read.player.size() == 3 && read.of("030000005e04-2") == 0 && read.of("deck") == kPadNobody && read.of("other") == kPadAuto);
    CHECK(read.of("x") == kPadAuto);

    if (failures == 0) {
        std::printf("pad_players_test: ok\n");
    }
    return failures == 0 ? 0 : 1;
}
