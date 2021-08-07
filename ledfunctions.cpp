// ESP8266 Wordclock
// Copyright (C) 2016 Thoralt Franz, https://github.com/thoralt
// also (C) 2021 by Stefan Rinke, https://github.com/sker65
//
//  This module implements functions to manage the WS2812B LEDs. Two buffers contain
//  color information with current state and fade target state and are updated by
//  either simple set operations or integrated screensavers (matrix, stars, heart).
//  Also contains part of the data and logic for the hourglass animation.
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
#include "ledfunctions.h"
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// global instance
//---------------------------------------------------------------------------------------
LEDMatrix LED = LEDMatrix();

uint8_t fireBuf[NUM_PIXELS] __attribute__( ( aligned( 4 ) ) );
uint8_t plasmaBuf[NUM_PIXELS] __attribute__( ( aligned( 4 ) ) );

//---------------------------------------------------------------------------------------
// variables in PROGMEM (mapping table, images)
//---------------------------------------------------------------------------------------
#include "hourglass_animation.h"

// This defines the LED output for different minutes
// param0 controls whether the hour has to be incremented for the given minutes
// param1 is the matching minimum minute count (inclusive)
// param2 is the matching maximum minute count (inclusive)
// clang-format off
const std::vector<leds_template_t> LEDMatrix::minutesTemplate[3][2] =
{
	{
		// layout 1
		{
			{ 0,  0,  4,{ 106, 107, 108 } },                                  // UHR
			{ 0,  5,  9,{ 7, 8, 9, 10, 34, 35, 36, 37 } },                    // FüNF NACH
			{ 0, 10, 14,{ 11, 12, 13, 14, 34, 35, 36, 37 } },                 // ZEHN NACH
			{ 0, 15, 19,{ 15, 16, 17, 18, 19, 20, 21, 34, 35, 36, 37 } },     // VIERTEL NACH
			{ 0, 20, 24,{ 22, 23, 24, 25, 26, 27, 28, 34, 35, 36, 37 } },     // ZWANZIG NACH
			{ 1, 25, 29,{ 7, 8, 9, 10, 30, 31, 32, 39, 40, 41, 42 } },        // FüNF VOR HALB
			{ 1, 30, 34,{ 39, 40, 41, 42 } },                                 // HALB
			{ 1, 35, 39,{ 7, 8, 9, 10, 34, 35, 36, 37, 39, 40, 41, 42 } },    // FüNF NACH HALB
			{ 1, 40, 44,{ 22, 23, 24, 25, 26, 27, 28, 30, 31, 32 } },         // ZWANZIG VOR
			{ 1, 45, 49,{ 15, 16, 17, 18, 19, 20, 21, 30, 31, 32 } },         // VIERTEL VOR
			{ 1, 50, 54,{ 11, 12, 13, 14, 30, 31, 32 } },                     // ZEHN VOR
			{ 1, 55, 59,{ 7, 8, 9, 10, 30, 31, 32 } }                         // FüNF VOR
		},
		{
			{ 0,  0,  4,{ 106, 107, 108 } },                                  // UHR
			{ 0,  5,  9,{ 7, 8, 9, 10, 34, 35, 36, 37 } },                    // FüNF NACH
			{ 0, 10, 14,{ 11, 12, 13, 14, 34, 35, 36, 37 } },                 // ZEHN NACH
			{ 0, 15, 19,{ 15, 16, 17, 18, 19, 20, 21, 34, 35, 36, 37 } },     // VIERTEL NACH
			{ 0, 20, 24,{ 22, 23, 24, 25, 26, 27, 28, 34, 35, 36, 37 } },     // ZWANZIG NACH
			{ 1, 25, 29,{ 7, 8, 9, 10, 30, 31, 32, 39, 40, 41, 42 } },        // FüNF VOR HALB
			{ 1, 30, 34,{ 39, 40, 41, 42 } },                                 // HALB
			{ 1, 35, 39,{ 7, 8, 9, 10, 34, 35, 36, 37, 39, 40, 41, 42 } },    // FüNF NACH HALB
			{ 1, 40, 44,{ 22, 23, 24, 25, 26, 27, 28, 30, 31, 32 } },         // ZWANZIG VOR
			{ 1, 45, 49,{ 15, 16, 17, 18, 19, 20, 21, 30, 31, 32 } },         // VIERTEL VOR
			{ 1, 50, 54,{ 11, 12, 13, 14, 30, 31, 32 } },                     // ZEHN VOR
			{ 1, 55, 59,{ 7, 8, 9, 10, 30, 31, 32 } }                         // FüNF VOR
		}
	},
	// layout 2
	{
		{
			{ 0,  0,  4,{ 107, 108, 109 } },                             // UHR
			{ 0,  5,  9,{ 7,8,9,10, 40,41,42,43 } },                     // FüNF NACH
			{ 0, 10, 14,{ 11, 12, 13, 14, 40,41,42,43 } },               // ZEHN NACH
			{ 0, 15, 19,{ 26,27,28,29,30,31,32, 40,41,42,43 } },         // VIERTEL NACH
			{ 0, 20, 24,{ 15,16,17,18,19,20,21, 40,41,42,43 } },         // ZWANZIG NACH
			{ 1, 25, 29,{ 7,8,9,10, 33,34,35, 44,45,46,47 } },           // FüNF VOR HALB
			{ 1, 30, 34,{ 44,45,46,47 } },                               // HALB
			{ 1, 35, 39,{ 7,8,9,10, 40,41,42,43, 44,45,46,47 } },        // FüNF NACH HALB
			{ 1, 40, 44,{ 15,16,17,18,19,20,21, 33,34,35 } },            // ZWANZIG VOR
			{ 1, 45, 49,{ 26,27,28,29,30,31,32, 33,34,35 } },            // VIERTEL VOR
			{ 1, 50, 54,{ 11,12,13,14, 33,34,35 } },                     // ZEHN VOR
			{ 1, 55, 59,{ 7,8,9,10, 33,34,35 } }                         // FüNF VOR
		},
		{
			{ 0,  0,  4,{ 107, 108, 109 } },                             // UHR
			{ 0,  5,  9,{ 7,8,9,10, 40,41,42,43 } },                     // FüNF NACH
			{ 0, 10, 14,{ 11,12,13,14, 40,41,42,43 } },                  // ZEHN NACH
			{ 1, 15, 19,{ 26,27,28,29,30,31,32 } },         						 // VIERTEL 
			{ 1, 20, 24,{ 11,12,13,14, 33,34,35, 44,45,46,47 } },        // ZEHN VOR HALB
			{ 1, 25, 29,{ 7,8,9,10, 33,34,35, 44,45,46,47 } },           // FüNF VOR HALB
			{ 1, 30, 34,{ 44,45,46,47 } },                               // HALB
			{ 1, 35, 39,{ 7,8,9,10, 40,41,42,43, 44,45,46,47 } },        // FüNF NACH HALB
			{ 1, 40, 44,{ 11,12,13,14, 40,41,42,43, 44,45,46,47 } },  // ZEHN NACH HALB
			{ 1, 45, 49,{ 22,23,24,25, 26,27,28,29,30,31,32 } },            // DREI VIERTEL
			{ 1, 50, 54,{ 11,12,13,14, 33,34,35 } },                     // ZEHN VOR
			{ 1, 55, 59,{ 7,8,9,10, 33,34,35 } }                         // FüNF VOR
		}
	},
	{
// words for layout 3
#define L3_UHR 107,108,109
#define L3_FUNF 7,8,9,10
#define L3_ZEHN 11,12,13,14
#define L3_NACH 35,36,37,38
#define L3_VOR 39,40,41
#define L3_HALB 44,45,46,47
#define L3_ZWANZIG 15,16,17,18,19,20,21
#define L3_DREI 22,23,24
#define L3_VIERTEL 26,27,28,29,30,31,32
		{
			{ 0,  0,  4,{ L3_UHR } },
			{ 0,  5,  9,{ L3_FUNF, L3_NACH } },
			{ 0, 10, 14,{ L3_ZEHN, L3_NACH } },
			{ 0, 15, 19,{ L3_VIERTEL, L3_NACH } },
			{ 0, 20, 24,{ L3_ZWANZIG, L3_NACH } },
			{ 1, 25, 29,{ L3_FUNF, L3_VOR, L3_HALB } },
			{ 1, 30, 34,{ L3_HALB } },
			{ 1, 35, 39,{ L3_FUNF, L3_NACH, L3_HALB } },
			{ 1, 40, 44,{ L3_ZWANZIG, L3_VOR } },
			{ 1, 45, 49,{ L3_VIERTEL, L3_VOR } },
			{ 1, 50, 54,{ L3_ZEHN, L3_VOR } },
			{ 1, 55, 59,{ L3_FUNF, L3_VOR } }
		},
		{
			{ 0,  0,  4,{ L3_UHR } },
			{ 0,  5,  9,{ L3_FUNF, L3_NACH } },
			{ 0, 10, 14,{ L3_ZEHN, L3_NACH } },
			{ 1, 15, 19,{ L3_VIERTEL } },
			{ 1, 20, 24,{ L3_ZEHN, L3_VOR, L3_HALB } },
			{ 1, 25, 29,{ L3_FUNF, L3_VOR, L3_HALB } }, 
			{ 1, 30, 34,{ L3_HALB } },
			{ 1, 35, 39,{ L3_FUNF, L3_NACH, L3_HALB } },
			{ 1, 40, 44,{ L3_ZEHN, L3_NACH, L3_HALB } }, 
			{ 1, 45, 49,{ L3_DREI,L3_VIERTEL } },
			{ 1, 50, 54,{ L3_ZEHN, L3_VOR } },
			{ 1, 55, 59,{ L3_FUNF, L3_VOR } }
		}
	}
};

