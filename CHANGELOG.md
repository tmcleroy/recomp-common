# Changelog

Versions follow [semantic versioning](https://semver.org/). Before 1.0 the interface may change in
a minor version. A port's copy only changes when it runs `tools/sync-common.sh`.

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
