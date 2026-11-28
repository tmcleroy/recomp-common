// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
// The launcher's notification chime (launcher/src/chime.cpp): its samples, with no sound device.
#include "../launcher/src/chime.h"

#include <cmath>
#include <cstdio>

static int failures = 0;
#define CHECK(x)                                                     \
    do {                                                             \
        if (!(x)) {                                                  \
            std::printf("%s:%d: failed: %s\n", __FILE__, __LINE__, #x); \
            failures++;                                              \
        }                                                            \
    } while (0)

int main() {
    const int rate = 44100;
    const std::vector<float> samples = launcher::chime_samples(rate, 0.35f);
    // about 0.6 seconds, no louder than asked (and about as loud: it is scaled to), starts and ends quiet
    CHECK(samples.size() > static_cast<size_t>(rate * 0.5) && samples.size() < static_cast<size_t>(rate * 0.8));
    float loudest = 0.0f;
    bool finite = true;
    for (float sample : samples) {
        loudest = std::fmax(loudest, std::fabs(sample));
        finite = finite && std::isfinite(sample);
    }
    CHECK(finite);
    CHECK(loudest <= 0.35f + 1e-4f && loudest > 0.3f);
    CHECK(std::fabs(samples.front()) < 0.01f && std::fabs(samples.back()) < 0.001f);
    // (the first 100 ms is mostly the first tone, 880 Hz: about 2 * 880 * 0.1 = 176 zero crossings, a few more
    // once the second tone comes in)
    int crossings = 0;
    for (size_t i = 1; i < static_cast<size_t>(rate * 0.1); i++) {
        crossings += (samples[i - 1] < 0) != (samples[i] < 0);
    }
    CHECK(crossings > 140 && crossings < 230);
    // a volume of nothing is silence
    for (float sample : launcher::chime_samples(rate, 0.0f)) {
        CHECK(sample == 0.0f);
    }
    if (failures == 0) {
        std::printf("chime_test: all as it should be\n");
    }
    return failures ? 1 : 0;
}