// This defines the LED output for different hours
// param0 deals with special cases:
//     = 0: matches hour in param1 and param2
//     = 1: matches hour in param1 and param2 whenever minute is < 5
//     = 2: matches hour in param1 and param2 whenever minute is >= 5
// param1: hour to match
// param2: alternative hour to match
const std::vector<leds_template_t> LEDMatrix::hoursTemplate[3] =
{
	{
		{ 0,  0, 12,{ 100, 101, 102, 103, 104 } }, // ZWÖLF
		{ 1,  1, 13,{ 44, 45, 46 } },             // EIN
		{ 2,  1, 13,{ 44, 45, 46, 47 } },         // EINS
		{ 0,  2, 14,{ 49, 50, 51, 52 } },         // ZWEI
		{ 0,  3, 15,{ 56, 57, 58, 59 } },         // DREI
		{ 0,  4, 16,{ 61, 62, 63, 64 } },         // VIER
		{ 0,  5, 17,{ 66, 67, 68, 69 } },         // FÜNF
		{ 0,  6, 18,{ 71, 72, 73, 74, 75 } },     // SECHS
		{ 0,  7, 19,{ 78, 79, 80, 81, 82, 83 } }, // SIEBEN
		{ 0,  8, 20,{ 84, 85, 86, 87 } },         // ACHT
		{ 0,  9, 21,{ 88, 89, 90, 91 } },         // NEUN
		{ 0, 10, 22,{ 92, 93, 94, 95 } },         // ZEHN
		{ 0, 11, 23,{ 96, 97, 98 } },             // ELF
	},
	{
		{ 0,  0, 12,{ 94,95,96,97,98 } },      // ZWÖLF
		{ 1,  1, 13,{ 55,56,57 } },            // EIN
		{ 2,  1, 13,{ 55,56,57,58 } },         // EINS
		{ 0,  2, 14,{ 62,63,64,65 } },         // ZWEI
		{ 0,  3, 15,{ 66,67,68,69 } },         // DREI
		{ 0,  4, 16,{ 73,74,75,76 } },         // VIER
		{ 0,  5, 17,{ 51,52,53,54 } },         // FÜNF
		{ 0,  6, 18,{ 77,78,79,80,81 } },      // SECHS
		{ 0,  7, 19,{ 88,89,90,91,92,93 } },   // SIEBEN
		{ 0,  8, 20,{ 84, 85, 86, 87 } },      // ACHT
		{ 0,  9, 21,{ 102,103,104,105 } },     // NEUN
		{ 0, 10, 22,{ 99,100,101,102 } },      // ZEHN
		{ 0, 11, 23,{ 49,50,51 } },            // ELF
	},
	{
		{ 0,  0, 12,{ 49,50,51,52,53 } },      // ZWÖLF
		{ 1,  1, 13,{ 57,58,59 } },            // EIN
		{ 2,  1, 13,{ 57,58,59,60 } },         // EINS
		{ 0,  2, 14,{ 55,56,57,58 } },         // ZWEI
		{ 0,  3, 15,{ 67,68,69,70 } },         // DREI
		{ 0,  4, 16,{ 84,85,86,87 } },         // VIER
		{ 0,  5, 17,{ 73,74,75,76 } },         // FÜNF
		{ 0,  6, 18,{ 100,101,102,103,104 } }, // SECHS
		{ 0,  7, 19,{ 60,61,62,63,64,65 } },   // SIEBEN
		{ 0,  8, 20,{ 89,90,91,92 } },         // ACHT
		{ 0,  9, 21,{ 80,81,82,83 } },         // NEUN
		{ 0, 10, 22,{ 93,94,95,96 } },         // ZEHN
		{ 0, 11, 23,{ 77,78,79 } },            // ELF
	} 
};



// this mapping table maps the linear memory buffer structure used throughout the
// project to the physical layout of the LEDs
const uint32_t PROGMEM __attribute__ ((aligned (4))) LEDMatrix::mapping[NUM_PIXELS]  = {
	0  , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10  ,
	21  , 20  , 19  , 18  , 17  , 16  , 15  , 14  , 13  , 12  , 11  ,
	22  , 23  , 24  , 25  , 26  , 27  , 28  , 29  , 30  , 31  , 32  ,
	43  , 42  , 41  , 40  , 39  , 38  , 37  , 36  , 35  , 34  , 33  ,
	44  , 45  , 46  , 47  , 48  , 49  , 50  , 51  , 52  , 53  , 54  ,
	65  , 64  , 63  , 62  , 61  , 60  , 59  , 58  , 57  , 56  , 55  ,
	66  , 67  , 68  , 69  , 70  , 71  , 72  , 73  , 74  , 75  , 76  ,
	87  , 86  , 85  , 84  , 83  , 82  , 81  , 80  , 79  , 78  , 77  ,
	88  , 89  , 90  , 91  , 92  , 93  , 94  , 95  , 96  , 97  , 98  ,
	109 , 108 , 107 , 106 , 105 , 104 , 103 , 102 , 101 , 100 , 99 ,
	110, 111, 112, 113
};


const uint32_t PROGMEM  __attribute__ ((aligned (4))) LEDMatrix::brightnessCurveSelect[NUM_PIXELS]  = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0
	/*		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1,
	0, 1, 0, 1*/
};

const uint32_t PROGMEM LEDMatrix::brightnessCurvesR[256 * NUM_BRIGHTNESS_CURVES] __attribute__ ((aligned (4))) = {
	// LED type 1, 1:1 mapping (neutral)
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
	37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
	54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
	71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87,
	88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
	117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
	130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142,
	143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155,
	156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168,
	169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
	182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
	195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
	208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220,
	221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233,
	234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246,
	247, 248, 249, 250, 251, 252, 253, 254, 255,

	// LED type 2
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7,
	7, 7, 9, 9, 9, 9, 11, 11, 11, 11, 13, 13, 13, 13, 15, 15, 15, 15,
	17, 17, 17, 17, 19, 19, 21, 21, 23, 23, 25, 25, 27, 27, 29, 29, 31,
	31, 33, 33, 35, 35, 37, 37, 39, 39, 41, 41, 43, 43, 45, 45, 47, 47,
	49, 49, 50, 50, 53, 53, 53, 53, 55, 55, 57, 57, 59, 59, 61, 61, 63,
	63, 65, 65, 67, 67, 69, 69, 71, 71, 73, 73, 75, 75, 77, 77, 79, 79,
	81, 81, 83, 83, 83, 83, 85, 85, 87, 87, 89, 89, 91, 91, 93, 93, 95,
	95, 97, 97, 99, 99, 101, 101, 103, 103, 105, 105, 107, 107, 109, 109,
	111, 111, 113, 113, 115, 115, 115, 115, 117, 117, 119, 119, 121, 121,
	123, 123, 125, 125, 127, 127, 130, 130, 132, 132, 134, 134, 136, 136,
	138, 138, 140, 140, 142, 142, 144, 144, 144, 144, 146, 146, 148, 148,
	150, 150, 152, 152, 153, 153, 155, 155, 156, 156, 158, 158, 160, 160,
	162, 162, 164, 164, 166, 166, 168, 168, 170, 170, 172, 172, 174, 174,
	176, 176, 178, 178, 180, 180, 182, 182, 184, 184, 184, 184, 186, 186,
	188, 188, 190, 190, 192, 192, 194, 194, 196, 196, 198, 198, 200, 200,
	202, 202, 204, 204, 206, 206, 208, 208, 210, 210, 212, 212, 214, 214,
	216, 216, 218, 218
};

const uint32_t PROGMEM LEDMatrix::brightnessCurvesG[256 * NUM_BRIGHTNESS_CURVES] __attribute__ ((aligned (4))) = {
	// LED type 1, 1:1 mapping (neutral)
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
	37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
	54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
	71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87,
	88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
	117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
	130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142,
	143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155,
	156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168,
	169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
	182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
	195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
	208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220,
	221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233,
	234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246,
	247, 248, 249, 250, 251, 252, 253, 254, 255,

	// LED type 2
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7, 7,
	7, 9, 9, 9, 9, 11, 11, 11, 11, 13, 13, 13, 13, 15, 15, 15, 15, 17,
	17, 17, 17, 19, 19, 21, 21, 23, 23, 25, 25, 27, 27, 29, 29, 31, 31,
	33, 33, 35, 35, 37, 37, 39, 39, 41, 41, 43, 43, 45, 45, 47, 47, 49,
	49, 51, 51, 53, 53, 54, 54, 56, 56, 57, 57, 59, 59, 61, 61, 63, 63,
	66, 66, 68, 68, 70, 70, 72, 72, 74, 74, 76, 76, 78, 78, 80, 80, 80,
	80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80,
	80, 80, 80, 80, 80, 80, 80, 80, 108, 108, 110, 110, 112, 112, 113,
	113, 116, 116, 117, 117, 119, 119, 122, 122, 124, 124, 126, 126, 127,
	127, 131, 131, 132, 132, 135, 135, 137, 137, 139, 139, 141, 141, 143,
	143, 145, 145, 147, 147, 149, 149, 151, 151, 153, 153, 155, 155, 157,
	157, 159, 159, 160, 160, 163, 163, 165, 165, 167, 167, 169, 169, 171,
	171, 173, 173, 175, 175, 177, 177, 179, 179, 181, 181, 183, 183, 185,
	185, 187, 187, 189, 189, 191, 191, 194, 194, 197, 197, 198, 198, 201,
	201, 203, 203, 205, 205, 208, 208, 210, 210, 212, 212, 215, 215, 218,
	218, 220, 220, 222, 222, 224, 224, 226, 226, 228, 228, 230, 230, 232,
	232, 234, 234
};

