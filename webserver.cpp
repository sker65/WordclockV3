// ESP8266 Wordclock
// Copyright (C) 2016 Thoralt Franz, https://github.com/thoralt
// also (C) 2021 by Stefan Rinke, https://github.com/sker65
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
#include <stdio.h>

#include "brightness.h"
#include "ledfunctions.h"
#include "ntp.h"
#include "webserver.h"

//---------------------------------------------------------------------------------------
// global instance
//---------------------------------------------------------------------------------------
WebServer HttpServer = WebServer();

const char* WebServer::textPlain = "text/plain";
const char* WebServer::applicationJson = "application/json";

//---------------------------------------------------------------------------------------
// WebServer
//
// Constructor, currently empty
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
WebServer::WebServer() {}

//---------------------------------------------------------------------------------------
// ~WebServer
//
// Destructor, removes allocated web server object
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
WebServer::~WebServer() {
	if( this->server )
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
void WebServer::begin() {
	SPIFFS.begin();

	this->server = new ESP8266WebServer( 80 );

	this->server->on( "/info", std::bind( &WebServer::handleInfo, this ) );
	this->server->on( "/saveconfig", std::bind( &WebServer::handleSaveConfig, this ) );
	this->server->on( "/loadconfig", std::bind( &WebServer::handleLoadConfig, this ) );
	this->server->on( "/config", std::bind( &WebServer::handleGetConfig, this ) );
	this->server->on( "/d", std::bind( &WebServer::handleD, this ) );
	this->server->on( "/h", std::bind( &WebServer::handleH, this ) );
	this->server->on( "/m", std::bind( &WebServer::handleM, this ) );
	this->server->on( "/r", std::bind( &WebServer::handleR, this ) );
	this->server->on( "/g", std::bind( &WebServer::handleG, this ) );
	this->server->on( "/b", std::bind( &WebServer::handleB, this ) );
	this->server->on( "/getadc", std::bind( &WebServer::handleGetADC, this ) );
	this->server->on( "/setvar", std::bind( &WebServer::handleSetVar, this ) );
	this->server->on( "/debug", std::bind( &WebServer::handleDebug, this ) );

	this->server->onNotFound( std::bind( &WebServer::handleNotFound, this ) );
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
void WebServer::process() { this->server->handleClient(); }

//---------------------------------------------------------------------------------------
// serveFile
//
// Looks up a given file name in internal flash file system, streams the file if found
//
// -> path: name of the file; "index.html" will be added if name ends with "/"
// <- true: file was found and served to client
//	false: file not found
//---------------------------------------------------------------------------------------
bool WebServer::serveFile( String path ) {
	Serial.println( "WebServer::serveFile(): " + path );
	if( path.endsWith( "/" ) )
		path += "index.html";
	if( SPIFFS.exists( path ) ) {
		File file = SPIFFS.open( path, "r" );
		this->server->streamFile( file, this->contentType( path ) );
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
String WebServer::contentType( String filename ) {
	if( this->server->hasArg( "download" ) )
		return "application/octet-stream";
	else if( filename.endsWith( ".htm" ) )
		return "text/html";
	else if( filename.endsWith( ".html" ) )
		return "text/html";
	else if( filename.endsWith( ".css" ) )
		return "text/css";
	else if( filename.endsWith( ".js" ) )
		return "application/javascript";
	else if( filename.endsWith( ".png" ) )
		return "image/png";
	else if( filename.endsWith( ".gif" ) )
		return "image/gif";
	else if( filename.endsWith( ".jpg" ) )
		return "image/jpeg";
	else if( filename.endsWith( ".ico" ) )
		return "image/x-icon";
	else if( filename.endsWith( ".xml" ) )
		return "text/xml";
	else if( filename.endsWith( ".pdf" ) )
		return "application/x-pdf";
	else if( filename.endsWith( ".zip" ) )
		return "application/x-zip";
	else if( filename.endsWith( ".gz" ) )
		return "application/x-gzip";
	return textPlain;
}

//---------------------------------------------------------------------------------------
// handleM
//
// Handles the /m request, increments the minutes counter (for testing purposes)
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
extern int h, m, day, month, year;

void WebServer::handleM() {
	if( ++m > 59 )
		m = 0;
	this->server->send( 200, textPlain, "OK" );
}

// debug handler to also test date increments (moon phase)
void WebServer::handleD() {
	day += 1;
	switch( month ) {
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		if( day == 32 ) {
			day = 1;
			month++;
		}
		break;
	case 2:
		if( day == 29 ) {
			day = 1;
			month++;
		}
		break;
	default:
		if( day == 31 ) {
			day = 1;
			month++;
		}
		break;
	}
	if( month == 13 ) {
		month = 1;
		year++;
	}
	this->server->send( 200, textPlain, "OK" );
}

//---------------------------------------------------------------------------------------
// handleH
//
// Handles the /h request, increments the hours counter (for testing purposes)
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void WebServer::handleH() {
	if( ++h > 23 )
		h = 0;
	this->server->send( 200, textPlain, "OK" );
}

//---------------------------------------------------------------------------------------
// handleR
//
// Handles the /r request, sets LED matrix to all red (for testing purposes)
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void WebServer::handleR() {
	LED.setMode( DisplayMode::red );
	this->server->send( 200, textPlain, "OK" );
}

//---------------------------------------------------------------------------------------
// handleG
//
// Handles the /g request, sets LED matrix to all green (for testing purposes)
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void WebServer::handleG() {
	LED.setMode( DisplayMode::green );
	this->server->send( 200, textPlain, "OK" );
}

//---------------------------------------------------------------------------------------
// handleB
//
// Handles the /b request, sets LED matrix to all blue (for testing purposes)
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void WebServer::handleB() {
	LED.setMode( DisplayMode::blue );
	this->server->send( 200, textPlain, "OK" );
}

void WebServer::handleDebug() {
	if( this->server->hasArg( "led" ) && this->server->hasArg( "r" ) && this->server->hasArg( "g" ) &&
	    this->server->hasArg( "b" ) ) {
		int led = this->server->arg( "led" ).toInt();
		int r = this->server->arg( "r" ).toInt();
		int g = this->server->arg( "g" ).toInt();
		int b = this->server->arg( "b" ).toInt();
		if( led < 0 )
			led = 0;
		if( led >= NUM_PIXELS )
			led = NUM_PIXELS - 1;
		if( r < 0 )
			r = 0;
		if( r > 255 )
			r = 255;
		if( g < 0 )
			g = 0;
		if( g > 255 )
			g = 255;
		if( b < 0 )
			b = 0;
		if( b > 255 )
			b = 255;

		LED.currentValues[led * 3 + 0] = r;
		LED.currentValues[led * 3 + 1] = g;
		LED.currentValues[led * 3 + 2] = b;
		LED.show();
		Config.debugMode = 1;
	}

	if( this->server->hasArg( "clear" ) ) {
		for( int i = 0; i < 3 * NUM_PIXELS; i++ )
			LED.currentValues[i] = 0;
		LED.show();
	}

	if( this->server->hasArg( "end" ) ) {
		Config.debugMode = 0;
	}
	this->server->send( 200, textPlain, "OK" );
}

void WebServer::handleGetADC() {
	int __attribute__( ( unused ) ) temp = Brightness.value(); // to trigger A/D conversion
	this->server->send( 200, textPlain, String( Brightness.avg ) );
}

void WebServer::handleSetVar() {
	bool mustSave = false;
	char* err = NULL;
	if( this->server->hasArg( "value" ) && this->server->hasArg( "name" ) ) {
		Serial.println( "WebServer::handleSetVar(): " + this->server->arg( "name" ) + " = " +
		                this->server->arg( "value" ) );
		if( this->server->arg( "name" ) == "itIs" ) {
			if( this->server->arg( "value" ) == "0" || this->server->arg( "value" ) == "false" )
				Config.showItIs = false;
			else
				Config.showItIs = true;
			mustSave = true;
		} else if( this->server->arg( "name" ) == "autoOnOff" ) {
			if( this->server->arg( "value" ) == "0" || this->server->arg( "value" ) == "false" ) {
				Config.autoOnOff = false;
				LED.setDisplayOn( true );
			} else {
				Config.autoOnOff = true;
			}
			mustSave = true;
		} else if( this->server->arg( "name" ) == "autoOn" ) {
			uint8_t h, m;
			int res = sscanf( this->server->arg( "value" ).c_str(), "%02d:%02d", &h, &m );
			if( res == 2 ) {
				Config.autoOnHour = h;
				Config.autoOnMin = m;
				mustSave = true;
			} else {
				err = "ERR: bad time format, must be HH:MM";
			}
		} else if( this->server->arg( "name" ) == "autoOff" ) {
			uint8_t h, m;
			int res = sscanf( this->server->arg( "value" ).c_str(), "%02d:%02d", &h, &m );
			if( res == 2 ) {
				Config.autoOffHour = h;
				Config.autoOffMin = m;
				mustSave = true;
			} else {
				err = "ERR: bad time format, must be HH:MM";
			}
		} else if( this->server->arg( "name" ) == "rainbow" ) {
			if( this->server->arg( "value" ) == "0" || this->server->arg( "value" ) == "false" ) {
				Config.fgRainbow = false;
			} else {
				Config.fgRainbow = true;
				LED.resetRainbowColor();
			}
			mustSave = true;
		} else if( this->server->arg( "name" ) == "minuteType" ) {
			if( this->server->arg( "value" ) == "0" )
				Config.minuteType = 0;
			else
				Config.minuteType = 1;
			mustSave = true;
		} else if( this->server->arg( "name" ) == "brightness" ) {
			int v = this->server->arg( "value" ).toInt();
			if( v < 0 || v > 257 ) {
				err = "ERR: brightness not in range 0..256";
			} else {
				Brightness.brightnessOverride = v;
				mustSave = true;
			}
		} else if( this->server->arg( "name" ) == "rainbowSpeed" ) {
			int v = this->server->arg( "value" ).toInt();
			if( v < 0 || v > 2 ) {
				err = "ERR: rainbowSpeed not in range 0..2";
			} else {
				Config.rainbowSpeed = v;
				mustSave = true;
			}
		} else if( this->server->arg( "name" ) == "timezone" ) {
			int newTimeZone = this->server->arg( "value" ).toInt();
			if( newTimeZone < -12 || newTimeZone > 14 ) {
				err = "ERR: timezone not in range -12 ... 14";
			} else {
				Config.timeZone = newTimeZone;
				NTP.setTimeZone( Config.timeZone );
				mustSave = true;
			}
		} else if( this->server->arg( "name" ) == "ntpserver" ) {
			IPAddress ip;
			if( ip.fromString( this->server->arg( "ip" ) ) ) {
				Config.ntpserver = ip;
				// set IP address in client
				NTP.setServer( ip );
				mustSave = true;
			} else {
				err = "ERR: bad ip adress format, must be x.x.x.x";
			}
		} else if( this->server->arg( "name" ) == "heartbeat" ) {
			if( this->server->arg( "value" ) == "0" )
				Config.heartbeat = 0;
			else
				Config.heartbeat = 1;
			mustSave = true;
		} else if( this->server->arg( "name" ) == "tmpl" ) {
			int v = this->server->arg( "value" ).toInt();
			if( v < 0 || v > 2 ) {
				err = "ERR: tmpl not in range 0..2";
			} else {
				Config.tmpl = v;
				mustSave = true;
			}
		} else if( this->server->arg( "name" ) == "fg" ) {
			this->extractColor( "value", Config.fg );
			mustSave = true;
		} else if( this->server->arg( "name" ) == "s" ) {
			this->extractColor( "value", Config.s );
			mustSave = true;
		} else if( this->server->arg( "name" ) == "bg" ) {
			this->extractColor( "value", Config.bg );
			mustSave = true;
		} else if( this->server->arg( "name" ) == "displaymode" ) {
			int newMode = this->server->arg( "value" ).toInt();
			if( newMode >= 0 && newMode <= MAX_DISPLAY_MODE_TO_SET ) {
				DisplayMode mode = (DisplayMode)newMode;
				LED.setMode( mode );
				Config.defaultMode = mode;
				mustSave = true;
			} else {
				err = "ERR: displaymode not in range 0..?";
			}
		} else if( this->server->arg( "name" ) == "fillMode" ) {
			int newMode = this->server->arg( "value" ).toInt();
			if( newMode >= 0 && newMode <= 4 ) {
				Config.fillMode = newMode;
				mustSave = true;
			} else {
				err = "ERR: fillMode not in range 0..?";
			}
		} else {
			err = "ERR: var name not valid";
		}
	}
	if( mustSave ) {
		Config.save();
		this->server->send( 200, textPlain, "OK" );
	} else {
		this->server->send( 400, textPlain, err ? err : "unknown error" );
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
void WebServer::handleNotFound() {
	// first, try to serve the requested file from flash
	if( !serveFile( this->server->uri() ) ) {
		// create 404 message if no file was found for this URI
		String message = "File Not Found\n\n";
		message += "URI: ";
		message += this->server->uri();
		message += "\nMethod: ";
		message += ( this->server->method() == HTTP_GET ) ? "GET" : "POST";
		message += "\nArguments: ";
		message += this->server->args();
		message += "\n";
		for( uint8_t i = 0; i < this->server->args(); i++ ) {
			message += " " + this->server->argName( i ) + ": " + this->server->arg( i ) + "\n";
		}
		this->server->send( 404, textPlain, message );
	}
}

#define RESPONSE_BUF_SIZE 1024

// responde complete config at once as json object
void WebServer::handleGetConfig() {
	char buf[RESPONSE_BUF_SIZE];
	snprintf( buf, RESPONSE_BUF_SIZE,
	          "{ "
	          "\"ntpserver\": \"%s\", "
	          "\"heartbeat\": %s, "
	          "\"itIs\": %s, "
	          "\"rainbow\": %s, "
	          "\"autoOnOff\": %s, "
	          "\"minuteType\": %i, "
	          "\"rainbowSpeed\": %i, "
	          "\"timezone\": %i, "
	          "\"brightness\": %i, "
	          "\"autoOn\": \"%02d:%02d\", "
	          "\"autoOff\": \"%02d:%02d\", "
	          "\"tmpl\": %i, "
	          "\"displaymode\": %i, "
	          "\"fillMode\": %i, "
	          "\"fg\": \"#%02x%02x%02x\", "
	          "\"bg\": \"#%02x%02x%02x\", "
	          "\"s\": \"#%02x%02x%02x\" "
	          "}",
	          Config.ntpserver.toString().c_str(), Config.heartbeat ? "true" : "false",
	          Config.showItIs ? "true" : "false", Config.fgRainbow ? "true" : "false",
	          Config.autoOnOff ? "true" : "false", Config.minuteType, Config.rainbowSpeed, Config.timeZone,
	          Brightness.brightnessOverride, Config.autoOnHour, Config.autoOnMin, Config.autoOffHour, Config.autoOffMin,
	          Config.tmpl, (int)Config.defaultMode, Config.fillMode, Config.fg.r, Config.fg.g, Config.fg.b, Config.bg.r,
	          Config.bg.g, Config.bg.b, Config.s.r, Config.s.g, Config.s.b );
	Serial.printf( "WebServer::handleConfig %s\r\n", buf );
	this->server->send( 200, applicationJson, buf );
}

//---------------------------------------------------------------------------------------
// handleInfo
//
// Handles requests to "/info", replies with JSON structure containing system status
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void WebServer::handleInfo() {
	char buf[RESPONSE_BUF_SIZE];
	snprintf( buf, RESPONSE_BUF_SIZE,
	          "{"
	          "\"heap\": %i, "
	          "\"sketchsize\": %i, "
	          "\"sketchspace\": %i, "
	          "\"cpufrequency\": %i, "
	          "\"chipid\": %i, "
	          "\"sdkversion\": \"%s\", "
	          "\"bootversion\": %i, "
	          "\"bootmode\": %i, "
	          "\"flashid\": %i, "
	          "\"flashspeed\": %i, "
	          "\"flashsize\": %i, "
	          "\"resetreason\": \"%s\", "
	          "\"resetinfo\": \"%s\" "
	          "}",
	          ESP.getFreeHeap(), ESP.getSketchSize(), ESP.getFreeSketchSpace(), ESP.getCpuFreqMHz(), ESP.getChipId(),
	          ESP.getSdkVersion(), ESP.getBootVersion(), ESP.getBootMode(), ESP.getFlashChipId(), ESP.getFlashChipSpeed(),
	          ESP.getFlashChipRealSize(), ESP.getResetReason().c_str(), ESP.getResetInfo().c_str() );
	Serial.printf( "WebServer::handleInfo %s\r\n", buf );
	this->server->send( 200, applicationJson, buf );
}

//---------------------------------------------------------------------------------------
// extractColor
//
// Converts the given web server argument to a color struct
// -> argName: Name of the web server argument
//	result: Pointer to palette_entry struct to receive result
// <- --
//---------------------------------------------------------------------------------------
void WebServer::extractColor( String argName, palette_entry& result ) {
	char c[3];

	if( this->server->hasArg( argName ) && this->server->arg( argName ).length() == 6 ) {
		String color = this->server->arg( argName );
		color.substring( 0, 2 ).toCharArray( c, sizeof( c ) );
		result.r = strtol( c, NULL, 16 );
		color.substring( 2, 4 ).toCharArray( c, sizeof( c ) );
		result.g = strtol( c, NULL, 16 );
		color.substring( 4, 6 ).toCharArray( c, sizeof( c ) );
		result.b = strtol( c, NULL, 16 );
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
void WebServer::handleSaveConfig() {
	Config.save();
	this->server->send( 200, textPlain, "OK" );
}

//---------------------------------------------------------------------------------------
// handleLoadConfig
//
// Loads the current configuration from EEPROM
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void WebServer::handleLoadConfig() {
	Config.load();
	this->server->send( 200, textPlain, "OK" );
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
void WebServer::handleGetColors() {
	String message = String( Config.bg.r ) + "," + String( Config.bg.g ) + "," + String( Config.bg.b ) + "," +
	                 String( Config.fg.r ) + "," + String( Config.fg.g ) + "," + String( Config.fg.b ) + "," +
	                 String( Config.s.r ) + "," + String( Config.s.g ) + "," + String( Config.s.b );
	this->server->send( 200, textPlain, message );
}
