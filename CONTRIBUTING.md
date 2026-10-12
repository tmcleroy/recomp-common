# Contributing

Thanks for helping. Bug reports, fixes, and ports of the menu to hosts it has not met are all
welcome.

## The rules that are not negotiable

Pull requests that break them are closed.

1. **Never add anything from a game.** No ROM, disc or archive contents, no extracted files, text or
   graphics dumps, RAM dumps, save states, screenshots of a game, or code generated from a game.
   This library knows nothing about any game and must stay that way. The same goes for issues:
   describe the problem or attach a picture made with `build/shot`, not a game's files.
2. **Never add keys**, or point to where keys, ROMs or other copyrighted files can be found.
3. **Only add code you have the right to contribute**, under the GPL-3.0. Code from another project
   must have a compatible license and be credited in `THIRD_PARTY_NOTICES.md`.

`tools/audit.sh` checks the first two mechanically; CI runs it on every pull request.

## What a change must keep

- **No dependencies.** Plain C++17 and the standard library; no SDL, no platform calls. The host
  owns the window, the input and the files.
- **Nothing about a game or a console in the library.** If a port needs something special, it goes
  in what the host hands over (`MenuConfig`), not as a special case here.
- **The look does not change by accident.** Ports compare their menu screenshots before and after
  taking a new version. If your change moves a pixel on purpose, say so in the pull request and
  show before and after pictures (`build/shot`).
- **The C interface keeps up.** Something added to `menu.h` that a host needs is added to
  `menu_c.h` too, and to `tests/c_test.c`.

## Before you open a pull request

```sh
make test       # must pass; it is built with the address and undefined-behaviour sanitizers
make example    # must build
make audit      # must pass
```

Add a test for what you changed (`tests/menu_test.cpp`). Add a line to `CHANGELOG.md` under
"Unreleased". Match the code around you: its naming, its comment style, 4 spaces in C++ and 2 in
the C files.

## Reporting bugs

Open an issue with what you did (the `MenuConfig`, or the `build/shot` keys that show it), what
happened, what you expected, the picture's size, and the commit your copy is at (the `commit` line
of your `recomp-common/COMMIT`).

## Security

See [SECURITY.md](SECURITY.md).