const uint32_t PROGMEM LEDMatrix::brightnessCurvesB[256 * NUM_BRIGHTNESS_CURVES] __attribute__ ((aligned (4))) = {
	// LED type 1, 1:1 mapping (neutral)
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
	37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
	54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
	71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87,
	88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
	117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
	130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142,
	143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155,
	156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168,
	169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
	182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
	195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
	208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220,
	221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233,
	234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246,
	247, 248, 249, 250, 251, 252, 253, 254, 255,

	// LED type 2
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7, 9,
	9, 9, 9, 9, 9, 11, 11, 11, 11, 13, 13, 13, 13, 15, 15, 15, 15, 17,
	17, 19, 19, 20, 20, 21, 21, 23, 23, 25, 25, 27, 27, 29, 29, 31, 31,
	33, 33, 35, 35, 37, 37, 39, 39, 41, 41, 42, 42, 43, 43, 45, 45, 47,
	47, 49, 49, 51, 51, 53, 53, 55, 55, 57, 57, 59, 59, 61, 61, 63, 63,
	64, 64, 66, 66, 68, 68, 72, 72, 72, 72, 74, 74, 76, 76, 78, 78, 80,
	80, 82, 82, 84, 84, 86, 86, 88, 88, 90, 90, 91, 91, 93, 93, 94, 94,
	96, 96, 98, 98, 100, 100, 102, 102, 104, 104, 106, 106, 108, 108,
	110, 110, 112, 112, 114, 114, 116, 116, 118, 118, 119, 119, 121, 121,
	123, 123, 124, 124, 127, 127, 129, 129, 131, 131, 133, 133, 135, 137,
	137, 139, 139, 141, 141, 143, 143, 145, 145, 147, 147, 149, 149, 151,
	151, 153, 153, 155, 155, 157, 157, 160, 160, 161, 161, 163, 163, 166,
	166, 168, 168, 170, 170, 171, 171, 174, 174, 176, 176, 178, 178, 180,
	180, 183, 183, 184, 184, 187, 187, 188, 188, 191, 191, 194, 194, 196,
	196, 198, 198, 200, 200, 202, 202, 204, 204, 206, 206, 209, 209, 210,
	210, 212, 212, 215, 215, 217, 217, 219, 219, 222, 222, 224, 224, 226,
	226, 228, 228, 230
};
// clang-format on

//---------------------------------------------------------------------------------------
// getters, setters, data flow
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// ~LEDMatrix
//
// Destructor
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
LEDMatrix::~LEDMatrix() {
	//	for(MatrixObject* m : this->matrix_objects) delete m;
}

//---------------------------------------------------------------------------------------
// LEDMatrix
//
// Constructor, initializes data structures
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
LEDMatrix::LEDMatrix() {
	// initialize matrix objects with random coordinates
	for( int i = 0; i < NUM_MATRIX_OBJECTS; i++ ) {
		this->matrix.push_back( MatrixObject() );
	}

	// initialize star objects with default coordinates
	for( int i = 0; i < NUM_STARS; i++ )
		this->stars.push_back( StarObject() );

	// set random coordinates with minimum distance to other star objects
	for( StarObject& s : this->stars )
		s.randomize( this->stars );
}

//---------------------------------------------------------------------------------------
// begin
//
// Initializes the LED driver
//
// -> pin: hardware pin to use for WS2812B data output
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::begin( int pin ) {
	// this->pixels = new Adafruit_NeoPixel(NUM_PIXELS, pin, NEO_GRB + NEO_KHZ800);
	this->strip = new NeoPixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod>( NUM_PIXELS );
	this->strip->Begin();
}
const DisplayMode LEDMatrix::randomModes[] = { DisplayMode::fade, DisplayMode::flyingLettersVerticalUp,
                                               DisplayMode::flyingLettersVerticalDown, DisplayMode::explode,
                                               DisplayMode::snake };
#define NUM_RANDOM_MODES 5

void LEDMatrix::resetRainbowColor() { this->currentRainbowColor = Config.fg; }

void LEDMatrix::preparePalette( palette_entry* palette ) {
	// load palette colors from configuration
	palette[0] = Config.bg;
	palette[1] = Config.fg;
	palette[2] = Config.s;

	if( this->displayOn ) {
		if( Config.fgRainbow ) {
			palette[1] = this->currentRainbowColor;
		}
	} else {
		for( int i = 0; i <= 2; i++ ) {
			palette[i] = LEDMatrix::black;
		}
	}
}
//---------------------------------------------------------------------------------------
// process
//
// Drives internal data flow, should be called repeatedly from main loop()
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::process() {
	if( Config.debugMode )
		return;

	// check time values against boundaries
	if( this->h > 23 || this->h < 0 )
		this->h = 0;
	if( this->m > 59 || this->m < 0 )
		this->m = 0;
	if( this->s > 59 || this->s < 0 )
		this->s = 0;
	if( this->ms > 999 || this->ms < 0 )
		this->ms = 0;

	if( Config.autoOnOff ) {
		// check against switch off
		if( this->h == Config.autoOffHour && this->m == Config.autoOffMin )
			displayOn = false;
		if( this->h == Config.autoOnHour && this->m == Config.autoOnMin )
			displayOn = true;
	}

	palette_entry palette[3];
	this->preparePalette( palette );
	bool displayTimeChanged = this->displayTimeChanged();

	int lm = this->lastM;
	int lh = this->lastH;

	this->lastM = this->m;
	this->lastH = this->h;

	// deal with random handling
	if( this->randomMode != DisplayMode::plain && displayTimeChanged ) {
		this->randomMode = randomModes[random( NUM_RANDOM_MODES )];
		this->mode = this->randomMode;
		Serial.printf( "random: mode changed to=%i\r\n", this->mode );
	}

	if( this->rainbowTicker > 0 ) {
		this->rainbowTicker--;
	}
	if( Config.fgRainbow && this->rainbowTicker <= 0 ) {
		this->rainbowTicker = 800 * ( 3 - Config.rainbowSpeed );
		if( this->rainbowIndex++ > 64 )
			this->rainbowIndex = 0;
		HsbColor hsb( (float)this->rainbowIndex / 64.0, 1.0, 1.0 );
		RgbColor col( hsb );
		this->currentRainbowColor.r = col.R;
		this->currentRainbowColor.g = col.G;
		this->currentRainbowColor.b = col.B;
		Serial.printf( "rainbow=%i, r=%i, g=%i b=%i\r\n", this->rainbowIndex, col.R, col.G, col.B );
	}

	uint32_t buf[NUM_PIXELS >> 2 + 1]; // use u32 just to ensure it is aligned

	// Serial.printf("mode=%i\r\n", this->mode);
	switch( this->mode ) {
	case DisplayMode::wifiManager:
		this->renderWifiManager();
		break;
	case DisplayMode::yellowHourglass:
		this->renderHourglass( Config.hourglassState, false );
		break;
	case DisplayMode::greenHourglass:
		this->renderHourglass( Config.hourglassState, true );
		break;
	case DisplayMode::update:
		this->renderUpdate();
		break;
	case DisplayMode::updateComplete:
		this->renderUpdateComplete();
		break;
	case DisplayMode::updateError:
		this->renderUpdateError();
		break;
	case DisplayMode::red:
		this->renderRed();
		break;
	case DisplayMode::green:
		this->renderGreen();
		break;
	case DisplayMode::blue:
		this->renderBlue();
		break;
	case DisplayMode::fire:
		this->renderFire();
		break;
	case DisplayMode::plasma:
		this->renderPlasma();
		break;
	case DisplayMode::flyingLettersVerticalUp:
	case DisplayMode::flyingLettersVerticalDown:
		this->renderFlyingLetters( displayTimeChanged );
		break;
	case DisplayMode::explode:
		this->renderExplosion( displayTimeChanged, lh, lm );
		break;
	case DisplayMode::snake:
		this->renderSnake( displayTimeChanged, lh, lm );
		break;
	case DisplayMode::moon:
		this->renderMoon();
		break;
	case DisplayMode::matrix:
		this->renderMatrix();
		break;
	case DisplayMode::heart:
		this->renderHeart();
		break;
	case DisplayMode::stars:
		this->renderStars();
		break;
	case DisplayMode::fade:
		this->renderTime( (uint8_t*)buf, this->h, this->m, this->s, this->ms );
		this->set( (uint8_t*)buf, palette, false );
		this->fade();
		break;

	case DisplayMode::random:
	case DisplayMode::plain:
	default:
		this->renderTime( (uint8_t*)buf, this->h, this->m, this->s, this->ms );
		this->set( (uint8_t*)buf, palette, true );
		break;
	}

	// transfer this->currentValues to LEDs
	this->show();
}

//---------------------------------------------------------------------------------------
// setBrightness
//
// Sets the brightness for the WS2812 values. Will be multiplied with each color
// component when sending data to WS2812.
//
// -> brightness: [0...256]
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::setBrightness( int brightness ) { this->brightness = brightness; }

//---------------------------------------------------------------------------------------
// setTime
//
// Sets the time which will be used to drive the LED matrix. The internal time is _not_
// updated, this function must be called repeatedly if the time changes.
//
// -> h, m, s, ms: Time in hours, minutes, seconds, milliseconds
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::setTime( int h, int m, int s, int ms ) {
	this->h = h;
	this->m = m;
	this->s = s;
	this->ms = ms;
}

