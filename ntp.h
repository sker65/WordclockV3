// ESP8266 Wordclock
// Copyright (C) 2016 Thoralt Franz, https://github.com/thoralt
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
//  See ntp.cpp for description.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// This code is based on (heavily modified):
// https://github.com/sandeepmistry/esp8266-Arduino/blob/master/esp8266com/esp8266/libraries/ESP8266WiFi/examples/NTPClient
#ifndef _NTP_H_
#define _NTP_H_

#include <stdint.h>
#include <Ticker.h>
#include <WiFiUdp.h>

// type definition for NTP callback
typedef void (*TNtpCallback)(uint8_t hour, uint8_t min, uint8_t sec, uint8_t ms, int year, int mon, int day);

class NtpClass
{
public:
	// public methods
	NtpClass();
	void begin(IPAddress ip, TNtpCallback callback, int timezone, bool DST);
	void setServer(IPAddress address);
	IPAddress getServer();
	void setTimeZone(int timeZone);

	// public members
	bool syncInProgress = false;

private:
	enum class NtpState
	{
		idle, startRequest, waitingForReply, waitingForReload
	};

	int lastSunday(int year, int month, int lastDayInMonth);
	static void tickerFunctionWrapper(NtpClass *obj);
	int dayOfWeek(int y, int m, int d);
	void decodeTime(long long t);
	void tickerFunction();
	bool isDSTactive();
	void sendPacket();
	void parse();

	IPAddress timeServer;
	Ticker ticker;
	WiFiUDP udp;
	NtpState state = NtpState::idle;
	TNtpCallback _callback = NULL;
	int timer = 0;
	int h = 0;
	int m = 0;
	int s = 0;
	int year = 0;
	int month = 0;
	int day = 0;
	int weekday = 0;
	int yearday = 0;
	int ms = 0;
	int tz = 0;
	bool useDST = false;
};

extern NtpClass NTP;

#endif

