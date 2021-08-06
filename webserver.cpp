// ESP8266 Wordclock
// Copyright (C) 2016 Thoralt Franz, https://github.com/thoralt
//
//  This module encapsulates a small webserver. It replies to requests on port 80
//  and triggers actions, manipulates configuration attributes or serves files
//  from the internal flash file system.
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
#include <Arduino.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <stdio.h>

#include "ledfunctions.h"
#include "brightness.h"
#include "webserver.h"
#include "ntp.h"

//---------------------------------------------------------------------------------------
// global instance
//---------------------------------------------------------------------------------------
WebServerClass WebServer = WebServerClass();

//---------------------------------------------------------------------------------------
// WebServerClass
//
// Constructor, currently empty
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
WebServerClass::WebServerClass()
{
}

//---------------------------------------------------------------------------------------
// ~WebServerClass
//
// Destructor, removes allocated web server object
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
WebServerClass::~WebServerClass()
{
	if (this->server)
		delete this->server;
}

//---------------------------------------------------------------------------------------
// begin
//
// Sets up internal handlers and starts the server at port 80
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void WebServerClass::begin()
{
	SPIFFS.begin();

	this->server = new ESP8266WebServer(80);

	this->server->on("/info", std::bind(&WebServerClass::handleInfo, this));
	this->server->on("/saveconfig", std::bind(&WebServerClass::handleSaveConfig, this));
	this->server->on("/loadconfig", std::bind(&WebServerClass::handleLoadConfig, this));
	this->server->on("/config", std::bind(&WebServerClass::handleGetConfig, this));
	this->server->on("/h", std::bind(&WebServerClass::handleH, this));
	this->server->on("/m", std::bind(&WebServerClass::handleM, this));
	this->server->on("/r", std::bind(&WebServerClass::handleR, this));
	this->server->on("/g", std::bind(&WebServerClass::handleG, this));
	this->server->on("/b", std::bind(&WebServerClass::handleB, this));
	this->server->on("/getadc", std::bind(&WebServerClass::handleGetADC, this));
	this->server->on("/setvar", std::bind(&WebServerClass::handleSetVar, this));
	this->server->on("/debug", std::bind(&WebServerClass::handleDebug, this));

	this->server->onNotFound(std::bind(&WebServerClass::handleNotFound, this));
	this->server->begin();
}

//---------------------------------------------------------------------------------------
// process
//
// Must be called repeatedly from main loop
//
// ->
// <- --
//---------------------------------------------------------------------------------------
void WebServerClass::process()
{
	this->server->handleClient();
}