bool LEDMatrix::modeHasTransition( DisplayMode m ) {
	return m == DisplayMode::snake || m == DisplayMode::flyingLettersVerticalDown ||
	       m == DisplayMode::flyingLettersVerticalUp || m == DisplayMode::explode;
}
//---------------------------------------------------------------------------------------
// setMode
//
// Sets the display mode to one of the members of the DisplayMode enum and thus changes
// what will be shown on the display during the next calls of LEDMatrix.process()
//
// -> newMode: mode to be set
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::setMode( DisplayMode newMode ) {
	DisplayMode previousMode = this->mode;
	if( newMode == DisplayMode::random ) {
		if( this->randomMode == DisplayMode::plain )
			this->randomMode = randomModes[random( NUM_RANDOM_MODES )];
		newMode = this->randomMode;
	} else {
		this->randomMode = DisplayMode::plain;
	}
	this->mode = newMode;

	if( newMode != previousMode && modeHasTransition( newMode ) ) {
		this->forceTransition = true;
	}

	this->process();
}

//---------------------------------------------------------------------------------------
// set
//
// Sets the internal LED buffer to new values based on an indexed source buffer and an
// associated palette. Does not display colors immediately, fades to new colors instead.
//
// Attention: If buf is PROGMEM, make sure it is aligned at 32 bit and its size is
// a multiple of 4 bytes!
//
// -> buf: indexed source buffer
//	palette: color definition for source buffer
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::set( const uint8_t* buf, palette_entry palette[] ) { this->set( buf, palette, false ); }

//---------------------------------------------------------------------------------------
// set
//
// Sets the internal LED buffer to new values based on an indexed source buffer and an
// associated palette.
//
// Attention: If buf is PROGMEM, make sure it is aligned at 32 bit and its size is
// a multiple of 4 bytes!
//
// -> buf: indexed source buffer
//	  palette: color definition for source buffer
//	  immediately: if true, display buffer immediately; fade to new colors if false
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::set( const uint8_t* buf, palette_entry palette[], bool immediately ) {
	this->setBuffer( this->targetValues, buf, palette );

	if( immediately ) {
		this->setBuffer( this->currentValues, buf, palette );
	}
}

//---------------------------------------------------------------------------------------
// setBuffer
//
// Fills a buffer (e. g. this->targetValues) with color data based on indexed source
// pixels and a palette. Pays attention to 32 bit boundaries, so use with PROGMEM is
// safe.
//
// -> target: color buffer, e. g. this->targetValues or this->currentValues
//    source: buffer with color indexes
//	  palette: colors for indexed source buffer
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::setBuffer( uint8_t* target, const uint8_t* source, palette_entry palette[] ) {
	uint32_t mappedPos, curveOffset;
	uint8_t palette_index;

	// cast source to 32 bit pointer to ensure 32 bit aligned access
	uint32_t* buf = (uint32_t*)source;
	// this holds the current 4 bytes
	uint32_t currentDWord;
	// this is a pointer to the current 4 bytes for access as single bytes
	uint8_t* currentBytes = (uint8_t*)&currentDWord;
	// this counts bytes from 0...3
	uint32_t byteCounter = 0;
	if( ( (uint32_t)source & 3 ) == 0 ) { // 4 byte aligned
		for( int i = 0; i < NUM_PIXELS; i++ ) {
			// get next 4 bytes
			if( byteCounter == 0 )
				currentDWord = buf[i >> 2];

			palette_index = currentBytes[byteCounter];
			// palette_index = source[i];
			mappedPos = LEDMatrix::mapping[i] * 3;
			curveOffset = LEDMatrix::brightnessCurveSelect[i] << 8;

			// select color value using palette and brightness correction curves
			target[mappedPos + 0] = brightnessCurvesR[curveOffset + palette[palette_index].r];
			target[mappedPos + 1] = brightnessCurvesG[curveOffset + palette[palette_index].g];
			target[mappedPos + 2] = brightnessCurvesB[curveOffset + palette[palette_index].b];

			byteCounter = ( byteCounter + 1 ) & 0x03;
		}
	} else {
		for( int i = 0; i < NUM_PIXELS; i++ ) {
			palette_index = source[i];
			mappedPos = LEDMatrix::mapping[i] * 3;
			curveOffset = LEDMatrix::brightnessCurveSelect[i] << 8;

			// select color value using palette and brightness correction curves
			target[mappedPos + 0] = brightnessCurvesR[curveOffset + palette[palette_index].r];
			target[mappedPos + 1] = brightnessCurvesG[curveOffset + palette[palette_index].g];
			target[mappedPos + 2] = brightnessCurvesB[curveOffset + palette[palette_index].b];
		}
	}
}

//---------------------------------------------------------------------------------------
// fade
//
// Fade one step of the color values from this->currentValues[i] to
// this->targetValues[i]. Uses non-linear fade speed depending on distance to target
// value.
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::fade() {
	static int prescaler = 0;
	if( ++prescaler < 2 )
		return;
	prescaler = 0;

	int delta;
	for( int i = 0; i < NUM_PIXELS * 3; i++ ) {
		delta = this->targetValues[i] - this->currentValues[i];
		if( delta > 64 )
			this->currentValues[i] += 8;
		else if( delta > 16 )
			this->currentValues[i] += 4;
		else if( delta > 0 )
			this->currentValues[i]++;
		else if( delta < -64 )
			this->currentValues[i] -= 8;
		else if( delta < -16 )
			this->currentValues[i] -= 4;
		else if( delta < 0 )
			this->currentValues[i]--;
	}
}

//---------------------------------------------------------------------------------------
// show
//
// Internal method, copies this->currentValues to WS2812 object while applying brightness
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::show() {
	uint8_t* data = this->currentValues;
	int ofs = 0;

	// copy current color values to LED object and display it
	for( int i = 0; i < NUM_PIXELS; i++ ) {
		this->strip->SetPixelColor( i, RgbColor( ( (int)data[ofs + 0] * this->brightness ) >> 8,
		                                         ( (int)data[ofs + 1] * this->brightness ) >> 8,
		                                         ( (int)data[ofs + 2] * this->brightness ) >> 8 ) );
		ofs += 3;
	}
	this->strip->Show();
}

//---------------------------------------------------------------------------------------
// getOffset
//
// Calculates the offset of a given RGB triplet inside the LED buffer.
// Does range checking for x and y, uses the internal mapping table.
//
// -> x: x coordinate
//    y: y coordinate
// <- offset for given coordinates
//---------------------------------------------------------------------------------------
int LEDMatrix::getOffset( int x, int y ) {
	if( x >= 0 && y >= 0 && x < LEDMatrix::width && y < LEDMatrix::height ) {
		return LEDMatrix::mapping[x + y * LEDMatrix::width] * 3;
	} else {
		return 0;
	}
}

//---------------------------------------------------------------------------------------
// fillBackground
//
// Initializes the buffer with either background (=0) or seconds progress (=2),
// part of the background will be illuminated with color 2 depending on current
// seconds/milliseconds value, whole screen is backlit when seconds = 59
//
// -> seconds, milliseconds: Time value which the fill process will base on
//    buf: destination buffer
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::fillBackground( int seconds, int milliseconds, uint8_t* buf ) {
	int pos = ( ( ( seconds * 1000 + milliseconds ) * 110 ) / 60000 ) + 1;
	if( pos != lastFillPos ) {
		lastFillPos = pos;
		if( pos == 1 ) { // start again
			if( Config.fillMode == 0 ) {
				fillInvers = false;
			} else if( Config.fillMode == 1 ) {
				fillInvers = !fillInvers;
			}
		}
	}
	// if( Config.fillMode == 0 || Config.fillMode == 1)
	for( int i = 0; i < NUM_PIXELS; i++ )
		buf[i] = ( i < pos ) ? ( fillInvers ? 0 : 2 ) : ( fillInvers ? 2 : 0 );
	// if( Config.fillMode == 2 )
	// 	for (int i = 0; i < NUM_PIXELS; i++) buf[i] = (i == pos) ? (fillInvers?0:2) : (fillInvers?2:0);
}

//---------------------------------------------------------------------------------------
// rendering methods
//---------------------------------------------------------------------------------------

void LEDMatrix::renderCorner( uint8_t* target, int m ) {
	// minutes 1...4 for the corners
	for( int i = 0; i <= ( ( m % 5 ) - 1 ); i++ )
		target[height * width + i] = 1;
}

//---------------------------------------------------------------------------------------
// renderTime
//
// Loads internal buffers with the current time representation
//
// -> target: If NULL, a local buffer is used and time is actually being displayed
//            if not null, time is not being displayed, but the given buffer is being
//            filled with palette indexes representing the time
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::renderTime( uint8_t* target, int h, int m, int s, int ms ) {
	this->fillBackground( s, ms, target );

	if( Config.showItIs ) {
		// set static LEDs
		target[0] = 1; // E
		target[1] = 1; // S

		target[3] = 1; // I
		target[4] = 1; // S
		target[5] = 1; // T
	}
	this->renderCorner( target, m );

	// iterate over minutes_template
	int mt = Config.minuteType;
	if( mt > 1 || mt < 0 )
		mt = 0;
	int adjust_hour = 0;
	for( leds_template_t t : LEDMatrix::minutesTemplate[Config.tmpl][mt] ) {
		// test if this template matches the current minute
		if( m >= t.param1 && m <= t.param2 ) {
			// set all LEDs defined in this template
			for( int i : t.LEDs )
				target[i] = 1;
			adjust_hour = t.param0;
			break;
		}
	}

	// adjust hour display if necessary (e. g. 09:45 = quarter to *TEN* instead of NINE)
	h += adjust_hour;
	if( h > 23 )
		h -= 24;

	// iterate over hours template
	for( leds_template_t t : LEDMatrix::hoursTemplate[Config.tmpl] ) {
		// test if this template matches the current hour
		if( ( t.param1 == h || t.param2 == h ) && ( ( t.param0 == 1 && m < 5 ) ||  // special case full hour
		                                            ( t.param0 == 2 && m >= 5 ) || // special case hour + minutes
		                                            ( t.param0 == 0 ) ) )          // normal case
		{
			// set all LEDs defined in this template
			for( int i : t.LEDs )
				target[i] = 1;
			break;
		}
	}

	// DEBUG
	static int last_minutes = -1;
	if( last_minutes != this->m ) {
		last_minutes = this->m;
		Serial.printf( "h=%i, m=%i, s=%i\r\n", this->h, this->m, this->s );
		for( int y = 0; y < 10; y++ ) {
			for( int x = 0; x < 11; x++ ) {
				Serial.print( target[y * 11 + x] );
				Serial.print( ' ' );
			}
			Serial.println( ' ' );
		}
	}
}

