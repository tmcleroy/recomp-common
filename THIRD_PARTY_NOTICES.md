# Third-party notices

recomp-common is licensed under the GNU General Public License v3.0 (see `LICENSE`).

## Code

There is none from other projects. Every source file was written for this project and the ports it
came out of; nothing is vendored, and nothing is downloaded at build time. The only dependencies are
the C and C++ standard libraries.

Code added later from another project must have a license compatible with the GPL-3.0 and be listed
here, with its notice kept in the file.

## The font

`src/text.h` contains a 5x7 dot-matrix font for ASCII 32 to 126 (475 bytes). It is the generic
dot-matrix character set that has been reproduced in LCD and LED display code since the 1980s,
the family that descends from character-generator ROMs. It was typed in for this project, not
copied from a file of any particular project; like every 5x7 table it shares many glyphs with the
others (a little over half of them are the same as in Adafruit GFX's `glcdfont.c`, for instance,
and the rest differ).

## Nothing from any game

No game's code, data, text, graphics or keys are in this repository, and none are needed to build,
test or use it.