//---------------------------------------------------------------------------------------
// serveFile
//
// Looks up a given file name in internal flash file system, streams the file if found
//
// -> path: name of the file; "index.html" will be added if name ends with "/"
// <- true: file was found and served to client
//	false: file not found
//---------------------------------------------------------------------------------------
bool WebServerClass::serveFile(String path)
{
	Serial.println("WebServerClass::serveFile(): " + path);
	if (path.endsWith("/"))
		path += "index.html";
	if (SPIFFS.exists(path))
	{
		File file = SPIFFS.open(path, "r");
		this->server->streamFile(file, this->contentType(path));
		file.close();
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------------------
// contentType
//
// Returns an HTML content type based on a given file name extension
//
// -> filename: name of the file
// <- HTML content type matching file extension
//---------------------------------------------------------------------------------------
String WebServerClass::contentType(String filename)
{
	if (this->server->hasArg("download")) return "application/octet-stream";
	else if (filename.endsWith(".htm")) return "text/html";
	else if (filename.endsWith(".html")) return "text/html";
	else if (filename.endsWith(".css")) return "text/css";
	else if (filename.endsWith(".js")) return "application/javascript";
	else if (filename.endsWith(".png")) return "image/png";
	else if (filename.endsWith(".gif")) return "image/gif";
	else if (filename.endsWith(".jpg")) return "image/jpeg";
	else if (filename.endsWith(".ico")) return "image/x-icon";
	else if (filename.endsWith(".xml")) return "text/xml";
	else if (filename.endsWith(".pdf")) return "application/x-pdf";
	else if (filename.endsWith(".zip")) return "application/x-zip";
	else if (filename.endsWith(".gz")) return "application/x-gzip";
	return "text/plain";
}

//---------------------------------------------------------------------------------------
// handleM
//
// Handles the /m request, increments the minutes counter (for testing purposes)
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
extern int h, m;
void WebServerClass::handleM()
{
	if(++m>59) m = 0;
	this->server->send(200, "text/plain", "OK");
}

//---------------------------------------------------------------------------------------
// handleH
//
// Handles the /h request, increments the hours counter (for testing purposes)
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void WebServerClass::handleH()
{
	if(++h>23) h = 0;
	this->server->send(200, "text/plain", "OK");
}

//---------------------------------------------------------------------------------------
// handleR
//
// Handles the /r request, sets LED matrix to all red (for testing purposes)
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void WebServerClass::handleR()
{
	LED.setMode(DisplayMode::red);
	this->server->send(200, "text/plain", "OK");
}

//---------------------------------------------------------------------------------------
// handleG
//
// Handles the /g request, sets LED matrix to all green (for testing purposes)
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void WebServerClass::handleG()
{
	LED.setMode(DisplayMode::green);
	this->server->send(200, "text/plain", "OK");
}

//---------------------------------------------------------------------------------------
// handleB
//
// Handles the /b request, sets LED matrix to all blue (for testing purposes)
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void WebServerClass::handleB()
{
	LED.setMode(DisplayMode::blue);
	this->server->send(200, "text/plain", "OK");
}

void WebServerClass::handleDebug()
{
	if(this->server->hasArg("led") &&
			   this->server->hasArg("r") &&
			   this->server->hasArg("g") &&
			   this->server->hasArg("b"))
	{
		int led = this->server->arg("led").toInt();
		int r = this->server->arg("r").toInt();
		int g = this->server->arg("g").toInt();
		int b = this->server->arg("b").toInt();
		if(led < 0) led = 0;
		if(led >= NUM_PIXELS) led = NUM_PIXELS - 1;
		if(r < 0) r = 0;
		if(r > 255) r = 255;
		if(g < 0) g = 0;
		if(g > 255) g = 255;
		if(b < 0) b = 0;
		if(b > 255) b = 255;

		LED.currentValues[led*3+0] = r;
		LED.currentValues[led*3+1] = g;
		LED.currentValues[led*3+2] = b;
		LED.show();
		Config.debugMode = 1;
	}

	if(this->server->hasArg("clear"))
	{
		for(int i=0; i<3*NUM_PIXELS; i++) LED.currentValues[i] = 0;
		LED.show();
	}

	if(this->server->hasArg("end"))
	{
		Config.debugMode = 0;
	}
	this->server->send(200, "text/plain", "OK");
}

void WebServerClass::handleGetADC()
{
	int __attribute__ ((unused)) temp = Brightness.value(); // to trigger A/D conversion
	this->server->send(200, "text/plain", String(Brightness.avg));
}

void WebServerClass::handleSetVar()
{
	bool mustSave = false;
	char* err = NULL;
	if(this->server->hasArg("value") && this->server->hasArg("name")) {
		Serial.println("WebServerClass::handleSetVar(): " + this->server->arg("name") + " = " + this->server->arg("value"));
		if(this->server->arg("name") == "itIs" ) {
			if(this->server->arg("value") == "0" || this->server->arg("value") == "false") Config.showItIs = false; else Config.showItIs = true;
			mustSave = true;
		} else
		if(this->server->arg("name") == "autoOnOff" ) {
			if(this->server->arg("value") == "0"|| this->server->arg("value") == "false") {
				Config.autoOnOff = false;
				LED.setDisplayOn(true);
			} else {
				Config.autoOnOff = true;
			}
			mustSave = true;
		} else
		if(this->server->arg("name") == "autoOn" ) {
			uint8_t h,m;
			int res = sscanf(this->server->arg("value").c_str(), "%02d:%02d", &h, &m);
			if( res == 2 ) {
				Config.autoOnHour = h;
				Config.autoOnMin = m;
				mustSave = true;
			} else {
				err = "text/plain", "ERR: bad time format, must be HH:MM";
			}
		} else	
		if(this->server->arg("name") == "autoOff" ) {
			uint8_t h,m;
			int res = sscanf(this->server->arg("value").c_str(), "%02d:%02d", &h, &m);
			if( res == 2 ) {
				Config.autoOffHour = h;
				Config.autoOffMin = m;
				mustSave = true;
			} else {
				err =  "ERR: bad time format, must be HH:MM";
			}
		} else		
		if(this->server->arg("name") == "rainbow" ) {
			if(this->server->arg("value") == "0"|| this->server->arg("value") == "false") {
				Config.fgRainbow = false; 
			} else {
				Config.fgRainbow = true;
				LED.resetRainbowColor();
			}
			mustSave = true;
		} else
		if(this->server->arg("name") == "minuteType" ) {
			if(this->server->arg("value") == "0") Config.minuteType = 0; else Config.minuteType = 1;
			mustSave = true;
		} else
		if(this->server->arg("name") == "brightness" ) {
			int v = this->server->arg("value").toInt();
			if(v < 0 || v > 257) {
				err =  "ERR: brightness not in range 0..256";
			} else {
				Brightness.brightnessOverride = v;
				mustSave = true;
			}
		} else
		if(this->server->arg("name") == "rainbowSpeed" ) {
			int v = this->server->arg("value").toInt();
			if(v < 0 || v > 2)
			{
				err =  "ERR: rainbowSpeed not in range 0..2";
			}
			else
			{
				Config.rainbowSpeed = v;
				mustSave = true;
			}
		} else
		if(this->server->arg("name") == "timezone" ) {
			int newTimeZone = this->server->arg("value").toInt();
			if(newTimeZone < - 12 || newTimeZone > 14)
			{
				err = "ERR: timezone not in range -12 ... 14";
			}
			else
			{
				Config.timeZone = newTimeZone;
				NTP.setTimeZone(Config.timeZone);
				mustSave = true;
			}
		} else
		if(this->server->arg("name") == "ntpserver" ) {
			IPAddress ip;
			if (ip.fromString(this->server->arg("ip")))
			{
				Config.ntpserver = ip;
				// set IP address in client
				NTP.setServer(ip);
				mustSave = true;
			} else {
				err = "ERR: bad ip adress format, must be x.x.x.x";
			}
		} else
		if(this->server->arg("name") == "heartbeat" ) {
			if(this->server->arg("value") == "0") Config.heartbeat = 0; else Config.heartbeat = 1;
			mustSave = true;
		} else
		if(this->server->arg("name") == "tmpl" ) {
			int v = this->server->arg("value").toInt();
			if(v < 0 || v > 2)
			{
				err =  "ERR: tmpl not in range 0..2";
			}
			else
			{
				Config.tmpl = v;
				mustSave = true;
			}
		} else
		if( this->server->arg("name") == "fg" ){
			this->extractColor("value", Config.fg);
			mustSave = true;
		}	 else
		if( this->server->arg("name") == "s" ){
			this->extractColor("value", Config.s);
			mustSave = true;
		} else
		if( this->server->arg("name") == "bg" ){
			this->extractColor("value", Config.bg);
			mustSave = true;
		} else
		if(this->server->arg("name") == "displaymode" ) {
			int newMode = this->server->arg("value").toInt();
			if( newMode >= 0 && newMode <= MAX_DISPLAY_MODE_TO_SET ) {
				DisplayMode mode = (DisplayMode)newMode;
				LED.setMode(mode);
				Config.defaultMode = mode;
				mustSave = true;
			} else {
				err =  "ERR: displaymode not in range 0..?";
			}
		} else
		if(this->server->arg("name") == "fillMode" ) {
			int newMode = this->server->arg("value").toInt();
			if( newMode >= 0 && newMode <= 4 ) {
				Config.fillMode = newMode;
				mustSave = true;
			} else {
				err =  "ERR: fillMode not in range 0..?";
			}
		} else {
				err =  "ERR: var name not valid";
		}
	}
	if( mustSave ) {
		Config.save();
		this->server->send(200, "text/plain", "OK");
	} else {
			this->server->send(400, "text/plain", err ? err:"unknown error");
	}
}

//---------------------------------------------------------------------------------------
// handleNotFound
//
// Handles all requests not bound to other handlers, tries to serve a file if found in
// flash, responds with 404 otherwise
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void WebServerClass::handleNotFound()
{
	// first, try to serve the requested file from flash
	if (!serveFile(this->server->uri()))
	{
		// create 404 message if no file was found for this URI
		String message = "File Not Found\n\n";
		message += "URI: ";
		message += this->server->uri();
		message += "\nMethod: ";
		message += (this->server->method() == HTTP_GET) ? "GET" : "POST";
		message += "\nArguments: ";
		message += this->server->args();
		message += "\n";
		for (uint8_t i = 0; i < this->server->args(); i++)
		{
			message += " " + this->server->argName(i) + ": "
					+ this->server->arg(i) + "\n";
		}
		this->server->send(404, "text/plain", message);
	}
}

void WebServerClass::handleGetConfig() {
	StaticJsonBuffer<1024> jsonBuffer;
	char buf[1024];
	JsonObject& json = jsonBuffer.createObject();
	json["ntpserver"] = Config.ntpserver.toString();
	json["heartbeat"] = Config.heartbeat;
	json["itIs"] = Config.showItIs;
	json["rainbow"] = Config.fgRainbow;
	json["minuteType"] = Config.minuteType;
	json["rainbowSpeed"] = Config.rainbowSpeed;
	json["autoOnOff"] = Config.autoOnOff;
	json["timezone"] = Config.timeZone;
	json["brightness"] = Brightness.brightnessOverride;
	char autoOn[9];
	sprintf(autoOn,"%02d:%02d", Config.autoOnHour, Config.autoOnMin);
	json["autoOn"] = autoOn;
	char autoOff[9];
	sprintf(autoOff,"%02d:%02d", Config.autoOffHour, Config.autoOffMin);
	json["autoOff"] = autoOff;
	json["tmpl"] = Config.tmpl;
	json["displaymode"] = (int)Config.defaultMode;
	json["fillMode"] = Config.fillMode;
	char fg[9];
	sprintf(fg,"#%02x%02x%02x", Config.fg.r, Config.fg.g, Config.fg.b);
	json["fg"] = fg;
	char bg[9];
	sprintf(bg,"#%02x%02x%02x", Config.bg.r, Config.bg.g, Config.bg.b);
	json["bg"] = bg;
	char s[9];
	sprintf(s,"#%02x%02x%02x", Config.s.r, Config.s.g, Config.s.b);
	json["s"] = s;
	json.printTo(buf, sizeof(buf));
	this->server->send(200, "application/json", buf);
}

//---------------------------------------------------------------------------------------
// handleInfo
//
// Handles requests to "/info", replies with JSON structure containing system status
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void WebServerClass::handleInfo()
{
	StaticJsonBuffer<512> jsonBuffer;
	char buf[512];
	JsonObject& json = jsonBuffer.createObject();
	json["heap"] = ESP.getFreeHeap();
	json["sketchsize"] = ESP.getSketchSize();
	json["sketchspace"] = ESP.getFreeSketchSpace();
	json["cpufrequency"] = ESP.getCpuFreqMHz();
	json["chipid"] = ESP.getChipId();
	json["sdkversion"] = ESP.getSdkVersion();
	json["bootversion"] = ESP.getBootVersion();
	json["bootmode"] = ESP.getBootMode();
	json["flashid"] = ESP.getFlashChipId();
	json["flashspeed"] = ESP.getFlashChipSpeed();
	json["flashsize"] = ESP.getFlashChipRealSize();
	json["resetreason"] = ESP.getResetReason();
	json["resetinfo"] = ESP.getResetInfo();
//	switch(LED.getMode())
//	{
//	case DisplayMode::plain:
//		json["mode"] = "plain"; break;
//	case DisplayMode::fade:
//		json["mode"] = "fade"; break;
//	case DisplayMode::flyingLettersVertical:
//		json["mode"] = "flyingLettersVertical"; break;
//	case DisplayMode::matrix:
//		json["mode"] = "matrix"; break;
//	case DisplayMode::heart:
//		json["mode"] = "heart"; break;
//	case DisplayMode::stars:
//		json["mode"] = "stars"; break;
//	case DisplayMode::red:
//		json["mode"] = "red"; break;
//	case DisplayMode::green:
//		json["mode"] = "green"; break;
//	case DisplayMode::blue:
//		json["mode"] = "blue"; break;
//	case DisplayMode::yellowHourglass:
//		json["mode"] = "yellowHourglass"; break;
//	case DisplayMode::greenHourglass:
//		json["mode"] = "greenHourglass"; break;
//	case DisplayMode::update:
//		json["mode"] = "update"; break;
//	case DisplayMode::updateComplete:
//		json["mode"] = "updateComplete"; break;
//	case DisplayMode::updateError:
//		json["mode"] = "updateError"; break;
//	case DisplayMode::wifiManager:
//		json["mode"] = "wifiManager"; break;
//	default:
//		json["mode"] = "unknown"; break;
//	}

	json.printTo(buf, sizeof(buf));
	this->server->send(200, "application/json", buf);
}

//---------------------------------------------------------------------------------------
// extractColor
//
// Converts the given web server argument to a color struct
// -> argName: Name of the web server argument
//	result: Pointer to palette_entry struct to receive result
// <- --
//---------------------------------------------------------------------------------------
void WebServerClass::extractColor(String argName, palette_entry& result)
{
	char c[3];

	if (this->server->hasArg(argName) && this->server->arg(argName).length() == 6)
	{
		String color = this->server->arg(argName);
		color.substring(0, 2).toCharArray(c, sizeof(c));
		result.r = strtol(c, NULL, 16);
		color.substring(2, 4).toCharArray(c, sizeof(c));
		result.g = strtol(c, NULL, 16);
		color.substring(4, 6).toCharArray(c, sizeof(c));
		result.b = strtol(c, NULL, 16);
	}
}

//---------------------------------------------------------------------------------------
// handleSaveConfig
//
// Saves the current configuration to EEPROM
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void WebServerClass::handleSaveConfig()
{
	Config.save();
	this->server->send(200, "text/plain", "OK");
}

//---------------------------------------------------------------------------------------
// handleLoadConfig
//
// Loads the current configuration from EEPROM
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void WebServerClass::handleLoadConfig()
{
	Config.load();
	this->server->send(200, "text/plain", "OK");
}

//---------------------------------------------------------------------------------------
// handleGetColors
//
// Outputs the currently active colors as comma separated list for background, foreground
// and seconds color with 3 values each (red, green, blue)
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void WebServerClass::handleGetColors()
{
	String message = String(Config.bg.r) + "," + String(Config.bg.g) + ","
			+ String(Config.bg.b) + "," + String(Config.fg.r) + ","
			+ String(Config.fg.g) + "," + String(Config.fg.b) + ","
			+ String(Config.s.r) + "," + String(Config.s.g) + ","
			+ String(Config.s.b);
	this->server->send(200, "text/plain", message);
}