//---------------------------------------------------------------------------------------
// renderHourglass
//
// Immediately displays an animation step of the hourglass animation.
// ATTENTION: Animation frames must start at 32 bit boundary each!
//
// -> animationStep: Number of current frame [0...HOURGLASS_ANIMATION_FRAMES]
//    green: Flag to switch the palette color 3 to green instead of yellow (used in the
//           second half of the hourglass animation to indicate the short wait-for-OTA
//           window)
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::renderHourglass( uint8_t animationStep, bool green ) {
	// colors in palette: black, white, yellow
	palette_entry p[] = { { 0, 0, 0 }, { 255, 255, 255 }, { 255, 255, 0 }, { 255, 255, 0 } };

	// delete red component in palette entry 3 to make this color green
	if( green )
		p[3].r = 0;

	if( animationStep >= HOURGLASS_ANIMATION_FRAMES )
		animationStep = 0;
	this->set( hourglass_animation[animationStep], p, true );
}

//---------------------------------------------------------------------------------------
// renderMatrix
//
// Renders one frame of the matrix animation and displays it immediately
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::renderMatrix() {
	// clear buffer
	memset( this->currentValues, 0, sizeof( this->currentValues ) );

	// sort by y coordinate for correct overlapping
	std::sort( matrix.begin(), matrix.end() );

	// iterate over all matrix objects, move and render them
	for( MatrixObject& m : this->matrix )
		m.render( this->currentValues );
}
// clang-format off
const uint32_t LEDMatrix::moonphases[8][10] = {
	  {
				  0b0000111000000000,
				  0b0011000110000000,
				  0b0100000001000000,
				  0b0100000001000000,
				  0b1000000000100000,
				  0b1000000000100000,
				  0b0100000001000000,
				  0b0100000001000000,
				  0b0011000110000000,
				  0b0000111000000000
	  },
		{
				  0b0000111000000000,
				  0b0000001110000000,
				  0b0000000111000000,
				  0b0000000111000000,
				  0b0000000111100000,
				  0b0000000111100000,
				  0b0000000111000000,
				  0b0000000111000000,
				  0b0000001110000000,
				  0b0000111000000000
	  },
		{
				  0b0000011000000000,
				  0b0000011110000000,
				  0b0000011111000000,
				  0b0000011111000000,
				  0b0000011111100000,
				  0b0000011111100000,
				  0b0000011111000000,
				  0b0000011111000000,
				  0b0000011110000000,
				  0b0000011000000000
	  },
		{
				  0b0000111000000000,
				  0b0001111110000000,
				  0b0001111111000000,
				  0b0001111111000000,
				  0b0001111111100000,
				  0b0001111111100000,
				  0b0001111111000000,
				  0b0001111111000000,
				  0b0001111110000000,
				  0b0000111000000000
	  },
		{
				  0b0000111000000000,
				  0b0011111110000000,
				  0b0111111111000000,
				  0b0111111111000000,
				  0b1111111111100000,
				  0b1111111111100000,
				  0b0111111111000000,
				  0b0111111111000000,
				  0b0011111110000000,
				  0b0000111000000000
	  },
		{
				  0b0000111000000000,
				  0b0011111100000000,
				  0b0111111100000000,
				  0b0111111100000000,
				  0b1111111100000000,
				  0b1111111100000000,
				  0b0111111100000000,
				  0b0111111100000000,
				  0b0011111100000000,
				  0b0000111000000000
	  },
		{
				  0b0000110000000000,
				  0b0011110000000000,
				  0b0111110000000000,
				  0b0111110000000000,
				  0b1111110000000000,
				  0b1111110000000000,
				  0b0111110000000000,
				  0b0111110000000000,
				  0b0011110000000000,
				  0b0000110000000000
	  },
		{
				  0b0000111000000000,
				  0b0011100000000000,
				  0b0111000000000000,
				  0b0111000000000000,
				  0b1111000000000000,
				  0b1111000000000000,
				  0b0111000000000000,
				  0b0111000000000000,
				  0b0011100000000000,
				  0b0000111000000000
				
		}
};
// clang-format on

int LEDMatrix::getMoonphase( int y, int m, int d ) {
	int b, c, e;
	double jd;
	if( m < 3 ) {
		y--;
		m += 12;
	}
	++m;
	c = 365.25 * y;
	e = 30.6 * m;
	jd = c + e + d - 694039.09; // jd is total days elapsed
	jd /= 29.53;                // divide by the moon cycle (29.53 days)
	b = jd;                     // int(jd) -> b, take integer part of jd
	jd -= b;                    // subtract integer part to leave fractional part of original jd
	b = jd * 8 + 0.5;           // scale fraction from 0-8 and round by adding 0.5
	b = b & 7;                  // 0 and 8 are the same so turn 8 into 0
	return b;
}

void LEDMatrix::renderMoon() {
	palette_entry palette[3];
	this->preparePalette( palette );
	palette[1] = { 255, 255, 255 };
	int phase = getMoonphase( year, month, day );
	// Serial.printf("render moon: %i, %i, %i -> phase=%i\r\n", year, month, day, phase);
	this->fillBackground( this->s, this->ms, animationBuf );

	for( int i = 0; i < height; i++ ) {
		uint32_t pattern = moonphases[phase][i];
		for( int j = 0; j < width; j++ ) {
			if( pattern & ( 1 << ( 15 - j ) ) )
				animationBuf[i * width + j] = 1;
		}
		/*if( this->m != this->lastM ) {
		  Serial.printf("moon: %i %i %i %i %i %i %i %i %i %i %i \r\n",
		  animationBuf[i*width+0],animationBuf[i*width+1],animationBuf[i*width+2],animationBuf[i*width+3],animationBuf[i*width+4],animationBuf[i*width+5],animationBuf[i*width+6],
		  animationBuf[i*width+7],animationBuf[i*width+8],animationBuf[i*width+9],animationBuf[i*width+10] );
		}*/
	}
	// Serial.printf("buf = %08x\r\n", animationBuf);
	this->set( animationBuf, palette, true );
}

