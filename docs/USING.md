# Using recomp-common in a port

## Taking the copy

1. Clone this repository beside your project's (`../recomp-common`), or anywhere and point
   `RECOMP_COMMON` at it.
2. Copy `tools/sync-common.sh` into a `tools/` folder of your project. The copy of the library goes
   in a `recomp-common/` folder beside that `tools/` folder.
3. Run it:

   ```sh
   tools/sync-common.sh
   ```

   It copies `src/` exactly as committed at this repository's HEAD and writes
   `recomp-common/COMMIT`: the commit, and a SHA-256 of each file.
4. Commit `recomp-common/` and the script in your project.
5. Add `recomp-common/menu.cpp` to your build (C++17). A project in C also adds
   `recomp-common/menu_c.cpp`, compiles those two files with a C++ compiler and links with it (or
   links the C++ standard library).
6. Add the check to whatever you run before committing, and to CI:

   ```sh
   tools/sync-common.sh --check
   ```

   It fails if the copy is not what `COMMIT` records. With a clone of this repository at hand it
   also compares each file with that commit.

**Never edit the copy.** To change the menu, change it here (and run `make test`), then run the sync
script in each project and commit the new copy there. Projects move to a new version only when they
run the script, so they can be on different versions; the `COMMIT` file says which.

The copy is GPL-3.0 code. Keep the notice at the top of each file, and ship the license text with
your project.

## The interface (C++)

Everything is in `menu.h`, namespace `recomp`. `examples/shot.cpp` uses all of it.

```cpp
recomp::MenuConfig config;

// The first page, in groups; a line is drawn between groups.
config.groups = {
    {{MenuRow::Save}, {MenuRow::Load}, {MenuRow::Slot}},
    {{MenuRow::Pages}, {MenuRow::Speed}, {MenuRow::Action, kInsertCoin, "Insert coin", "Puts a coin in the first slot."}},
    {{MenuRow::Action, kReset, "Reset", "Starts the machine again."}, {MenuRow::Quit}, {MenuRow::Resume}},
};

// The game's optional features and commands. The menu makes a page for each category.
config.features = {{"Cheats", "Endless time", "The round's clock stands still."},
                   {"Cheats", "Health", "Whose health is kept full.", {"Off", "Player 1", "Both"}}};
config.commands = {{"Training", "Training mode", "Straight to a practice round."}};
config.value = [](size_t feature) { return /* 0 = off, or which choice */ 0; };
config.setValue = [](size_t feature, int value) { /* change it; write the settings file */ };

// The Speed row: the names, the first being full speed. What a speed means is the port's affair.
config.speeds = {"Full", "1/2", "1/4"};
config.speedDescription = "Left/Right: slow motion.";

// What the menu's buttons are called on this console, for the hint line.
config.confirmName = "A, B or C";
config.closeName = "Esc";

menu.Configure(std::move(config));
```

| Row kind | What it is | What `Buttons` returns when chosen |
|---|---|---|
| `Save`, `Load` | "Save state", "Load state" | `Chosen::Save`, `Chosen::Load` (use `Slot()`) |
| `Slot` | The slot, 1 to `slots` (9 unless the config says) | nothing: the menu keeps it (`Slot()`, `SetSlot()`) |
| `Speed` | One of `speeds`; left out if there are none | nothing: the menu keeps it (`Speed()`, `SetSpeed()`) |
| `Pages` | Stands for several rows: the commands of no category, then a row per category | a command: `Chosen::Command` with its index |
| `Action` | The port's own row: a title, a description, an id | `Chosen::Action` with the id |
| `Quit` | "Quit game" | `Chosen::Quit` |
| `Resume` | Closes the menu | nothing |

While the menu is open, each time buttons are newly pressed:

```cpp
unsigned buttons = 0;   // kMenuUp, kMenuDown, kMenuLeft, kMenuRight, kMenuConfirm, kMenuBack
const recomp::Menu::Chosen chosen = menu.Buttons(buttons);
switch (chosen.kind) {
case recomp::Menu::Chosen::Save:    /* save to menu.Slot(); menu.Close() if it worked */ break;
case recomp::Menu::Chosen::Load:    /* or menu.Note("Nothing is kept in slot 3.") if it did not */ break;
case recomp::Menu::Chosen::Action:  /* chosen.id is the row's id */ break;
case recomp::Menu::Chosen::Command: /* chosen.id is its place in config.commands */ break;
case recomp::Menu::Chosen::Quit:    /* end the game */ break;
default: break;
}
```

