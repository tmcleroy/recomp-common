// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Input Lab contributors
#include "../launcher/src/bindings.h"
#include <cassert>
#include <cstdio>
int main() {
    const recomp::BindAction actions[]={{"up","Up",{SDL_SCANCODE_W},{}},{"down","Down",{SDL_SCANCODE_S},{}}};
    const recomp::BindTable table{actions,2,2};
    auto b=recomp::DefaultBindings(table);
    b.key[0][0]=SDL_SCANCODE_W; b.key[1][0]=SDL_SCANCODE_A; b.key[1][1]=SDL_SCANCODE_S;
    recomp::BindKeyboardKey(b,table,1,0,SDL_SCANCODE_W,true);
    assert(b.key[0][0]==SDL_SCANCODE_W && b.key[1][0]==SDL_SCANCODE_W);
    recomp::BindKeyboardKey(b,table,1,1,SDL_SCANCODE_W,true);
    assert(b.key[1][0]==SDL_SCANCODE_S && b.key[0][0]==SDL_SCANCODE_W);
    recomp::BindKeyboardKey(b,table,1,0,SDL_SCANCODE_W,false);
    assert(b.key[0][0]==SDL_SCANCODE_UNKNOWN);
    std::puts("Player-scoped key swaps preserve other layouts; legacy cross-player clearing remains opt-out");
}
