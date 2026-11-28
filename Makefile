# recomp-common has no build of its own: each project compiles its copy of src/ and launcher/. This tests it
# (make test), builds the example host (make example) and the example launcher (make launcher), and checks the
# files before a push (make audit).
CXX ?= c++
CC ?= cc
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic -Isrc
B := build

.PHONY: test example launcher launcher-shots audit clean
test: $(B)/menu_test $(B)/i18n_test $(B)/c_test $(B)/settings_test $(B)/settings_c_test $(B)/folders_test $(B)/proxy_test $(B)/update_test $(B)/online_test $(B)/layout_test $(B)/curl_args_test $(B)/video_keys_test $(B)/menu_buttons_test $(B)/menu_buttons_port_test $(B)/own_shaders_test $(B)/own_shaders_test20 $(B)/menu_turn_test
	@$(B)/menu_test
	@$(B)/i18n_test
	@$(B)/c_test
	@$(B)/settings_test
	@cd $(B) && ./settings_c_test > /dev/null && echo "settings_c_test: all as it should be"
	@$(B)/folders_test
	@$(B)/proxy_test
	@$(B)/update_test
	@$(B)/online_test
	@$(B)/layout_test
	@$(B)/curl_args_test
	@$(B)/menu_buttons_test
	@$(B)/menu_buttons_port_test
	@$(B)/video_keys_test
	@$(B)/own_shaders_test
	@$(B)/own_shaders_test20
	@$(B)/menu_turn_test
	@if pkg-config --exists sdl3 2> /dev/null; then $(MAKE) -s $(B)/menu_sdl_test && $(B)/menu_sdl_test; else echo "menu_sdl_test: skipped (no SDL 3)"; fi
	@if pkg-config --exists sdl3 2> /dev/null; then $(MAKE) -s $(B)/arcade_sdl_test && $(B)/arcade_sdl_test; else echo "arcade_sdl_test: skipped (no SDL 3)"; fi
	@if pkg-config --exists sdl3 2> /dev/null; then $(MAKE) -s $(B)/pad_players_test && $(B)/pad_players_test; else echo "pad_players_test: skipped (no SDL 3)"; fi
	@if pkg-config --exists sdl3 2> /dev/null; then $(MAKE) -s $(B)/chime_test && $(B)/chime_test; else echo "chime_test: skipped (no SDL 3)"; fi

# Which gamepad is which player's (launcher/src/bindings.h; needs SDL 3, no gamepad)
$(B)/pad_players_test: tests/pad_players_test.cpp launcher/src/bindings.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined $$(pkg-config --cflags sdl3) tests/pad_players_test.cpp $$(pkg-config --libs sdl3) -o $@
# The Display switches a game changed while the launcher ran (launcher/src/video_keys.h)
$(B)/video_keys_test: tests/video_keys_test.cpp launcher/src/video_keys.h launcher/src/launcher.h src/settings.cpp
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -Ilauncher/src $$(pkg-config --cflags sdl3) -fsanitize=address,undefined tests/video_keys_test.cpp src/settings.cpp -o $@
# The launcher's notification chime (launcher/src/chime.cpp; needs SDL 3, no sound device)
$(B)/chime_test: tests/chime_test.cpp launcher/src/chime.cpp launcher/src/chime.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined $$(pkg-config --cflags sdl3) tests/chime_test.cpp launcher/src/chime.cpp $$(pkg-config --libs sdl3) -o $@
# The arcade monitor's choice of mode and the rest of its page (arcade_sdl.h, arcade_screen.h; needs SDL 3, no screen)
$(B)/arcade_sdl_test: tests/arcade_sdl_test.cpp src/arcade_sdl.h src/arcade_screen.h src/arcade_screen.cpp
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) $$(pkg-config --cflags sdl3) tests/arcade_sdl_test.cpp src/arcade_screen.cpp $$(pkg-config --libs sdl3) -o $@

$(B)/menu_test: tests/menu_test.cpp src/menu.cpp src/menu.h src/text.h src/i18n.h src/i18n_pt_br.h src/i18n_es.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined tests/menu_test.cpp src/menu.cpp -o $@

# curl's arguments, as Windows' own requests read them (launcher/src/winhttp_curl.h)
$(B)/curl_args_test: tests/curl_args_test.cpp launcher/src/winhttp_curl.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined tests/curl_args_test.cpp -o $@

# The standard layout (menu_layout.h)
$(B)/layout_test: tests/layout_test.cpp src/menu_layout.h src/menu.cpp src/menu.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined tests/layout_test.cpp src/menu.cpp -o $@

# The menu's gamepad buttons as the player chose them (menu_buttons.h), with recomp-common's own choice and with a
# port's (RECOMP_MENU_BUTTON_DEFAULT)
$(B)/menu_buttons_test: tests/menu_buttons_test.cpp src/menu_buttons.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined tests/menu_buttons_test.cpp -o $@
$(B)/menu_buttons_port_test: tests/menu_buttons_test.cpp src/menu_buttons.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined '-DRECOMP_MENU_BUTTON_DEFAULT="guide"' -DEXPECTED_DEFAULT=MenuButtons::Guide \
	  tests/menu_buttons_test.cpp -o $@