The port closes the menu after carrying out what was chosen (so that a failure can leave it open
with a note). Resume, and Back on the first page, close it themselves.

To show it, copy the game's picture as it stood when the menu opened and draw over the copy each
time something changes:

```cpp
shown = picture;                                  // 32-bit pixels, red in the low byte
menu.Draw(shown.data(), width, height);           // then show `shown` however you show pictures
```

Sizes follow the picture's height (one font dot is `height / 300` pixels, at least 1), so draw at
the size the player sees, not at the console's resolution, if you want sharp text in a big window.

### How the buttons behave

- Up and Down choose a row and wrap around.
- Confirm chooses a row, switches a feature, goes to the next of several values (around), or goes
  into a page.
- Left and Right go through a row's values and stop at the ends. On a row with no values, Left
  leaves a page for the one it was reached from, and Right on a page's row goes into that page.
- Back leaves a page for the one it was reached from; on the first page it closes the menu.
- A `MenuRow::Mute` row ("Mute", Off or On) switches the game's sound off and on through `MenuConfig::muted` and
  `setMuted`. The Speed and Mute rows may be on a page rather than the first (`MenuPage::rows`). Give Space, Enter and a gamepad's A to Confirm, and Backspace and B to Back.
- The menu opens on the page and the row it was closed on.

### Tabs

Give `MenuConfig::tabs` and the menu is tabbed: a row of commands at the top (Back at its left, Resume at its right,
`MenuConfig::bar` between: Save state, Load state, Slot, Reset game, Quit game), the tabs under it and the tab's rows
under them. `groups` is then not shown. `layout::Tabbed(config, rows)` (`menu_layout.h`) does it all for a port on the
standard layout: it calls `Standard`, then makes the nine standard tabs, which every port names and orders the same:

| Tab | Its pages |
|---|---|
| Game | Game (Speed, Mute and the game's options), Board; the port's own rows, any page no tab has, Reset all options |
| Display, Sound, Online, Stage, Cheats | the page of the same name |
| Training | Training mode (Dummy under it) |
| Modes | Alternative modes, and the port's other ways to play (`Rows::modes`) |
| Capture | Video, Scene export |

A tab with nothing on it is left out. A page under a tab's page is a row that opens it in the tab's place, with where
it is said above its rows. With two pages or more, each tab's page is under a heading of its name.

- Up from a tab's top row goes to the tabs (Left and Right change tab there), and Up again to the command row (Left
  and Right go along it); Down comes back. Down on the last row goes round to the top one.
- `kMenuTabPrev` and `kMenuTabNext` change tab from anywhere, round; each tab keeps the row it was left on.
- `kMenuDefault` puts the option chosen back to its default: `MenuConfig::defaultValue` (none: 0, off). A row whose
  value is not its default is marked, and each tab says how many it has.
- Left and Right set a switch off and on.
- Reset game and Quit game in the command row want a second press straight after the first; the hint line says so.
- Back leaves a page for the tab; on a tab it closes the menu.
- `menu_sdl.h` gives the buttons: Q and E and the shoulder buttons change tab, Delete and a gamepad's X (west) put back
  the default, and names them for the hint line (`kTabsName`, `kBackName`: `MenuConfig::tabsName`, `backName`).

#### The icon

`MenuConfig::icon` (`MenuIcon`: pixels, any size) is drawn at the left of the command row, scaled to the row; `icon.h`'s
`sdl::MenuIconOf(kIconPng, kIconPngSize)` makes it from the port's PNG. It is left out where the row is at its
tightest. Making and wiring the icon everywhere: docs/ICONS.md.

#### Hotkeys

A player can give any option a key or a gamepad button, which steps it while playing: Run-ahead through its frames,
the shader through its list, a switch on and off. `hotkey.h` does the work with SDL; the port wires it up:

