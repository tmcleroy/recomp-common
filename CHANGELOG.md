# Changelog

Versions follow [semantic versioning](https://semver.org/). Before 1.0 the interface may change in
a minor version. A port's copy only changes when it runs `tools/sync-common.sh`.

## Unreleased

- In-game menu: a description longer than three lines no longer runs over the hint at the panel's
  foot. The panel leaves room for the longest description on the page (three lines at least), and
  the rows close up when the picture is short.
- In-game menu: the panel is opaque; the game's own text no longer shows faintly through it.

## 0.4.0 (2026-10-13)

- **The launcher works with a gamepad, and with the arrow keys, alone.** The d-pad, the left stick
  or the arrow keys move a ring from one button, switch, list or sidebar row to the next; A, Space
  or Enter presses what the ring is on; B or Esc closes a list, or goes from a page to the sidebar;
  the shoulder buttons go to the page before and after; the right stick scrolls a page. In the
  sidebar, up and down change the page and right goes into it (at its first control); the ring
  never jumps from a sidebar row to whatever in the page happens to be nearest. With a gamepad connected the ring
  is shown from the start; using the mouse hides it. A launcher nobody presses a key in looks as it
  did: its pages' pictures are the same bytes.
- A game file can be chosen with a gamepad: Choose, pressed with one, opens a list of folders and
  files in the launcher's own window (the system's file dialog wants a mouse or a keyboard).
- Choosing a button on the Controllers page with a gamepad: the button pressed is the button
  chosen and nothing more; while a key is waited for, any gamepad button leaves things as they were.
- **Muted**: the Game page has a Sound card with one switch, off unless the player switches it on.
  When it is on the game is started with `--mute` after the port's own arguments
  (`Description::mute_argument`; `nullptr` for no card). A game should take that argument and have
  a row in its menu that switches the sound off and on.
- **Menu**: a `MenuRow::Mute` row ("Sound": On / Muted) through `MenuConfig::muted` and `setMuted`
  (`RMENU_ROW_MUTE` for C). `kMenuBack` now leaves a page for the one it was reached from and closes
  the menu only on the first page (it closed the menu from anywhere). A host should give Space,
  Enter and a gamepad's A to Confirm, and Backspace and B to Back.
- `ui.h`: `focus_item`, `focus_next_item`, `show_focus`, `focus_shown`, `gamepad_in_use`, and
  `nav_item` takes `reachable`. A port's own pages get the ring from the widgets they already use.
- `--screenshot PREFIX --press KEYS` presses keys, and the buttons of a gamepad made for the test,
  then takes one picture: the launcher's navigation can be checked with no window and no gamepad.

## 0.3.0 (2026-10-13)

- **The launcher takes pages of a port's own** (`launcher.h`: `Description::pages`), shown in the
  sidebar before its own and drawn with the launcher's widgets. A page may take files dropped on
  the window and may say it is busy. `launcher/src/layout.h` now holds what pages are built from
  (`Card`, `row`, `inset_separator`, `heading`, `label`), for those pages to use.
- A port with no one game file can leave out the Game page (`game_page = false`); its own pages
  then start the game (`PageContext::play`).
- The Features page can be given another name and other first lines, and can be shown without the
  preset chooser (`features_name`, `features_intro`, `presets`). The sidebar's line under the
  game's name can be set (`subtitle`).
- When the game stops, the launcher reads `settings.ini` again, so a feature switched in the game's
  menu is not undone by the launcher's next save.
- A launcher that sets none of these is as it was: its pages' pictures are the same bytes.
- **Menu**: pages under pages, and a page's row wherever the host wants it. `MenuConfig::pages`
  names a page's parent (its row is then on the parent's page, after the parent's commands), what
  its row says, and whether the row counts the features that are on. `MenuRow::Page` is a row for
  one page on the first page; the `Pages` row leaves that page out. Left and Back go to the page a
  page was reached from. With no `pages` and no `Page` row the menu is as it was.

## 0.2.0 (2026-10-12)

- **Settings** (`settings.h`, and `settings_c.h` for C): the `[features]` rules a launcher and its
  game must agree on, in one place. A file editor that keeps every line it was not asked to change;
  the feature list as text (`--list-features`), now with a feature's choices as an optional sixth
  field (a reader of five fields still reads it, and a list of five fields still parses); reading
  every feature's value; changing one without rewriting the others; choosing a preset.
- **The launcher** (`launcher/`): the Game, Features and Controllers pages and Play, as one program
  for every port. A port supplies a description (`launcher.h`): the words for its game file, the
  check of it, the game's arguments, the Display card's switches, the actions whose keys and gamepad
  buttons can be chosen. Features that are a choice show as a dropdown. `--screenshot` shows no
  window and takes `--size WxH`. `launcher/example` builds it with a stand-in for the game.
- **Key and button bindings** (`launcher/src/bindings.h`, SDL 3): the `[keyboard1]` / `[gamepad1]`
  format, for the launcher and for a game's frontend.
- `tools/sync-common.sh` copies `launcher/` as well as `src/`. The `COMMIT` file is as before; a
  project's copy of the script must be replaced with this one.
- The launcher's look comes from another project and its build downloads three libraries:
  `THIRD_PARTY_NOTICES.md` credits them, and the audit knows which files are whose.

## 0.1.0 (2026-10-12)

The first public version.

- The in-game menu (`menu.h`): a first page in groups (save states, slot, speed, the host's own
  rows, quit, resume), a page per category of features and commands, on/off and choice features,
  a description and a note line, a hint line with the host's button names, scrolling about the
  chosen row where the rows do not fit, and room for a two-line hint on a narrow picture.
- A C interface (`menu_c.h`).
- Text (`text.h`): a 5x7 font, blended fills, wrapped text and its measurement.
- `tools/sync-common.sh`: a project's committed copy, the commit and hashes it records, `--check`.
- Tests, an example host with no game, the file audit, CI.
