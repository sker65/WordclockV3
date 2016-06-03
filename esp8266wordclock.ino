// ESP8266 Wordclock
// Copyright (C) 2016 Thoralt Franz, https://github.com/thoralt
//
//  This project is the firmware for a Wordclock consisting of 114 WS2812B LEDs.
//  It implements:
//   - NTP client for time synchronization
//   - a web server for configuration access
//   - mDNS client for easy discovery
//   - class for easy LED access with fading
//   - OTA (over the air) updates
//   - WiFi manager for easy configuration in unknown WiFi networks
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
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <Ticker.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include "ledfunctions.h"
#include "brightness.h"
#include "ntp.h"
#include "webserver.h"
#include "config.h"

//---------------------------------------------------------------------------------------
// Network related variables
//---------------------------------------------------------------------------------------
int OTA_in_progress = 0;

// WiFiServer telnetServer(23);
// WiFiClient telnetClient; 

//---------------------------------------------------------------------------------------
// Timer related variables
//---------------------------------------------------------------------------------------
#define TIMER_RESOLUTION 10
#define HOURGLASS_ANIMATION_PERIOD 100
Ticker timer;
int h = 0;
int m = 0;
int s = 0;
int ms = 0;
int lastSecond = -1;
bool timeVarLock = false;

int hourglassState = 0;
int hourglassPrescaler = 0;

int updateCountdown = 25;

//---------------------------------------------------------------------------------------
// timerCallback
//
// Increments time, decrements timeout and NTP timer
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void timerCallback()
{
	if (!timeVarLock)
	{
		timeVarLock = true;
		ms += TIMER_RESOLUTION;
		if (ms >= 1000)
		{
			ms -= 1000;
			if (++s > 59)
			{
				s = 0;
				if (++m > 59)
				{
					m = 0;
					if (++h > 23)
					{
						h = 0;
					}
				}
			}
		}
		timeVarLock = false;
	}

	if (ms == 0 && Config.heartbeat)
		digitalWrite(2, LOW);
	else
		digitalWrite(2, HIGH);

	if (WebServer.showHourglass)
	{
		hourglassPrescaler += TIMER_RESOLUTION;
		if (hourglassPrescaler >= HOURGLASS_ANIMATION_PERIOD)
		{
			LED.hourglass(hourglassState, WebServer.showGreenHourglass);
			hourglassPrescaler -= HOURGLASS_ANIMATION_PERIOD;
			if (++hourglassState >= HOURGLASS_ANIMATION_FRAMES)
				hourglassState = 0;
		}
	}
}

//---------------------------------------------------------------------------------------
// configModeCallback
//
// ...
//
// -> 
// <- --
//---------------------------------------------------------------------------------------
void configModeCallback(WiFiManager *myWiFiManager) 
{
	WebServer.showHourglass = false;
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
	palette_entry p[] = {{0, 0, 0}, {255, 255, 0}};
	LED.set(wifimanager, p, true);
	Serial.println("Entered config mode");
	Serial.println(WiFi.softAPIP());
	Serial.println(myWiFiManager->getConfigPortalSSID());
}

//---------------------------------------------------------------------------------------
// NtpCallback
//
// Is called by the NTP class upon successful reception of an NTP data packet. Updates
// the global hour, minute, second and millisecond values.
//
// ->
// <- --
//---------------------------------------------------------------------------------------
void NtpCallback(uint8_t _h, uint8_t _m, uint8_t _s, uint8_t _ms)
{
	Serial.println("NtpCallback()");

	// wait if timer variable lock is set
	while (timeVarLock)
		delay(1);

	// lock timer variables to prevent changes during interrupt
	timeVarLock = true;
	h = _h;
	m = _m;
	s = _s;
	ms = _ms;
	timeVarLock = false;
}

