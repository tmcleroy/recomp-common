// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
// The settings rules from a host written in C (settings_c.h).
#include "settings_c.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(what) \
  do { \
    if(!(what)) { \
      fprintf(stderr, "%s:%d: not so: %s\n", __FILE__, __LINE__, #what); \
      exit(1); \
    } \
  } while(0)

int main(void) {
  static const char* const kinds[] = {"Off", "Some", "All", NULL};
  static const RFeature features[] = {
    {"wide", "Picture", "Wide", "A wider picture.", RFEATURE_ENHANCED, NULL},
    {"lives", "Cheats", "Lives", "How many are kept.", 0, kinds},
    {"easy", "Difficulty", "Easy", "", RFEATURE_EXCLUSIVE, NULL},
    {"hard", "Difficulty", "Hard", "", RFEATURE_EXCLUSIVE, NULL},
  };
  const char* path = "recomp_common_settings_c_test.ini";
  int values[4];

  remove(path);
  rsettings_feature_values(path, features, 4, values);
  CHECK(!values[0] && !values[1] && !values[2] && !values[3]);

  FILE* f = fopen(path, "w");
  CHECK(f);
  fputs("[sound]\nvolume = 7\n[features]\npreset = enhanced\nlives = 2\nhard = 1\n", f);
  fclose(f);
  rsettings_feature_values(path, features, 4, values);
  CHECK(values[0] == 1 && values[1] == 2 && values[2] == 0 && values[3] == 1);

  CHECK(rsettings_set_feature(path, features, 4, 2, 1));
  rsettings_feature_values(path, features, 4, values);
  CHECK(values[0] == 1 && values[1] == 2 && values[2] == 1 && values[3] == 0);
  char text[32];
  CHECK(rsettings_get(path, "sound", "volume", text, sizeof text) && !strcmp(text, "7"));
  CHECK(rsettings_get(path, "features", "preset", text, sizeof text) && !strcmp(text, "custom"));
  CHECK(!rsettings_get(path, "sound", "nothing", text, sizeof text) && !text[0]);

  rsettings_list_features(features, 4, stdout);
  remove(path);
  puts("settings_c_test: all as it should be");
  return 0;
}
