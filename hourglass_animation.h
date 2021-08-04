// ESP8266 Wordclock
// Copyright (C) 2016 Thoralt Franz, https://github.com/thoralt
//
//  This file contains the hourglass animation. This is not a regular header file,
//  it must be included only once from ledfunctions.cpp.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#ifndef _HOURGLASS_ANIMATION_INC_
#define _HOURGLASS_ANIMATION_INC_

#include <stdint.h>
#include "config.h"

// animation frames for hourglass animation
// second dimension is NUM_PIXELS+2 to guarantee each frame starts at
// a 32 bit boundary
#define NUM_PIXELS_ALIGNED ((NUM_PIXELS + 3) & ~0x03)
static const uint8_t PROGMEM hourglass_animation[HOURGLASS_ANIMATION_FRAMES][NUM_PIXELS_ALIGNED] __attribute__ ((aligned (4))) = {
	{ 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 1, 2, 2, 0, 2, 2, 1, 0, 0,
		0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0,
		0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 1, 0, 2, 2, 2, 0, 1, 0, 0,
		0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		3, 3, 3, 3, 0, 0},
	{   0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 1, 2, 2, 0, 2, 2, 1, 0, 0,
		0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 1, 0, 2, 2, 2, 0, 1, 0, 0,
		0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		3, 3, 3, 3, 0, 0},
	{   0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 1, 2, 2, 0, 2, 2, 1, 0, 0,
		0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 2, 0, 1, 0, 0, 0,
		0, 0, 1, 0, 2, 2, 2, 0, 1, 0, 0,
		0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		3, 3, 3, 3, 0, 0},
	{   0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 1, 2, 2, 0, 2, 2, 1, 0, 0,
		0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 1, 0, 2, 2, 2, 0, 1, 0, 0,
		0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		3, 3, 3, 3, 0, 0},
	{   0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 1, 2, 2, 0, 2, 2, 1, 0, 0,
		0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 1, 0, 2, 2, 2, 0, 1, 0, 0,
		0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		3, 3, 3, 3, 0, 0},
	{   0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 1, 2, 2, 0, 2, 2, 1, 0, 0,
		0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 1, 0, 2, 2, 2, 0, 1, 0, 0,
		0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		3, 3, 3, 3, 0, 0},
	{   0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 1, 2, 2, 0, 2, 2, 1, 0, 0,
		0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 1, 0, 2, 2, 2, 0, 1, 0, 0,
		0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		3, 3, 3, 3, 0, 0},
	{   0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 1, 2, 2, 0, 2, 2, 1, 0, 0,
		0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 1, 0, 2, 2, 2, 0, 1, 0, 0,
		0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		3, 3, 3, 3, 0, 0}
};

#endif