# The menu turned with the picture (menu_turn.h)
$(B)/menu_turn_test: tests/menu_turn_test.cpp src/menu_turn.h src/menu.cpp src/menu.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined tests/menu_turn_test.cpp src/menu.cpp -o $@

# The menu's SDL controls (menu_sdl.h; needs SDL 3)
$(B)/menu_sdl_test: tests/menu_sdl_test.cpp src/menu_sdl.h src/menu_turn.h src/menu_buttons.h src/menu.cpp src/menu.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) $$(pkg-config --cflags sdl3) tests/menu_sdl_test.cpp src/menu.cpp $$(pkg-config --libs sdl3) -o $@

# The words in the player's language, and the font's accented letters
$(B)/i18n_test: tests/i18n_test.cpp src/i18n.h src/i18n_pt_br.h src/i18n_es.h src/text.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined tests/i18n_test.cpp -o $@

# (the C interface, from a host compiled as C)
$(B)/c_test: tests/c_test.c src/menu.cpp src/menu_c.cpp src/menu_c.h src/menu.h src/text.h src/i18n.h
	@mkdir -p $(B)
	$(CC) -std=c99 -Wall -Wextra -Wpedantic -Isrc -c tests/c_test.c -o $(B)/c_test.o
	$(CXX) $(CXXFLAGS) $(B)/c_test.o src/menu.cpp src/menu_c.cpp -o $@

# The settings rules (settings.h), and from C
$(B)/settings_test: tests/settings_test.cpp src/settings.cpp src/settings.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined tests/settings_test.cpp src/settings.cpp -o $@

$(B)/settings_c_test: tests/settings_c_test.c src/settings.cpp src/settings_c.cpp src/settings_c.h src/settings.h
	@mkdir -p $(B)
	$(CC) -std=c99 -Wall -Wextra -Wpedantic -Isrc -c tests/settings_c_test.c -o $(B)/settings_c_test.o
	$(CXX) $(CXXFLAGS) $(B)/settings_c_test.o src/settings.cpp src/settings_c.cpp -o $@

# The proxy Windows is set to use, for curl (proxy.h)
$(B)/proxy_test: tests/proxy_test.cpp src/proxy.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined tests/proxy_test.cpp -o $@

# Where the player's things are, and portable mode (folders.h)
$(B)/folders_test: tests/folders_test.cpp src/folders.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined tests/folders_test.cpp -o $@

# The player's own shader presets (own_shaders.h)
$(B)/own_shaders_test: tests/own_shaders_test.cpp src/own_shaders.h src/folders.h src/settings.cpp src/settings.h src/i18n.h src/i18n_es.h src/i18n_pt_br.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined tests/own_shaders_test.cpp src/settings.cpp -o $@
# (and as a port of C++20 has it, where a path's UTF-8 is of char8_t)
$(B)/own_shaders_test20: tests/own_shaders_test.cpp src/own_shaders.h src/folders.h src/settings.cpp src/settings.h src/i18n.h src/i18n_es.h src/i18n_pt_br.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -std=c++20 tests/own_shaders_test.cpp src/settings.cpp -o $@

# The launcher's updates (launcher/src/update.h; needs curl and tar, or ditto, to run)
$(B)/update_test: tests/update_test.cpp launcher/src/update.cpp launcher/src/update.h launcher/src/run_program.cpp launcher/src/run_program.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -Ilauncher/src -fsanitize=address,undefined tests/update_test.cpp launcher/src/update.cpp launcher/src/run_program.cpp -o $@

# A host with no game: build/shot OUT.ppm [KEYS] [WIDTHxHEIGHT] (examples/shot.cpp)
example: $(B)/shot
$(B)/shot: examples/shot.cpp src/menu.cpp src/menu.h src/text.h src/i18n.h src/i18n_pt_br.h src/i18n_es.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) examples/shot.cpp src/menu.cpp -o $@

# The launcher with a stand-in for the game (launcher/example; needs cmake, and fetches Dear ImGui, the Inter
# fonts and, where the system has none, SDL 3)
launcher:
	@cmake -S launcher/example -B $(B)/launcher > /dev/null
	@cmake --build $(B)/launcher
# Its pages as pictures, $(B)/launcher-shots/page0.png.., with no window and no display needed
launcher-shots: launcher
	@mkdir -p $(B)/launcher-shots/home
	@SDL_VIDEO_DRIVER=dummy SDL_RENDER_DRIVER=software EXAMPLE_HOME=$(B)/launcher-shots/home \
	  $(B)/launcher/example-launcher --screenshot $(B)/launcher-shots/page
	@ls $(B)/launcher-shots/page0.png $(B)/launcher-shots/page1.png $(B)/launcher-shots/page2.png

audit:
	@tools/audit.sh

clean:
	rm -rf $(B)

# The lobby's list as the launcher reads it (launcher/src/online.cpp): Join, or Watch for a running match
$(B)/online_test: tests/online_test.cpp launcher/src/online.cpp launcher/src/online.h launcher/src/run_program.cpp
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -Ilauncher/src -fsanitize=address,undefined tests/online_test.cpp launcher/src/run_program.cpp -o $@
