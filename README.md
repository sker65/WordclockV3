# Links
http://www.leuchtbildshop.net/                                              - Material (Diffusionsfolie)

http://diskussion.christians-bastel-laden.de/index.php                      - Forum (Anleitungen, Folienlieferanten etc)

https://www.mikrocontroller.net/articles/WordClock_mit_WS2812#Frontplatten  - alternative Version (mehr Funktionen)

# changes / extensions by sker65

- added some alternative mapping tables for other word positions (for now only compile time switches)
- added random mode for transitions
- added rainbow color mode.
- added alternative way of spelling minutes (dreiviertel vs. viertel vor)
- make constant "Es ist" display switchable
- code refactored to run with neopixel bus (https://github.com/Makuna/NeoPixelBus) to avoid flickering
- "PROGMEM" aligned to 4 to avoid unaligned access exception
- added new snake transition
- added switch off at night time feature
- cleanup weg ui / JS
- added fill modes for background filling to not just reset the background (big brightness change)
- added moon phase rendering
# esp8266wordclock
Wordclock with WS2812B RGB LED modules driven by an ESP8266 module

This is my interpretation of the popular wordclock project. Key features:

- runs on ESP8266 (ESP-07)
- using Arduino framework
- 114 WS2812B LEDs (driven by neopixel bus (formerly AdaFruit_NeoPixel) library)
- WiFi connected
- WiFiManager allows for easy configuration when WiFi network is not yet configured
- NTP client regularly fetches time
- integrated web server handles configuration interface for colors, time server etc.
- automatic brightness using LDR 

![front](https://github.com/thoralt/esp8266wordclock/blob/master/doc/exploding_letters.gif)
![front](https://github.com/thoralt/esp8266wordclock/blob/master/doc/IMG_5712.JPG)

The clock is built into an [IKEA Ribba picture frame](http://www.ikea.com/de/de/catalog/products/00078051/) (€5), the front plate is screen printed to get fully opaque and sharp mask. The time is displayed using words with five minute resolution and the LEDs in the four corners display the minutes between the word changes. Seconds are visualized using the background slowly filling with an alternative color (configurable using the Web interface).

![screenshot](https://github.com/thoralt/esp8266wordclock/blob/master/doc/IMG_5714_small.PNG)

The configuration interface is accessible using any browser at the URL http://wordclock.local and allows to change foreground color, background color, the seconds progress color and other options.

![back](https://github.com/thoralt/esp8266wordclock/blob/master/doc/IMG_5711.JPG)

The ESP8266 has been wired in dead bug style, I didn't bother to create a PCB for that. [Modules with integrated voltage regulator, buttons, USB and LDR](http://www.cnx-software.com/2015/12/14/3-compact-esp8266-board-includes-rgd-led-photo-resistor-buttons-and-a-usb-to-ttl-interface/) would have been a better option, but delivery from China is so slow and I didn't want to wait that long. The WS2812B LEDs are wired using thin copper wire. When fully powered, the voltage drop on the power wires is quite high and the last LEDs in the chain don't get enough voltage and stop responding. In the next version, I will use thicker wire for the power lines.

The base for the LEDs is made of MDF milled on my CNC mill. It consists of a 12 mm back plate with holes and small rims for the LEDs to rest on and a 12 mm front plate having holes with equal diameter. I added a first diffusor of thin paper between the two plates (to make the LED less visible) and painted the inside of the holes white. On top I added a second diffusor (plastic foil) so the light tunnel gets invisible.