//---------------------------------------------------------------------------------------
// setup
//
// Initializes everything
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void setup()
{
	// ESP8266 LED
	pinMode(2, OUTPUT);

	// serial port
	Serial.begin(115200);
	Serial.println();
	Serial.println();
	Serial.println("ESP8266 WordClock setup() begin");

	// timer
	Serial.println("Starting timer");
	timer.attach(TIMER_RESOLUTION / 1000.0, timerCallback);

	// configuration
	Serial.println("Loading configuration");
	Config.begin();

	// LEDs
	Serial.println("Starting LED module");
	LED.begin(5);
	WebServer.showHourglass = true;

	// WiFi
	Serial.println("Initializing WiFi");
	WiFiManager wifiManager;
	wifiManager.setAPCallback(configModeCallback);
	if (!wifiManager.autoConnect("WordClock"))
	{
		Serial.println("failed to connect, timeout");
		delay(1000);
		ESP.reset();
	}

	// OTA update
	Serial.println("Initializing OTA");
	ArduinoOTA.setPort(8266);
	ArduinoOTA.setHostname("WordClock");
	//ArduinoOTA.setPassword((const char *)"123");
	ArduinoOTA.onStart([]()
	{
		WebServer.showHourglass = false;

		const uint8_t update[] = {
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
		OTA_in_progress = 1;
		Serial.println("OTA Start");
		palette_entry p[] = {{0, 0, 0}, {255, 0, 0}};
		LED.set(update, p, true);
	});
	ArduinoOTA.onEnd([]()
	{
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
		palette_entry p[] = {{0, 21, 0}, {0, 255, 0}};
		LED.set(update_ok, p, true);
		Serial.println("\nOTA End");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
	{
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
		palette_entry p[] = {{0, 0, 0}, {255, 0, 0}, {42, 21, 0}, {255, 85, 0}};
		int current_progress = progress * 110 / total;
		for(int i=0; i<110; i++)
		{
			if(i<current_progress)
			{
				if(update[i] == 0) update[i] = 2;
				else update[i] = 3;
			}
		}
		LED.set(update, p, true);
		Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error)
	{
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
		palette_entry p[] = {{0, 0, 0}, {255, 0, 0}};
		LED.set(update_err, p, true);

		Serial.printf("OTA Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
	});
	ArduinoOTA.begin();

	// NTP
	Serial.println("Starting NTP module");
	NTP.begin(Config.ntpserver, NtpCallback, 2);

	// web server
	Serial.println("Starting HTTP server");
	WebServer.begin();

	// telnetServer.begin();
	// telnetServer.setNoDelay(true);
}

//-----------------------------------------------------------------------------------
// loop
//-----------------------------------------------------------------------------------
void loop()
{
	delay(10);

	// do OTA update stuff
	ArduinoOTA.handle();

	// do not continue if OTA update is in progress
	if (OTA_in_progress)
		return;

	if(updateCountdown)
	{
		WebServer.showGreenHourglass = true;
		Serial.print(".");
		delay(100);
		updateCountdown--;
		return;
	}

	WebServer.showGreenHourglass = false;
	WebServer.showHourglass = false;

	// do web server stuff
	WebServer.process();

	if (!WebServer.showHourglass)
	{
		LED.showHeart(WebServer.showHeart || (h == 22 && m == 00));
		LED.showMatrix(WebServer.showMatrix || (h == 13 && m == 37));
		LED.showStars(WebServer.showStars || (h == 23 && m == 00));
		LED.displayTime(h, m, s, ms);
		LED.setBrightness(Brightness.value());
		LED.process();
	}

	// output current time if seconds value has changed
	if (s != lastSecond)
	{
		lastSecond = s;
		Serial.print(STRING2(h) + ":" + STRING2(m) + ":" + STRING2(s));
		Serial.print(String(", ADC=") + Brightness.avg);
		Serial.print(String(", heap=") + ESP.getFreeHeap());
		Serial.println(String(", brightness=") + Brightness.value());
	}

	if (Serial.available())
	{
		int incoming = Serial.read();
		switch (incoming)
		{
		case 'i':
			Serial.println("WordClock ESP8266 ready.");
			break;

		case 'm':
			WebServer.showMatrix = !WebServer.showMatrix;
			break;

		case 'X':
			WiFi.disconnect();
			ESP.reset();
			break;

		default:
			Serial.print("Unknown command '");
			Serial.print((char) incoming);
			Serial.println("'");
			break;
		}
	}

	// if (telnetServer.hasClient())
	// {
	// if(telnetClient.connected())
	// {
	// telnetClient.stop();
	// }
	// telnetClient = telnetServer.available();
	// telnetClient.println("WordClock telnet server ready.");
	// }

}
// ./esptool.py --port /dev/tty.usbserial --baud 460800 write_flash --flash_size=8m 0 /var/folders/yh/bv744591099f3x24xbkc22zw0000gn/T/build006b1a55228a1b90dda210fcddb62452.tmp/test.ino.bin
// FlashSize 1M (256k SPIFFS)
// C:\Python27\python.exe "C:\Program Files\esptool\espota.py" --ip=192.168.178.95 --port=8266 --progress --file=${workspace_loc}\${project_path}\Release\${project_name}.bin
