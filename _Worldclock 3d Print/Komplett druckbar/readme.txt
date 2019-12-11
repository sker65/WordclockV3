This is a fully printable case for a word-clock. It is designed for use of LED stripes WS281x with 60 LED/m.
Additional there are needed only a RaspberryPi, a 5V power-supply, some wires and the software from here: https://github.com/xSnowHeadx/word-clock
If using an additional USB-soundcard the device can be used as spectrum-analyzer too (see third picture).

Edit 2017/02/28: wordclock_case_half.stl updated

Edit 2018/08/15: now the cheaper ESP8266 can be used instead the expensive RaspberryPi. A first try of the software is available here: https://github.com/xSnowHeadx/WordClock_ESP8266. There are no audio-functions and (at the moment) no moodlight modus supportet. I also designed a fitting case for the ESP8266: https://www.thingiverse.com/thing:2842012

Print Settings
Printer:

FlashForge Dreamer

Rafts:

No

Supports:

No


Notes:
Print two pieces of the half outer case. On the bottom of the case there are marks for the placement of the LED stripes.
For the grid laying direct on the LED stripes use the parts "bottom_grid". They have slots for the stripes and wires. You need two of them. The upper grids are without slots for better shadowing. Here are 4 pieces necessary. These parts are not at double height to give the possibility for placing additional diffusor foils between them when using a clear shade. Print the grids with black material.
The size of the shades is 184x184mm. If your printer has a sufficient build volume you can use the single parts. If not, use the halved parts. The grids are generally halved for better insertion in the case.

I uploaded all thinkable variants of the shades for your convenience. They must be printed with a resolution of 0.2mm and usually with the plate downwards. The first two layers (the plate) must be printed with a transparent or translucent material. After the second layer the filament has to be changed to black.
The types "normal" can be used, when the letters should be visible even if they are not lighted to show the functionality.
The "mirror" types can be used to hide the letters and show only the lighted. With a translucent white plate and a turquois light setting (RGB 0%,78%,100%) there is an "Apple-effect" possible as to see in the photos.
If you want to use your own print strategy I uploaded the shade for the letters and the plate for the shade separate.
If you don't want to print the shade with the 3D-printer you can print the attached wordclock_shade_laserprinter.bmp with a laserprinter on an overhead-foil and use this as shade. Then you should use additional diffusor foils between the grid layers. This looks like the last photo above. It is recommended to stack two or three of the laserprinted foils for better shadowing.
The wiring of the LED starts at the lower left corner and continues as meander from the lower to the upper stripe (see "word-processor.c" in the source files). It is recommended to draw an additional power connection directly from the lower to the uppermost stripe for evenly brightness of the LED all over the area.