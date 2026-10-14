# recomp-common

[![CI](https://github.com/tmcleroy/recomp-common/actions/workflows/ci.yml/badge.svg)](https://github.com/tmcleroy/recomp-common/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/tmcleroy/recomp-common)](https://github.com/tmcleroy/recomp-common/releases)
[![License: GPL-3.0](https://img.shields.io/badge/license-GPL--3.0-blue)](LICENSE)

Code shared between recomp ports (native ports of games made by static recompilation): an
**in-game overlay menu** and the small bitmap text it is drawn with, the **settings rules** a
launcher and its game must agree on, and the **launcher** itself.

![The menu's first page](docs/menu.png)

The menu knows nothing about any game, console or windowing library. A port tells it what rows to
show, hands it button presses, and gets back what the player chose; it draws itself into a buffer of
32-bit pixels over a copy of the game's picture. The same menu, with the same look and rules, then
works in every port that takes it.

- `src/` is C++17 with no dependencies beyond the standard library. No SDL.
- C interfaces (`menu_c.h`, `settings_c.h`) for ports written in C.
- `launcher/` is a program of its own (SDL 3 and Dear ImGui, fetched by its build).
- Nothing from any game is in this repository, and nothing from a game is needed to build or test it.

## Who uses it

| Project | What |
| --- | --- |
| [cps2recomp](https://github.com/tmcleroy/cps2recomp) | A toolkit for native ports of Capcom CPS-2 arcade games; keeps a copy in `recomp-common/` |
| [satrecomp](https://github.com/tmcleroy/satrecomp) | A toolkit for native ports of Sega Saturn games; keeps a copy in `recomp-common/` |
| [Vampire Savior Recomp](https://github.com/tmcleroy/vampire-savior-recomp) | A native port of *Vampire Savior* (CPS-2) with a full training mode, made with cps2recomp: the menu and launcher you see there are this library's ([download](https://github.com/tmcleroy/vampire-savior-recomp/releases)) |

To play a game, download a port; this repository is for people making ports.

## What the menu does

- A first page in groups with a line between them: save states (save, load, a slot from 1 to 9),
  what the game offers (the port's own rows, the game's speed, a row for each page of optional
  features), and leaving (reset, quit, resume).
- A page per category of the game's optional features (cheats, training aids, picture options) and
  commands, built from one list the port hands over. A feature is on or off, or a choice of several
  named things.
- A description of the chosen row, a line for messages ("Nothing is kept in slot 3."), and a hint
  line naming the buttons as this console calls them.
- Six buttons: up, down, left, right, confirm, back. The port maps its own onto them.
- Any picture size: sizes follow the picture's height, and where the rows do not all fit, the ones
  about the chosen row are shown.

![A page of features](docs/menu-page.png)

## The launcher

![The launcher's Features page](docs/launcher.png)

One launcher for every port: a Game page (the player's own game file, which is only ever read; the
picture's options; Play), a Features page (the presets Pure, Enhanced and Custom, and a switch or a
choice for each feature the game lists), and a Controllers page (connected gamepads, a tester, and a
key and a gamepad button for each of the game's buttons). All of it can be used with a gamepad alone,
or the arrow keys with Space or Enter, as well as with the mouse. A port adds one file that describes
itself: the words for its game file and the check of it, what the game is started with, which
buttons can be chosen. A port that is not about one game file can add pages of its own and leave
the Game page out. The launcher and the game read and write the same `settings.ini` by the same
rules (`src/settings.h`), so they cannot disagree about what a line in it means.

Its look, and the files that make it, come from the launcher of
[wiiparty-recomp](https://github.com/arelkair/wiiparty-recomp) by arelkair; see
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

## Try it

```sh
make test       # the menu's rules, and the C interface compiled as C
make example    # build/shot: a host with no game
build/shot menu.ppm ddddar     # press Down x4, Confirm, Right; write the picture
```

`examples/shot.cpp` is a complete host in about a hundred lines and is the place to start reading.

```sh
make launcher   # build/launcher/example-launcher, with a stand-in for the game (needs cmake; SDL 3 is fetched if absent)
make launcher-shots            # its three pages as pictures, with no window shown
```

`launcher/example` is what a port's own launcher folder looks like.

## Using it in a port

The library is not installed and has no package: a port keeps a **committed copy** of `src/` and
`launcher/` and compiles it with the rest of its code, so the port builds from a fresh clone with nothing else
present. `tools/sync-common.sh` makes the copy, records which commit it came from, and (with
`--check`) fails if anyone changed the copy by hand.

[docs/USING.md](docs/USING.md) has the steps, the interface in full, and the rules a port is
expected to follow (what opens the menu, what "Quit game" means, the order of the first page).

| File | What |
|---|---|
| `src/menu.h`, `menu.cpp` | The menu. |
| `src/menu_c.h`, `menu_c.cpp` | The same menu for hosts written in C. |
| `src/settings.h`, `settings.cpp` | The settings file and the rules for features: presets, values, choices. |
| `src/settings_c.h`, `settings_c.cpp` | The same rules for hosts written in C. |
| `launcher/` | The launcher: `launcher.cmake` (what a port includes), `src/` (`launcher.h` is what a port fills in), `example/`. |
| `src/text.h` | A 5x7 dot-matrix font on a canvas of 32-bit pixels; usable on its own for a port's readouts. |
| `tools/sync-common.sh` | The script a port keeps to take and check its copy. |
| `examples/shot.cpp` | A host with no game. |
| `tests/` | `make test`. |

## Status

Version 0.4.0 (see [CHANGELOG.md](CHANGELOG.md)). The interface may still change before 1.0; a
port is never changed under its feet, because it only moves to a new version when it runs the sync
script.

## Contributing

Bug reports and pull requests are welcome: see [CONTRIBUTING.md](CONTRIBUTING.md). The one hard
rule: nothing from any game, and no keys, ever go in this repository.

## License

[GNU General Public License v3.0](LICENSE) (`GPL-3.0-only`). A project that includes the copy must
itself be distributed under terms compatible with it, keep the notices at the top of the files, and
ship the license text. [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) says where everything came
from.
