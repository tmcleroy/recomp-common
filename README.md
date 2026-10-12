# recomp-common

Code shared between recomp ports (native ports of games made by static recompilation). So far: an
**in-game overlay menu** and the small bitmap text it is drawn with.

![The menu's first page](docs/menu.png)

The menu knows nothing about any game, console or windowing library. A port tells it what rows to
show, hands it button presses, and gets back what the player chose; it draws itself into a buffer of
32-bit pixels over a copy of the game's picture. The same menu, with the same look and rules, then
works in every port that takes it.

- C++17, no dependencies beyond the standard library. No SDL.
- A C interface (`menu_c.h`) for ports written in C.
- Nothing from any game is in this repository, and nothing from a game is needed to build or test it.

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

## Try it

```sh
make test       # the menu's rules, and the C interface compiled as C
make example    # build/shot: a host with no game
build/shot menu.ppm ddddar     # press Down x4, Confirm, Right; write the picture
```

`examples/shot.cpp` is a complete host in about a hundred lines and is the place to start reading.

## Using it in a port

The library is not installed and has no package: a port keeps a **committed copy** of `src/` and
compiles it with the rest of its code, so the port builds from a fresh clone with nothing else
present. `tools/sync-common.sh` makes the copy, records which commit it came from, and (with
`--check`) fails if anyone changed the copy by hand.

[docs/USING.md](docs/USING.md) has the steps, the interface in full, and the rules a port is
expected to follow (what opens the menu, what "Quit game" means, the order of the first page).

| File | What |
|---|---|
| `src/menu.h`, `menu.cpp` | The menu. |
| `src/menu_c.h`, `menu_c.cpp` | The same menu for hosts written in C. |
| `src/text.h` | A 5x7 dot-matrix font on a canvas of 32-bit pixels; usable on its own for a port's readouts. |
| `tools/sync-common.sh` | The script a port keeps to take and check its copy. |
| `examples/shot.cpp` | A host with no game. |
| `tests/` | `make test`. |

## Status

Version 0.1.0 (see [CHANGELOG.md](CHANGELOG.md)). The interface may still change before 1.0; a
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
