// ESP8266 Wordclock
// Copyright (C) 2016 Thoralt Franz, https://github.com/thoralt
//
//  See webserver.cpp for description.
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
#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <stdint.h>

#include "config.h"

class WebServer {
public:
	WebServer();
	virtual ~WebServer();
	void begin();
	void process();

private:
	ESP8266WebServer* server = NULL;
	static const char* textPlain;
	static const char* applicationJson;

	String contentType( String filename );
	bool serveFile( String path );
	void handleSaveConfig();
	void handleLoadConfig();
	void handleGetColors();
	void handleGetConfig();
	void handleSetColor();
	void handleNotFound();
	void handleSetTimeZone();
	void handleGetTimeZone();
	void handleSetMode();
	void handleGetMode();
	void handleSetVar();
	void handleGetVar();
	void handleSetHeartbeat();
	void handleGetHeartbeat();
	void handleInfo();
	void handleD();
	void handleH();
	void handleM();
	void handleR();
	void handleG();
	void handleB();
	void handleDebug();
	void handleSetBrightness();
	void handleGetADC();
	void handleGetNtpServer();
	void handleSetNtpServer();
	void extractColor( String argName, palette_entry& result );
};

extern WebServer HttpServer;

#endif