const palette_entry LEDMatrix::black = { 0, 0, 0 };
// clang-format off
const palette_entry LEDMatrix::firePalette[256] = {
	{0, 0, 0}, {4, 0, 0}, {8, 0, 0}, {12, 0, 0}, {16, 0, 0}, {20, 0, 0}, {24, 0, 0}, {28, 0, 0},
	{32, 0, 0}, {36, 0, 0}, {40, 0, 0}, {44, 0, 0}, {48, 0, 0}, {52, 0, 0}, {56, 0, 0}, {60, 0, 0},
	{64, 0, 0}, {68, 0, 0}, {72, 0, 0}, {76, 0, 0}, {80, 0, 0}, {85, 0, 0}, {89, 0, 0}, {93, 0, 0},
	{97, 0, 0}, {101, 0, 0}, {105, 0, 0}, {109, 0, 0}, {113, 0, 0}, {117, 0, 0}, {121, 0, 0}, {125, 0, 0},
	{129, 0, 0}, {133, 0, 0}, {137, 0, 0}, {141, 0, 0}, {145, 0, 0}, {149, 0, 0}, {153, 0, 0}, {157, 0, 0},
	{161, 0, 0}, {165, 0, 0}, {170, 0, 0}, {174, 0, 0}, {178, 0, 0}, {182, 0, 0}, {186, 0, 0}, {190, 0, 0},
	{194, 0, 0}, {198, 0, 0}, {202, 0, 0}, {206, 0, 0}, {210, 0, 0}, {214, 0, 0}, {218, 0, 0}, {222, 0, 0},
	{226, 0, 0}, {230, 0, 0}, {234, 0, 0}, {238, 0, 0}, {242, 0, 0}, {246, 0, 0}, {250, 0, 0}, {255, 0, 0},
	{255, 0, 0}, {255, 8, 0}, {255, 16, 0}, {255, 24, 0}, {255, 32, 0}, {255, 41, 0}, {255, 49, 0}, {255, 57, 0},
	{255, 65, 0}, {255, 74, 0}, {255, 82, 0}, {255, 90, 0}, {255, 98, 0}, {255, 106, 0}, {255, 115, 0}, {255, 123, 0},
	{255, 131, 0}, {255, 139, 0}, {255, 148, 0}, {255, 156, 0}, {255, 164, 0}, {255, 172, 0}, {255, 180, 0}, {255, 189, 0},
	{255, 197, 0}, {255, 205, 0}, {255, 213, 0}, {255, 222, 0}, {255, 230, 0}, {255, 238, 0}, {255, 246, 0}, {255, 255, 0},
	{255, 255, 0}, {255, 255, 1}, {255, 255, 3}, {255, 255, 4}, {255, 255, 6}, {255, 255, 8}, {255, 255, 9}, {255, 255, 11},
	{255, 255, 12}, {255, 255, 14}, {255, 255, 16}, {255, 255, 17}, {255, 255, 19}, {255, 255, 20}, {255, 255, 22}, {255, 255, 24},
	{255, 255, 25}, {255, 255, 27}, {255, 255, 28}, {255, 255, 30}, {255, 255, 32}, {255, 255, 33}, {255, 255, 35}, {255, 255, 36},
	{255, 255, 38}, {255, 255, 40}, {255, 255, 41}, {255, 255, 43}, {255, 255, 44}, {255, 255, 46}, {255, 255, 48}, {255, 255, 49},
	{255, 255, 51}, {255, 255, 52}, {255, 255, 54}, {255, 255, 56}, {255, 255, 57}, {255, 255, 59}, {255, 255, 60}, {255, 255, 62},
	{255, 255, 64}, {255, 255, 65}, {255, 255, 67}, {255, 255, 68}, {255, 255, 70}, {255, 255, 72}, {255, 255, 73}, {255, 255, 75},
	{255, 255, 76}, {255, 255, 78}, {255, 255, 80}, {255, 255, 81}, {255, 255, 83}, {255, 255, 85}, {255, 255, 86}, {255, 255, 88},
	{255, 255, 89}, {255, 255, 91}, {255, 255, 93}, {255, 255, 94}, {255, 255, 96}, {255, 255, 97}, {255, 255, 99}, {255, 255, 101},
	{255, 255, 102}, {255, 255, 104}, {255, 255, 105}, {255, 255, 107}, {255, 255, 109}, {255, 255, 110}, {255, 255, 112}, {255, 255, 113},
	{255, 255, 115}, {255, 255, 117}, {255, 255, 118}, {255, 255, 120}, {255, 255, 121}, {255, 255, 123}, {255, 255, 125}, {255, 255, 126},
	{255, 255, 128}, {255, 255, 129}, {255, 255, 131}, {255, 255, 133}, {255, 255, 134}, {255, 255, 136}, {255, 255, 137}, {255, 255, 139},
	{255, 255, 141}, {255, 255, 142}, {255, 255, 144}, {255, 255, 145}, {255, 255, 147}, {255, 255, 149}, {255, 255, 150}, {255, 255, 152},
	{255, 255, 153}, {255, 255, 155}, {255, 255, 157}, {255, 255, 158}, {255, 255, 160}, {255, 255, 161}, {255, 255, 163}, {255, 255, 165},
	{255, 255, 166}, {255, 255, 168}, {255, 255, 170}, {255, 255, 171}, {255, 255, 173}, {255, 255, 174}, {255, 255, 176}, {255, 255, 178},
	{255, 255, 179}, {255, 255, 181}, {255, 255, 182}, {255, 255, 184}, {255, 255, 186}, {255, 255, 187}, {255, 255, 189}, {255, 255, 190},
	{255, 255, 192}, {255, 255, 194}, {255, 255, 195}, {255, 255, 197}, {255, 255, 198}, {255, 255, 200}, {255, 255, 202}, {255, 255, 203},
	{255, 255, 205}, {255, 255, 206}, {255, 255, 208}, {255, 255, 210}, {255, 255, 211}, {255, 255, 213}, {255, 255, 214}, {255, 255, 216},
	{255, 255, 218}, {255, 255, 219}, {255, 255, 221}, {255, 255, 222}, {255, 255, 224}, {255, 255, 226}, {255, 255, 227}, {255, 255, 229},
	{255, 255, 230}, {255, 255, 232}, {255, 255, 234}, {255, 255, 235}, {255, 255, 237}, {255, 255, 238}, {255, 255, 240}, {255, 255, 242},
	{255, 255, 243}, {255, 255, 245}, {255, 255, 246}, {255, 255, 248}, {255, 255, 250}, {255, 255, 251}, {255, 255, 253}, {255, 255, 255}
};
const palette_entry LEDMatrix::plasmaPalette[256] = {
	{255, 0, 0}, {255, 6, 0}, {255, 12, 0}, {255, 18, 0}, {255, 24, 0}, {255, 30, 0}, {255, 36, 0}, {255, 42, 0},
	{255, 48, 0}, {255, 54, 0}, {255, 60, 0}, {255, 66, 0}, {255, 72, 0}, {255, 78, 0}, {255, 84, 0}, {255, 90, 0},
	{255, 96, 0}, {255, 102, 0}, {255, 108, 0}, {255, 114, 0}, {255, 120, 0}, {255, 126, 0}, {255, 131, 0}, {255, 137, 0},
	{255, 143, 0}, {255, 149, 0}, {255, 155, 0}, {255, 161, 0}, {255, 167, 0}, {255, 173, 0}, {255, 179, 0}, {255, 185, 0},
	{255, 191, 0}, {255, 197, 0}, {255, 203, 0}, {255, 209, 0}, {255, 215, 0}, {255, 221, 0}, {255, 227, 0}, {255, 233, 0},
	{255, 239, 0}, {255, 245, 0}, {255, 251, 0}, {253, 255, 0}, {247, 255, 0}, {241, 255, 0}, {235, 255, 0}, {229, 255, 0},
	{223, 255, 0}, {217, 255, 0}, {211, 255, 0}, {205, 255, 0}, {199, 255, 0}, {193, 255, 0}, {187, 255, 0}, {181, 255, 0},
	{175, 255, 0}, {169, 255, 0}, {163, 255, 0}, {157, 255, 0}, {151, 255, 0}, {145, 255, 0}, {139, 255, 0}, {133, 255, 0},
	{128, 255, 0}, {122, 255, 0}, {116, 255, 0}, {110, 255, 0}, {104, 255, 0}, {98, 255, 0}, {92, 255, 0}, {86, 255, 0},
	{80, 255, 0}, {74, 255, 0}, {68, 255, 0}, {62, 255, 0}, {56, 255, 0}, {50, 255, 0}, {44, 255, 0}, {38, 255, 0},
	{32, 255, 0}, {26, 255, 0}, {20, 255, 0}, {14, 255, 0}, {8, 255, 0}, {2, 255, 0}, {0, 255, 4}, {0, 255, 10},
	{0, 255, 16}, {0, 255, 22}, {0, 255, 28}, {0, 255, 34}, {0, 255, 40}, {0, 255, 46}, {0, 255, 52}, {0, 255, 58},
	{0, 255, 64}, {0, 255, 70}, {0, 255, 76}, {0, 255, 82}, {0, 255, 88}, {0, 255, 94}, {0, 255, 100}, {0, 255, 106},
	{0, 255, 112}, {0, 255, 118}, {0, 255, 124}, {0, 255, 129}, {0, 255, 135}, {0, 255, 141}, {0, 255, 147}, {0, 255, 153},
	{0, 255, 159}, {0, 255, 165}, {0, 255, 171}, {0, 255, 177}, {0, 255, 183}, {0, 255, 189}, {0, 255, 195}, {0, 255, 201},
	{0, 255, 207}, {0, 255, 213}, {0, 255, 219}, {0, 255, 225}, {0, 255, 231}, {0, 255, 237}, {0, 255, 243}, {0, 255, 249},
	{0, 255, 255}, {0, 249, 255}, {0, 243, 255}, {0, 237, 255}, {0, 231, 255}, {0, 225, 255}, {0, 219, 255}, {0, 213, 255},
	{0, 207, 255}, {0, 201, 255}, {0, 195, 255}, {0, 189, 255}, {0, 183, 255}, {0, 177, 255}, {0, 171, 255}, {0, 165, 255},
	{0, 159, 255}, {0, 153, 255}, {0, 147, 255}, {0, 141, 255}, {0, 135, 255}, {0, 129, 255}, {0, 124, 255}, {0, 118, 255},
	{0, 112, 255}, {0, 106, 255}, {0, 100, 255}, {0, 94, 255}, {0, 88, 255}, {0, 82, 255}, {0, 76, 255}, {0, 70, 255},
	{0, 64, 255}, {0, 58, 255}, {0, 52, 255}, {0, 46, 255}, {0, 40, 255}, {0, 34, 255}, {0, 28, 255}, {0, 22, 255},
	{0, 16, 255}, {0, 10, 255}, {0, 4, 255}, {2, 0, 255}, {8, 0, 255}, {14, 0, 255}, {20, 0, 255}, {26, 0, 255},
	{32, 0, 255}, {38, 0, 255}, {44, 0, 255}, {50, 0, 255}, {56, 0, 255}, {62, 0, 255}, {68, 0, 255}, {74, 0, 255},
	{80, 0, 255}, {86, 0, 255}, {92, 0, 255}, {98, 0, 255}, {104, 0, 255}, {110, 0, 255}, {116, 0, 255}, {122, 0, 255},
	{128, 0, 255}, {133, 0, 255}, {139, 0, 255}, {145, 0, 255}, {151, 0, 255}, {157, 0, 255}, {163, 0, 255}, {169, 0, 255},
	{175, 0, 255}, {181, 0, 255}, {187, 0, 255}, {193, 0, 255}, {199, 0, 255}, {205, 0, 255}, {211, 0, 255}, {217, 0, 255},
	{223, 0, 255}, {229, 0, 255}, {235, 0, 255}, {241, 0, 255}, {247, 0, 255}, {253, 0, 255}, {255, 0, 251}, {255, 0, 245},
	{255, 0, 239}, {255, 0, 233}, {255, 0, 227}, {255, 0, 221}, {255, 0, 215}, {255, 0, 209}, {255, 0, 203}, {255, 0, 197},
	{255, 0, 191}, {255, 0, 185}, {255, 0, 179}, {255, 0, 173}, {255, 0, 167}, {255, 0, 161}, {255, 0, 155}, {255, 0, 149},
	{255, 0, 143}, {255, 0, 137}, {255, 0, 131}, {255, 0, 126}, {255, 0, 120}, {255, 0, 114}, {255, 0, 108}, {255, 0, 102},
	{255, 0, 96}, {255, 0, 90}, {255, 0, 84}, {255, 0, 78}, {255, 0, 72}, {255, 0, 66}, {255, 0, 60}, {255, 0, 54},
	{255, 0, 48}, {255, 0, 42}, {255, 0, 36}, {255, 0, 30}, {255, 0, 24}, {255, 0, 18}, {255, 0, 12}, {255, 0, 6}
};
// clang-format on

