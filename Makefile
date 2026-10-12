# recomp-common has no build of its own: each project compiles its copy of src/. This tests it (make test),
# builds the example host (make example) and checks the files before a push (make audit).
CXX ?= c++
CC ?= cc
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic -Isrc
B := build

.PHONY: test example audit clean
test: $(B)/menu_test $(B)/c_test
	@$(B)/menu_test
	@$(B)/c_test

$(B)/menu_test: tests/menu_test.cpp src/menu.cpp src/menu.h src/text.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined tests/menu_test.cpp src/menu.cpp -o $@

# (the C interface, from a host compiled as C)
$(B)/c_test: tests/c_test.c src/menu.cpp src/menu_c.cpp src/menu_c.h src/menu.h src/text.h
	@mkdir -p $(B)
	$(CC) -std=c99 -Wall -Wextra -Wpedantic -Isrc -c tests/c_test.c -o $(B)/c_test.o
	$(CXX) $(CXXFLAGS) $(B)/c_test.o src/menu.cpp src/menu_c.cpp -o $@

# A host with no game: build/shot OUT.ppm [KEYS] [WIDTHxHEIGHT] (examples/shot.cpp)
example: $(B)/shot
$(B)/shot: examples/shot.cpp src/menu.cpp src/menu.h src/text.h
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) examples/shot.cpp src/menu.cpp -o $@

audit:
	@tools/audit.sh

clean:
	rm -rf $(B)