- `MenuConfig::hotkey(feature)`: the name of the key or button the feature has (`Hotkeys::Of(id).Name()`), shown on its
  row; `hotkeyName` for the hint line (`sdl::kHotkeyName`, "H or Y"). Without `hotkey` there are no hotkeys.
- `kMenuHotkey` (H, a gamepad's Y) on a feature's row asks for one; while `Menu::Listening()` is a feature, each SDL event
  goes to `sdl::HotkeyListen(event, menu, hotkeys, ids, taken, changed)` first (Esc or B cancels, Backspace or X takes
  the hotkey away). `taken(Hotkey)` says why a key or button cannot be one ("That key plays the game."), or "".
  The menu's own keys and buttons (`sdl::MenuOwn`) never can. One key, one feature: giving it to another takes it
  from the first.
- In the game, `sdl::HotkeyPress(event, menu, hotkeys, ids)` steps the feature (Shift with a key: back) through
  `Menu::Step`, round, the choices `offered` alone, and returns what to show ("Run-ahead: 2 frames"); "" when the
  event was no hotkey, which then goes to the game as before.
- `Hotkeys::Save()` / `Load()` keep them as one line of the game's settings, by the features' ids.

It has two looks, which share everything but the drawing, the mouse's places included:

- **The window look** (the usual): a card over the darkened game, the command row, a strip of tabs, the rows, and
  beside them what the chosen row does and, for a choice, its choices (the default said), which the mouse can click.
  Where the picture is narrow, the description goes under the rows. `Menu::SetFont` draws its text in a font of the
  port's (`MenuFont`); without one, the menu's dot font.
- **The CRT look** (`Menu::SetCompact(true)`): for a picture at the board's own size, a 15 kHz screen's. Capitals in the
  dot font, a solid bar for the row chosen, one tab at a time between arrows with its place (5/9), a navy ground
  (a white field blooms on a CRT) and a margin for the overscan.

## The interface (C)

`menu_c.h` is the same thing as plain functions and structs: `rmenu_new`, `rmenu_configure` (rows
carry a `group` number instead of being nested; feature choices are a `NULL`-ended array of names),
`rmenu_open`, `rmenu_buttons` (returns `RMENU_SAVE`, `RMENU_ACTION` and so on, with the id through a
pointer), `rmenu_draw`. Everything given to `rmenu_configure` is copied except the two function
pointers and `user`. `tests/c_test.c` is a complete example.

## What a port is expected to do

These make every port feel the same. Two headers do most of it: `menu_layout.h` lays the menu out, and
`menu_sdl.h` (for a port on SDL 3) gives it its controls.

- **The standard layout** (`recomp::layout::Standard`, after the features, commands and the port's own pages):
  - Online > · Display > · Game > (and Stage >, Replays >)
  - Alternative modes > (Training mode > Dummy >, and the other ways to play)
  - Save state · Load state · Slot
  - The other pages · Insert coin · Player 2 joins · Test menu (those the board has)
  - Reset all options · Reset game · Quit game · Resume

  Speed and Mute are at the top of the Game page; Board, Cheats and Sound are under it. Categories take the names in
  `menu_layout.h` (`kDisplay`, `kCheats`, `kTraining`, ...). The speeds are `layout::Speeds` (1/8 to 8x, Full in the
  middle). Every port has Reset game, Reset all options and Quit game.
- **The controls** (`menu_sdl.h`):
  - Esc, F1, a phone's Back, Tab (when the game hasn't got it), a gamepad's Guide, its left stick's click, and Start
    and Back together open the menu and close it, from any page. Which of the gamepad's do is the player's choice, the
    launcher's **Menu button** (Controllers page; `menu_buttons.h`, given to the game in `RECOMP_MENU_BUTTON`): Guide,
    left-stick click or Start+Select (recomp-common's own), Guide or Start+Select, or Start+Select only. Start and Back
    together always do. A port asks `sdl::MenuButton(button)` rather than naming the buttons itself, and changes what
    its players start with by building its game with `RECOMP_MENU_BUTTON_DEFAULT` and its launcher with
    `LAUNCHER_MENU_BUTTON` (the same key: `guide`, say).
  - In the menu, the arrows, the d-pad and the left stick choose, and held they go on (`MenuRepeat`). Enter, Space
    and A confirm. Backspace and B go back a page.
  - The mouse: over a row chooses it, a click confirms, the right button goes back, the wheel goes up and down
    (`sdl::MouseEvent`, given where the menu's picture is on the window).
- **The menu turned with the picture** (`menu_turn.h`), in a port that can show a vertical game as the board sends
  it, for a monitor turned on its side: a **Menu orientation** feature on the Display page (`turn::kTitle`,
  `kDescription`, `Choices()`: Auto, the default, Upright and the three turns). `turn::MenuTurn` gives the menu's turn
  on the screen (Auto: as the game's picture stands). The port draws the menu on a picture of `TurnedSize`'s shape
  (so it lays itself out for the turned screen), turns it onto the screen's with `TurnPixels`, does the same with its
  own notes and readouts, and gives the turn in `sdl::MenuPlace::turn`, which maps the mouse (and with `ToMenu`, the
  fingers) back through it. The stick and the arrows need nothing: they are held upright, as the menu now stands.

  Nothing but the menu's "Quit game" row (and closing the window) ends the game; Esc does not.
- **The keys while playing:**
  - F5 saves the state, F9 loads it, F6 and F7 choose the slot.
  - P stops the game and starts it again; O goes on by one frame; - and = change the speed.
  - F11 or Alt+Enter: full screen. F12: a screenshot.
  - Backspace: start over (in training). F2: the board's test switch; 9: its service switch.
- **The game stands still** while the menu is open, and buttons held as it closes do not reach the game until they
  are let go.
- **With the menu closed nothing changes:** no cost per frame, no change to input timing.
- **A test flag** that opens the menu at a frame, presses given buttons and writes the picture
  (`--menu FRAME:KEYS[:shot.png]`), so the menu can be checked without a window.

## An arcade monitor (SDL 3)

`src/arcade_sdl.h` (header only) is what an arcade port needs for a CRT driven at the board's own rates (CRT
Emudriver, or a custom resolution): full screen in exactly the board's resolution, or in a *super resolution* of the
board's lines and 1920 columns or more (240 lines where the card has none of 224), at the refresh nearest the board's;
and a choice of screen. The port gives it the board (`ArcadeBoard`: columns, lines, frame rate) and the player's
choices, which it keeps as three features under an **Arcade monitor** heading, the first two set on the launcher:

- *Full screen resolution* (`Off`, `ArcadeBoardName(board)`, `Super resolution`; `ArcadeDescription(board)`):
  `SetFullscreenMode` when the window is made, `ArcadeFollow::Step` each frame.
- *Screen* (Main, Second, Third, Fourth; `kScreenDescription`): `ChosenDisplay`, `CentreOnScreen`.
- *Compact menu* (`kCompactDescription`): `Menu::SetCompact`, on by itself while `OnArcadeMode`.

On an arcade mode the picture fills the screen (the monitor gives it its shape), with `ArcadeLines` keeping its lines
one for one on a mode of more lines; and a vertical game is shown as the board sends it, for a monitor on its side.

`src/arcade_screen.h` (with `arcade_screen.cpp`, and on Windows Switchres: `include(recomp-common/switchres.cmake)`,
`recomp_switchres(<target>)`) is the rest of the page, three more features under the same heading:

- *Make the mode at the board's rate* (on/off, launcher only, only where `SwitchresBuilt()`; `SwitchresDescription(board)`)
  and *Monitor* (`kMonitorNames`, launcher only; `kMonitorDescription`): full screen has Switchres make the board's or
  the super resolution's mode at exactly the board's rate within that monitor's ranges and switches the desktop to it,
  the desktop back at the end; the listed modes where it can't.
- *Keep the board's pace* (on/off, off by default; `BoardPaceDescription(board)`): off, in full screen at an arcade mode
  the frames are paced by the screen (VSync on, one frame a refresh), with a frame delay where the system says when it
  refreshes (Windows); on, the board's clock as in a window. Online always the board's clock.

The port keeps one `ArcadeScreen` and fills an `ArcadeChoices` from its features each frame: `SetFullscreen` when the
window is made, `Follow` each frame (instead of `ArcadeFollow`), `OnArcade` (instead of `OnArcadeMode`), `Paced` to
know whether the screen paces this frame (VSync on; no frame timer), `WorkDone` just before the picture is drawn and
shown, `Shown` just after (it waits as the frame delay says, and gives the next frame's time), `BoardPace` on the board's
clock, `Menu` while the in-game menu is up, `Close` at the end. The game's log says the mode chosen or made, and every
ten seconds the rate measured and the frame delay.

## Text on its own

`text.h` (namespace `recomp::text`) is a header with no dependencies: a `Canvas` over a pixel
buffer with `Fill` (a blended rectangle) and `Text` (wrapped at spaces to a width), and `TextWidth`
and `TextLines` to measure. Ports use it for readouts over the picture as well.

## Settings: the rules a launcher and its game share

`settings.h` (namespace `recomp`; `settings_c.h` for C) is the one place that says what the
`[features]` section of a port's `settings.ini` means:

```ini
[features]
preset = enhanced        ; pure | enhanced | custom
free_play = 0            ; a feature's own value wins over the preset: 0, 1, or the number of a choice
```

- **The game lists its features** for the launcher: `--list-features` prints
  `recomp::FeatureListText(features)`, a feature a line (name, category, flags, title, description
  and, when it is a choice of several things, their names with `|` between them, tab-separated).
  Features flagged `kFeatureSession` last one run and are not listed or saved.
  A line `recomp::FeatureListNote(text)` after them is a note about the program itself (a build with none of the
  game's code translated, which plays slower, say): the launcher shows it on the game's row and under its Play.
- **Both read values the same way:** `recomp::FeatureValues(ini, features)` gives every feature's
  value: the file's if it has one, else the preset's (on only in `enhanced`, for features flagged
  `kFeatureEnhanced`). Of a category's `kFeatureExclusive` features only one is on.
- **Both change a value the same way:** `recomp::SetFeature(ini, features, index, value)` makes the
  preset `custom`, writes down the features that were following the preset so that nothing else
  changes, and never rewrites a value that was already in the file. `recomp::SetPreset` chooses a
  preset.
- **A choice kept by name**: a feature whose choices may be put in another order later gives each one a stable
  name in `FeatureInfo::ids` (the first, off, `off`), and in `numbered` the ids its numbers meant before it had them
  (in that old order). Its value is then kept as `name_id = <id>` and, beside it, `name = <the id's old number>` (none
  for an id with no old number), so an older build that knows only the numbers reads the same choice; the menu's
  choices offered the same way (`[menu] name_id = crt,xbrz` and `name = 1,10`). Reads go by `name_id` (an id this
  build does not know: the number). `recomp::MigrateFeatures(ini, features)` makes a file from before the ids over
  once, and the two keys agree; the launcher does it as it reads a game's features, and a game should as it starts.
  `--list-features` carries the ids and the old order as a seventh and eighth field.
- `recomp::Ini` is the file itself: `load`, `get`, `set`, `save`. It keeps every line it was not
  asked to change, comments and other sections included, so the launcher, the game and the player
  can all write to one file.

With every feature off (the `pure` preset, or no file) a game must run exactly as the original.

## The player's own shader presets

`own_shaders.h` (namespace `recomp::shaders`, header only) is for a port whose Shader feature draws through RetroArch's
`.slangp` presets (librashader) and has a choice, "Your own", for the player's. The port keeps its built-in presets
and its librashader code; this is the rest:

- **One folder for every game**: `shaders/` in the launcher's own folder (beside `launcher.ini`). The launcher makes
  it when it starts (`Description::own_shaders` names the feature and its Your own choice) and tells the games it
  starts in `RECOMP_SHADERS`; a game started on its own finds it beside its own folder with `SharedFolder(game_folder,
  LAUNCHER_GAME_NAME)`. A game's own `shaders/` (`GameFolder`) comes first, and its preset of a name wins.
- **Found by name**: `Find(GameFolder(..), SharedFolder(..))` lists every `.slangp` in either folder and the folders
  under it (a RetroArch pack goes in whole), each by its path there (`crt-pack/newpixie-crt.slangp`).
- **Kept by name**: `[shader] own` in each game's `settings.ini` (`Chosen`, `Choose`); none is the first found.
  `Pick` gives the preset to draw with, or why there is none.
- **The same list in the launcher and the menu**: `MakeChoices` ("The first one found", then each preset, and a chosen
  one that is gone at the end), whose `IndexOf` and `NameAt` turn a place into a name and back. The launcher draws it
  under the feature as **Your own preset** (looked at again every two seconds) with the folder's row and **Open
  shaders folder**; a game's menu builds it anew each time it opens (`Menu::Configure` before `Open`) as a feature of
  its own, `kTitle`, with `MenuDescription(shared, game_own)` saying where the folders are. Choosing one chooses Your
  own. A preset that fails to load should say librashader's reason, in the menu (`Menu::Note`) or over the picture,
  and in the log.

## The launcher

A port's launcher is the shared one plus one file of its own. `launcher/example` is a complete
example with no game; a port's `launcher/CMakeLists.txt` is its `CMakeLists.txt` with other names:

```cmake
set(LAUNCHER_GAME_NAME "My Game")          # as the player reads it
set(LAUNCHER_GAME_EXE mygame)              # the game program, beside the launcher (mygame or mygame_app)
set(LAUNCHER_APP_ID org.example.mygame)
set(LAUNCHER_HOME_ENV MYGAME_HOME)         # a variable naming a folder to use instead of the player's
set(LAUNCHER_DISCLAIMER "Not affiliated with ...")
set(LAUNCHER_PORT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/description.cpp)
include(../recomp-common/launcher/launcher.cmake)     # makes the target `launcher`
```

`description.cpp` defines `launcher::describe()` (`launcher/src/launcher.h`):

| Field | What |
|---|---|
| `original` | Where the original ran, as the Features page says it: "in the arcade", "on the console". |
| `file_heading`, `file_intro`, `file_empty`, `file_not_set_up`, `file_filter`, `file_key` | The words for the player's game file, the file dialog's filter, and its key in `launcher.ini`. |
| `check` | Looks at the player's file and says `Good` (the file the port was made from), `Usable` (it may run) or `NotUsable`, with a line for the player. It must only read the file. |
| `check_on_thread` | For a check that reads a large file: the page says "Checking..." meanwhile. |
| `find_nearby` | Optional: a game file near the program, for a build inside the source tree. |
| `find_beside` | Optional: where in the folder the download was unpacked to (`recomp::InstallFolder`) the game's file may be, for a release that ships one beside the launcher (a `roms` folder). Looked for when the launcher opens and the game has no usable file; tried in turn with `check`, and only a file it says is usable becomes the game's. Read only, once for each game that has none, on a thread. |
| `arguments` | What the game is started with, from the file, the player's folder and the Display switches. |
| `mute_argument` | What the game is started with when the Game page's "Muted" switch is on (`launcher.ini`, `[audio]`); `"--mute"` unless the port says otherwise, `nullptr` for no switch. The game's menu should have a row that switches the sound off and on. |
| `folders` | Folders the player may choose for the game, as rows of the Sound card with Choose… and Clear (`FolderChoice`: the key in `launcher.ini`, beside the game's file; title; what it is for; the argument). While one is chosen the game is started with `ARGUMENT FOLDER` after the port's own arguments. The system's folder dialog chooses, or the launcher's own browser when a gamepad is in use. |
| `display` | The Display card's switches (`launcher.ini`, `[video]`): key, title, detail, the default, and another switch it needs. |
| `bindings` | The game's buttons and their usual keys and gamepad buttons (`bindings.h`). With it, the Controllers page lets the player choose them, for each of its players: one to four (`BindTable::players`; two unless it says), and which connected gamepad is which player's (`[controllers]`; the game gives its gamepads out with `AssignPads` when one is connected or let go). |
| `controls_heading`, `controls` | Rows of text for what is fixed (the menu's keys and so on). |
| `subtitle` | The line under the game's name in the sidebar. |
| `pages` | Pages of the port's own, before the launcher's in the sidebar (below). |
| `game_page` | `false`: no Game page, game file or Display card; the port's pages have Play. |
| `features_name`, `features_intro`, `presets` | Another name and other first lines for the Features page, and whether it has the preset chooser. |
| `launcher_features_only`, `board_category`, `controller_features` | Only the features marked `kFeatureLauncher` (settings.h) on the launcher's pages: what has to be set before a game starts. The rest are the in-game menu's alone, still saved and set by the presets. `board_category`: a small card of that category's features on each game's page, set for that game alone. `controller_features`: features by name on the Controllers page. |
| `layout` | `Layout::Sidebar` (the usual) or `Layout::Tabs`: the in-game menu's look, the pages as tabs along the top (LB/RB go through them), a collection's games as a grid of pictures (`picture.png` in a game's folder, which the game can leave there on quitting, else its newest screenshot, else its name) with the chosen game's Play, Host online, Display switches and its own page beside it. |

**Pages of a port's own.** A `launcher::CustomPage` is a name and a `draw(PageContext&)`, and
optionally `drop` (a file dropped on the window) and `busy` (work is going on on another thread:
keep drawing). The page is drawn inside the launcher's page column with `ui.h` (text, buttons,
switches, dropdowns) and `layout.h` (`heading`, `label`, `Card`, `row`, `inset_separator`), so it
looks like the others. The `PageContext` has the column's width, the window (for file dialogs),
`launcher.ini` and `settings.ini`, the player's folder, the program's folder, whether the game is
running, and `play()`, which starts it with the port's `arguments`. A port's pages come out in the
`--screenshot` pictures like the others, in the sidebar's order.

The player's choices are kept in their folder (`SDL_GetPrefPath("", LAUNCHER_GAME_NAME)`, or the
folder `LAUNCHER_HOME_ENV` names): `launcher.ini` is the launcher's own (where the game file is, the
Display switches), `settings.ini` is the one the game reads too (features, keys and buttons) and is
handed to it by the port's `arguments`.

**Keys and buttons.** `launcher/src/bindings.h` (it needs only SDL 3) is the format and its reader:
`[keyboard1]`, `[gamepad1]` (and `2` for a second player), an action's name and a key, button or
trigger by SDL's name, or `none`. A game written against SDL 3 includes the same header and reads
the file once when it starts (`recomp::LoadBindings`); while playing it looks only at that table, so
choosing keys adds nothing to reading input. A game not on SDL 3 reads the same sections itself.

**A gamepad, or the arrow keys, alone.** Everything in the launcher can be reached and pressed
without a mouse: the d-pad, the left stick or the arrow keys move a ring; A, Space or Enter presses
what it is on; B or Esc goes back (closes a list, or goes from a page to the sidebar); the shoulder
buttons change the page; the right stick scrolls. (What the mouse does by dragging, a collection's
games' order in the sidebar, the ring does with Move up and Move down on the game's page.) A port's own pages need do nothing for this if
they are made of `ui.h`'s widgets. A game's in-game menu should answer to the same: Space, Enter and
a gamepad's A choose, Backspace and B go back, whatever the game's own buttons are set to.

**Checking it without a window.** `<game>-launcher --screenshot PREFIX [--size WxH]` writes a
picture per page and shows nothing. With `SDL_VIDEO_DRIVER=dummy SDL_RENDER_DRIVER=software` it
needs no display at all, and two runs give the same bytes, so a port can compare before and after.
`--screenshot PREFIX --press KEYS` presses keys one after another and then writes one picture,
`PREFIX.png`: `u d l r` are the arrow keys, `e` Enter, `s` Space, `x` Esc; `U D L R A B [ ]` are the
d-pad, A, B and the shoulder buttons of a gamepad that exists only in the program; a digit shows that
page; `.` waits; `@X,Y;` moves the mouse to that place in the window, and `(` and `)` press and let
go of its left button (a drag: `@100,288;(@100,350;@100,406;)`).

**A small window.** The launcher works down to 600×400 (and never opens bigger than the screen): below
760 wide its sidebar is narrower and the page has less room round it, a card's row puts its control
under its words when there is too little room beside them (`layout::row`), and the sidebar's list
scrolls when it is longer than the window. A port's own pages built of `layout.h`'s rows close up
the same way.

A port that gives out a built launcher must include the licenses of SDL, Dear ImGui and the Inter
font with it; see `THIRD_PARTY_NOTICES.md`.