double _time = 0;
void LEDMatrix::renderPlasma() {
	int color;
	double cx, cy, xx, yy;

	_time += 0.05;

	for( int y = 0; y < LEDMatrix::height; y++ ) {
		yy = (double)y / (double)LEDMatrix::height / 3.0;
		for( int x = 0; x < LEDMatrix::width; x++ ) {
			xx = (double)x / (double)LEDMatrix::width / 3.0;
			cx = xx + 0.5 * sin( _time / 5.0 );
			cy = (double)y / (double)LEDMatrix::height / 3.0 + 0.5 * sin( _time / 3.0 );
			color = ( sin( sqrt( 100 * ( cx * cx + cy * cy ) + 1 + _time ) +
			               6.0 * ( xx * sin( _time / 2 ) + yy * cos( _time / 3 ) + _time / 4.0 ) ) +
			          1.0 ) *
			        128.0;
			plasmaBuf[x + y * LEDMatrix::width] = color;
		}
	}
	// for(int i = width*height; i < width*height+4; i++ ) plasmaBuf[i]=0;
	this->set( plasmaBuf, (palette_entry*)plasmaPalette, true );
}

void LEDMatrix::renderFire() {
	int f;

	// iterate over bottom row, create fire seed
	for( int i = 0; i < LEDMatrix::width; i++ ) {
		// only set hot spot with probability of 1/4
		f = ( random( 4 ) == 0 ) ? random( 256 ) : 0;

		// update one pixel in bottom row
		fireBuf[i + ( LEDMatrix::height - 1 ) * LEDMatrix::width] = f;
	}

	int y1, y2, l, r;
	for( int y = 0; y < LEDMatrix::height - 1; y++ ) {
		y1 = y + 1;
		if( y1 >= LEDMatrix::height )
			y1 = LEDMatrix::height - 1;
		y2 = y + 2;
		if( y2 >= LEDMatrix::height )
			y2 = LEDMatrix::height - 1;
		for( int x = 0; x < LEDMatrix::width; x++ ) {
			l = x - 1;
			if( l < 0 )
				l = 0;
			r = x + 1;
			if( r >= LEDMatrix::width )
				r = LEDMatrix::width - 1;
			fireBuf[x + y * LEDMatrix::width] =
			    ( ( fireBuf[y1 * LEDMatrix::width + l] + fireBuf[y1 * LEDMatrix::width + x] +
			        fireBuf[y1 * LEDMatrix::width + r] + fireBuf[y2 * LEDMatrix::width + x] ) *
			      32 ) /
			    129;
		}
	}
	this->set( fireBuf, (palette_entry*)firePalette, true );
	delay( 100 );
}

//---------------------------------------------------------------------------------------
// renderStars
//
// Renders one frame of the stars animation and displays it immediately
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::renderStars() {
	// clear buffer
	memset( this->currentValues, 0, sizeof( this->currentValues ) );

	for( StarObject& s : this->stars )
		s.render( this->currentValues, this->stars );
}

//---------------------------------------------------------------------------------------
// renderHeart
//
// Renders one frame of the heart animation and displays it immediately
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::renderHeart() {
	palette_entry palette[2];
	// clang-format off
	uint8_t heart[] = {
		0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
		0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1
	};
	// clang-format on
	palette[0] = { 0, 0, 0 };
	palette[1] = { (uint8_t)this->heartBrightness, 0, 0 };
	this->set( heart, palette, true );

	switch( this->heartState ) {
	case 0:
		if( this->heartBrightness >= 255 )
			this->heartState = 1;
		else
			this->heartBrightness += 32;
		break;

	case 1:
		if( this->heartBrightness < 128 )
			this->heartState = 2;
		else
			this->heartBrightness -= 32;
		break;

	case 2:
		if( this->heartBrightness >= 255 )
			this->heartState = 3;
		else
			this->heartBrightness += 32;
		break;

	case 3:
	default:
		if( this->heartBrightness <= 0 )
			this->heartState = 0;
		else
			this->heartBrightness -= 4;
		break;
	}

	if( this->heartBrightness > 255 )
		this->heartBrightness = 255;
	if( this->heartBrightness < 0 )
		this->heartBrightness = 0;
}

//---------------------------------------------------------------------------------------
// prepareExplosion
//
// Prepare particles based on current screen state
//
//
// -> source: buffer to read the currently active LEDs from
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::prepareExplosion( uint8_t* source ) {
#define PARTICLE_COUNT 16
#define PARTICLE_SPEED 0.15f

	float vx, vy, angle;
	int ofs = 0;
	Particle* p;
	int delay;
	Serial.printf( "prepare explosion ... \n\r" );
	// compute angle increment
	float angle_increment = 2.0f * 3.141592654f / (float)( PARTICLE_COUNT );

	/*if( this->particles.size() == 0 ) {
	  for (int i = 0; i < PARTICLE_COUNT; i++) { // init
	    p = new Particle(0, 0, 0, 0, 100);
	    this->particles.push_back(p);
	  }
	}*/
	// iterate over every position in the screen buffer
	for( int y = 0; y < LEDMatrix::height; y++ ) {
		for( int x = 0; x < LEDMatrix::width; x++ ) {
			// create entry in particles vector if current pixel is foreground
			if( source[ofs++] == 1 ) {
				// add a random delay of zero to approx. 3 seconds to each
				// explosion
				delay = random( 300 );

				// start with angle of zero radians, assign velocity vector
				// placed on a circle to each particle
				angle = 0;
				for( int i = 0; i < PARTICLE_COUNT; i++ ) {
					// calculate particle speed vector based on angle and
					// absolute speed value
					vx = PARTICLE_SPEED * sin( angle );
					vy = PARTICLE_SPEED * cos( angle );

					// this->particles[i]->init(x, y, vx, vy, delay);
					p = new Particle( x, y, vx, vy, delay );
					this->particles.push_back( p );
					angle += angle_increment;
				}
			}
		}
	}
}

bool LEDMatrix::activeParticles() {
	int c = 0;
	for( Particle* p : this->particles )
		if( p->alive )
			c++;
	return c;
}

void LEDMatrix::renderSnake( bool transition, int h, int m ) {
	uint8_t buf[NUM_PIXELS] __attribute__( ( aligned( 4 ) ) );
	uint8_t act[NUM_PIXELS] __attribute__( ( aligned( 4 ) ) );
	palette_entry palette[5]; // 4/5. color for snake
	this->preparePalette( palette );

	palette[3] = { 255, 0, 0 };
	palette[4] = { 0, 255, 0 };

	if( transition ) {
		// prepare new animation with old time
		this->renderTime( this->animationBuf, h, m, 0, 0 );
		snakeX = 0;
		snakeY = 0;
		snakeDX = 1;
		snakeHead = 0;
		snakeTail = 0;
		snake[snakeHead++] = 0;
		snakeTicker = snakeSpeed;
		Serial.printf( "prepare snake ... \n\r" );
	}

	// create empty buffer filled with seconds color
	this->fillBackground( this->s, this->ms, buf );
	this->fillBackground( this->s, this->ms, act );

	this->renderCorner( buf, m );

	this->renderTime( act, this->h, this->m, this->s, this->ms );

	// animate
	int len = snakeHead > snakeTail ? snakeHead - snakeTail : snakeTail - snakeHead;
	if( snakeY < height || len > 0 ) { // animate until the tail has vanished
		if( snakeTicker <= 0 ) {
			snakeTicker = snakeSpeed;

			if( snakeY < height ) { // move
				// check if there is someting in current line
				int hit = 0;
				for( int x = 0; x < width; x++ ) {
					if( animationBuf[x + snakeY * width] == 1 || act[x + snakeY * width] == 1 ) {
						hit = 1;
						break;
					}
				}
				if( hit )
					snakeX += snakeDX;
				else
					snakeY++;
				if( snakeX < 0 || snakeX > width - 1 ) {
					snakeY++;
					snakeDX = snakeDX == 1 ? -1 : 1;
					snakeX += snakeDX;
				}
				Serial.printf( "snake moved x=%i, y=%i\n\r", snakeX, snakeY );
				snake[snakeHead++] = snakeY * width + snakeX;
			}
			int len = snakeHead > snakeTail ? snakeHead - snakeTail : snakeTail - snakeHead;
			if( len > 8 || snakeY >= height )
				snakeTail++;
			if( snakeHead == SNAKE_LEN )
				snakeHead = 0;
			if( snakeTail == SNAKE_LEN )
				snakeTail = 0;
		} else {
			snakeTicker--;
		}
		for( int y = 0; y < height; y++ ) {
			for( int x = 0; x < width; x++ ) {
				if( y < snakeY ) {
					buf[x + y * width] = act[x + y * width];
				} else if( y > snakeY ) {
					buf[x + y * width] = animationBuf[x + y * width];
				} else {
					if( snakeDX == 1 ) { // rennt rechts
						buf[x + y * width] = x < snakeX ? act[x + y * width] : animationBuf[x + y * width];
					} else {
						buf[x + y * width] = x > snakeX ? act[x + y * width] : animationBuf[x + y * width];
					}
				}
			}
		}
		uint8_t t = snakeTail;
		while( t != snakeHead ) {
			buf[snake[t]] = 4;
			t++;
			if( t == SNAKE_LEN )
				t = 0;
		}
		if( snakeY * width + snakeX < width * height )
			buf[snakeY * width + snakeX] = 3;
		this->set( buf, palette, true );
	} else {
		this->renderTime( buf, this->h, this->m, this->s, this->ms );
		this->set( buf, palette, true );
		this->fade();
	}
}

