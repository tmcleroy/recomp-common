// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Input Lab contributors
#include "../launcher/src/window_layout.h"
#include <cassert>
#include <cstdio>
int main() {
    for (int x : {-2560,0,1920}) for (int width : {640,1280,1920,3840})
        for (int height : {480,720,1040,1400}) for (float scale : {1.f,1.25f,2.f}) {
            const SDL_Rect usable{x,-100,width,height};
            const auto bounds=launcher::InitialLauncherBounds(usable,scale);
            assert(bounds.w>0 && bounds.h>0 && bounds.x>usable.x && bounds.y>usable.y);
            assert(bounds.x+bounds.w<usable.x+usable.w && bounds.y+bounds.h<usable.y+usable.h);
            assert(bounds.w<=static_cast<int>(1440*scale));
        }
    std::puts("144 desktop geometry/DPI cases preserve usable bounds and margins");
}
