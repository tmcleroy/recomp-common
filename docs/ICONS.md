# Icons

Every port has one icon. It is the app's (macOS, Windows, Linux, Android), the window's while a game runs, and it sits
beside the name in the launcher's top bar and at the left of the in-game menu's command row. This page is how they
are drawn, so that each port's looks like one family, and how one is made and wired in. cps2recomp's is the first
(`assets/icon/` there): read its `icon.py` beside this page.

## The design language

**Draw the port's own hardware, as simple solid shapes.** The CPS-2 icon is the two plastic shells that make a CPS-2:
the B board (the game) stacked on the A board (the machine), seen from above at an angle. Another port draws its own:
the cartridge, the board or the disc its games came on. What the hardware *is*, never what the maker printed on it.

**Never:** a maker's logo or name, lettering moulded or printed on the hardware, a game's label or box art, characters,
screenshots, or anything traced from a photo. Shapes, proportions and colours of the hardware only, drawn from scratch.

**The tile.** A 1024 x 1024 SVG, its ground a rounded square (`rx="224"`) in the menu's ground colour, `#0d1016`.
The art spans about the middle 64% (x 192 to 832) and sits a little low of centre (CPS-2: everything moved down 42),
so the tile's corners never crop it and macOS's own rounding of the dock icon leaves it whole.

**The projection.** Isometric 2:1: an edge running right goes 320 across and 160 down (or up), an edge down goes
straight down. A slab is a top face (a rhombus) and its two front faces, 90 deep; stacked parts are 130 apart (a 40
gap shows the lower one's top). Light from the top left: the top face lightest, the left face middle, the right face
darkest, the same hue.

| Part | Top | Left | Right |
|---|---|---|---|
| CPS-2 B shell, green (Japan) | `#4cb26e` | `#2e8a50` | `#236b3e` |
| CPS-2 A shell, blue (USA) | `#3d73c9` | `#2a58a3` | `#1f4682` |

Other colours, the hardware's own where it has one (grey `#8d97a8` / `#6f7888`, orange `#e08a3c` / `#c9732e`), else
the menu's: its accent `#7cc4ff`, the CRT look's yellow `#ffd640`, its navy `#18256a`.

**Details, three kinds at most,** each the hardware's: vents (rounded slots, black at 32%), contacts (the edge
connector's gold fingers, `#d9b45a`), a blank label (`#e8ecf2`). Every detail goes ON a face, through the face's
own transform, so it lies flat on it.

**The mark.** On the label, the in-game menu's own 5x7 dot font (`src/text.h`), one character in `#0d1016`: the
CPS-2's is a 2. It ties the icon to the menu and the launcher, and tells the ports apart when their hardware looks
alike: a 3 for CPS-3, N for NAOMI, S for the Saturn ports, and so on. Dots 60 on a 70 grid, centred on a 560 label.

**Sizes.** It must read at 32 px (two colours, the stack, the label) and still be the right two colours at 16 px.
Check 16, 32 and 64 before showing it to anyone.

## Ideas for the other ports (not yet drawn)

- **sf3comp (CPS-3):** the CPS-3's game is a cartridge plus a CD: one slab with a disc half out of it, mark 3.
- **naomicomp (NAOMI, Atomiswave):** a cartridge board, long and flat, its contacts along one edge, mark N.
- **The Saturn ports:** a disc, flat-on at the same angle, a hub and a label ring, mark S (or the game's initial).
- **spelunkomp (Cave):** a bare circuit board (no shell): a green board with chips, mark the cave's.
- **Soulcalibur II (Xbox):** a disc, mark the game's initial.
- **Bonkers (SNES):** a grey cartridge, top-down, its label blank, mark B.

## Making one

1. Draw it as a script that writes the SVG (cps2recomp's `assets/icon/icon.py`): faces as polygons, details through a
   face's affine map, the script writing only plain `<polygon>` and `<rect>` with absolute points.
   **No `transform=` in the SVG:** ImageMagick's own renderer, which `make_icons.sh` uses, puts transformed shapes in
   the wrong place (the CPS-2 label landed off the board).
2. `tools/make_icons.sh icon.svg OUT ANDROID_RES` writes every size and form: PNGs 16 to 1024, `icon.icns` (on a Mac),
   `icon.ico`, `icon_png.h` (the art without its tile, for the launcher's and the menu's own dark grounds: the tile is
   found by its `rx="224"`, so keep that on the tile alone) and the Android mipmaps. Commit them all: a build needs no
   ImageMagick.
3. Wire it in:
   - **macOS:** `icon.icns` into `Contents/Resources/`, `CFBundleIconFile` = `icon` in Info.plist.
   - **Windows:** a `.rc` with `1 ICON "icon.ico"` in the program's sources (the .exe's own icon).
   - **Linux:** `icon-256.png` beside the program and in a `.desktop`'s `Icon=`; the window's icon is set at run time.
   - **Android:** the mipmaps, `mipmap-anydpi-v26/ic_launcher.xml` an adaptive icon of `@mipmap/ic_launcher_foreground`
     over `@color/ic_launcher_background` (the tile's colour, `#0d1016`).
   - **At run time** (`src/icon.h`): `sdl::SetWindowIcon(window, kIconPng, kIconPngSize)` for each game window;
     `Description::icon_png` / `icon_png_size` for the launcher (its window and its top bar);
     `MenuConfig::icon = sdl::MenuIconOf(kIconPng, kIconPngSize)` for the in-game menu's command row.