//---------------------------------------------------------------------------------------
// renderExplosion
//
// Renders the exploding letters animation
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::renderExplosion( bool transition, int h, int m ) {
	uint8_t buf[NUM_PIXELS] __attribute__( ( aligned( 4 ) ) );

	palette_entry palette[3];
	this->preparePalette( palette );

	// check if the displayed time has changed
	if( transition ) {
		// prepare new animation with old time
		this->renderTime( buf, h, m, 0, 0 );
		this->prepareExplosion( buf );
	}

	// create empty buffer filled with seconds color
	this->fillBackground( this->s, this->ms, buf );

	this->renderCorner( buf, m );

	// Do we have something to explode?
	if( this->activeParticles() > 0 ) {
		// transfer background created by fillBackground to target buffer
		this->set( buf, palette, true );
		// iterate over all particles
		for( Particle* p : this->particles ) {
			// move and render current particle
			if( p->alive )
				p->render( this->currentValues, palette );
		}
	} else {
		for( int i = this->particles.size() - 1; i >= 0; i-- )
			delete this->particles[i]; // delete in reverse order
		this->particles.clear();
		// present the current time in boring mode with simple fading
		this->renderTime( buf, this->h, this->m, this->s, this->ms );
		this->set( buf, palette, false );
		this->fade();
	}
}

//---------------------------------------------------------------------------------------
// prepareFlyingLetters
//
// Sets the current buffer as target state for flying letters, initializes current
// positions of the letters below the visible area with some random jitter
//
// -> source: buffer to read the currently active LEDs from
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::prepareFlyingLetters( uint8_t* source ) {
	// transfer the previous flying letters in the leaving letters vector to prepare for
	// outgoing animation
	this->leavingLetters.clear();
	for( xy_t& p : this->arrivingLetters ) {
		// delay every letter depending on its position
		// and set new target coordinate
		if( this->mode == DisplayMode::flyingLettersVerticalUp ) {
			p.delay = p.y * 2 + p.x + 1 + random( 5 );
			p.yTarget = -1;
		} else {
			p.delay = ( LEDMatrix::height - p.y - 1 ) * 2 + p.x + 1 + random( 5 );
			p.yTarget = LEDMatrix::height;
		}
		this->leavingLetters.push_back( p );
	}

	// initialize arriving letters from scratch
	this->arrivingLetters.clear();
	int ofs = 0;

	// iterate over every position in the screen buffer
	for( int y = 0; y < LEDMatrix::height; y++ ) {
		for( int x = 0; x < LEDMatrix::width; x++ ) {
			// create entry in arrivingLetters vector if current pixel is foreground
			if( source[ofs++] == 1 ) {
				if( this->mode == DisplayMode::flyingLettersVerticalUp ) {
					xy_t p = { x, y, x, LEDMatrix::height, y * 2 + x + 1 + random( 5 ), 200, 0 };
					this->arrivingLetters.push_back( p );
				} else {
					xy_t p = { x, y, x, -1, ( LEDMatrix::height - y - 1 ) * 2 + x + 1 + random( 5 ), 200, 0 };
					this->arrivingLetters.push_back( p );
				}
			}
		}
	}

	// DEBUG
	Serial.printf( "h=%i, m=%i, s=%i, lastH=%i, lastM=%i\r\n", this->h, this->m, this->s, this->lastH, this->lastM );
	Serial.println( "leavingLetters:" );
	for( xy_t& p : this->leavingLetters ) {
		Serial.printf( "  counter=%i, delay=%i, speed=%i, x=%i, y=%i, xTarget=%i, yTarget=%i\r\n", p.counter, p.delay,
		               p.speed, p.x, p.y, p.xTarget, p.yTarget );
	}
	Serial.println( "arrivingLetters:" );
	for( xy_t& p : this->arrivingLetters ) {
		Serial.printf( "  counter=%i, delay=%i, speed=%i, x=%i, y=%i, xTarget=%i, yTarget=%i\r\n", p.counter, p.delay,
		               p.speed, p.x, p.y, p.xTarget, p.yTarget );
	}
}

bool LEDMatrix::displayTimeChanged() {
	bool r = forceTransition || ( this->m / 5 != this->lastM / 5 ) || ( this->h != this->lastH );
	forceTransition = false;
	return r;
}

//---------------------------------------------------------------------------------------
// renderFlyingLetters
//
// Takes the current arrivingLetters and leavingLetters to render the flying letters
// animation
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::renderFlyingLetters( bool transition ) {
	uint8_t buf[NUM_PIXELS] __attribute__( ( aligned( 4 ) ) );

	palette_entry palette[3];
	this->preparePalette( palette );

	// check if the displayed time has changed
	if( transition ) {
		// prepare new animation
		this->renderTime( buf, this->h, this->m, this->s, this->ms );
		this->prepareFlyingLetters( buf );
	}

	// create empty buffer filled with seconds color
	this->fillBackground( this->s, this->ms, buf );

	this->renderCorner( buf, m );

	// leaving letters animation has priority
	if( this->leavingLetters.size() > 0 ) {
		// count actually moved letters to detect end of animation
		int movedLetters = 0;

		// iterate over all leavingLetters
		for( xy_t& p : this->leavingLetters ) {
			// draw letter only if inside visible area
			if( p.x >= 0 && p.y >= 0 && p.x < LEDMatrix::width && p.y < LEDMatrix::height )
				buf[p.x + p.y * LEDMatrix::width] = 1;

			// continue with next letter if the current letter already
			// reached its target position
			if( p.y == p.yTarget && p.x == p.xTarget )
				continue;
			p.counter += p.speed;
			movedLetters++;
			if( p.counter >= 1000 ) {
				p.counter -= 1000;
				if( p.delay > 0 ) {
					// do not move if animation of current letter is delayed
					p.delay--;
				} else {
					if( p.y > p.yTarget )
						p.y--;
					else
						p.y++;
				}
			}
		}
		if( movedLetters == 0 )
			this->leavingLetters.clear();
	} else {
		// iterate over all arrivingLetters
		for( xy_t& p : this->arrivingLetters ) {
			// draw letter only if inside visible area
			if( p.x >= 0 && p.y >= 0 && p.x < LEDMatrix::width && p.y < LEDMatrix::height )
				buf[p.x + p.y * LEDMatrix::width] = 1;

			// continue with next letter if the current letter already
			// reached its target position
			if( p.y == p.yTarget && p.x == p.xTarget )
				continue;
			p.counter += p.speed;
			if( p.counter >= 1000 ) {
				p.counter -= 1000;
				if( p.delay > 0 ) {
					// do not move if animation of current letter is delayed
					p.delay--;
				} else {
					if( p.y > p.yTarget )
						p.y--;
					else
						p.y++;
				}
			}
		}
	}

	// present the current content immediately without fading
	this->set( buf, palette, true );
}

//---------------------------------------------------------------------------------------
// renderRed
//
// Renders a complete red frame for testing purposes.
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::renderRed() {
	palette_entry palette[2];
	uint8_t heart[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	palette[0] = { 0, 0, 0 };
	palette[1] = { 32, 0, 0 };
	this->set( heart, palette, true );
}

//---------------------------------------------------------------------------------------
// renderGreen
//
// Renders a complete green frame for testing purposes.
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::renderGreen() {
	palette_entry palette[2];
	uint8_t heart[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	palette[0] = { 0, 0, 0 };
	palette[1] = { 0, 32, 0 };
	this->set( heart, palette, true );
}

//---------------------------------------------------------------------------------------
// renderBlue
//
// Renders a complete blue frame for testing purposes.
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::renderBlue() {
	palette_entry palette[2];
	uint8_t heart[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	palette[0] = { 0, 0, 0 };
	palette[1] = { 0, 0, 32 };
	this->set( heart, palette, true );
}

//---------------------------------------------------------------------------------------
// renderUpdate
//
// Renders the OTA update screen with progress information depending on
// Config.updateProgress
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::renderUpdate() {
	// clang-format off
	uint8_t update[] = {
		0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0,
		0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
		1, 1, 1, 1
	};
	// clang-format on
	palette_entry p[] = { { 0, 0, 0 }, { 255, 0, 0 }, { 42, 21, 0 }, { 255, 85, 0 } };
	for( int i = 0; i < 110; i++ ) {
		if( i < Config.updateProgress ) {
			if( update[i] == 0 )
				update[i] = 2;
			else
				update[i] = 3;
		}
	}
	this->set( update, p, true );
}

//---------------------------------------------------------------------------------------
// renderUpdateComplete
//
// Renders the OTA update complete screen
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::renderUpdateComplete() {
	// clang-format off
	const uint8_t update_ok[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1
	};
	// clang-format on
	palette_entry p[] = { { 0, 21, 0 }, { 0, 255, 0 } };
	this->set( update_ok, p, true );
}

//---------------------------------------------------------------------------------------
// renderUpdateError
//
// Renders the OTA update error screen
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::renderUpdateError() {
	// clang-format off
	const uint8_t update_err[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1
	};
	// clang-format on
	palette_entry p[] = { { 0, 0, 0 }, { 255, 0, 0 } };
	this->set( update_err, p, true );
}

//---------------------------------------------------------------------------------------
// renderWifiManager
//
// Renders the WifiManager screen
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void LEDMatrix::renderWifiManager() {
	// clang-format off
	const uint8_t wifimanager[] = {
		0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
		0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0,
		0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
		1, 1, 1, 1
	};
	// clang-format on
	palette_entry p[] = { { 0, 0, 0 }, { 255, 255, 0 } };
	this->set( wifimanager, p, true );
}
