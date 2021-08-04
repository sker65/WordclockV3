// ESP8266 Wordclock
// Copyright (C) 2016 Thoralt Franz, https://github.com/thoralt
//
//  This is the configuration module. It contains methods to load/save the
//  configuration from/to the internal EEPROM (simulated EEPROM in flash).
//  Data is loaded into this->eeprom_data[EEPROM_SIZE] which shares RAM with
//  this->config. Configuration variables are copied to public class members
//  ntpserver, heartbeat, ... where they can be used by other modules. Upon
//  save, the public members are copied back to this->config/this->eeprom_data[]
//  and then written to the EEPROM.
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
#include <EEPROM.h>
#include "config.h"

//---------------------------------------------------------------------------------------
// global instance
//---------------------------------------------------------------------------------------
ConfigClass Config = ConfigClass();

//---------------------------------------------------------------------------------------
// ConfigClass
//
// Constructor, loads default values
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
ConfigClass::ConfigClass()
{
	this->reset();
}

//---------------------------------------------------------------------------------------
// ~ConfigClass
//
// destructor
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
ConfigClass::~ConfigClass()
{
}

//---------------------------------------------------------------------------------------
// begin
//
// Initializes the class and loads current configuration from EEPROM into class members.
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void ConfigClass::begin()
{
	EEPROM.begin(EEPROM_SIZE);
	this->load();
}

//---------------------------------------------------------------------------------------
// saveDelayed
//
// Copies the current class member values to EEPROM buffer and writes it to the EEPROM
// after 10 seconds.
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void ConfigClass::saveDelayed()
{
	this->delayedWriteTimer = 1000; // 10 seconds using 10 ms timer
}

//---------------------------------------------------------------------------------------
// save
//
// Copies the current class member values to EEPROM buffer and writes it to the EEPROM.
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void ConfigClass::save()
{
	this->delayedWriteFlag = false;
	this->config->bg = this->bg;
	this->config->fg = this->fg;
	this->config->s = this->s;
	this->config->timeZone = this->timeZone;
	this->config->heartbeat = this->heartbeat;
	this->config->mode = (uint32_t) this->defaultMode;
	for (int i = 0; i < 4; i++)
		this->config->ntpserver[i] = this->ntpserver[i];

	this->config->showItIs = this->showItIs;
	this->config->minuteType = this->minuteType;
	this->config->fgRainbow = this->fgRainbow;
	
	this->config->rainbowSpeed = this->rainbowSpeed;
	this->config->autoOnOff = this->autoOnOff;
	this->config->autoOnHour = this->autoOnHour;
	this->config->autoOnMin = this->autoOnMin;
	this->config->autoOffHour = this->autoOffHour; 
	this->config->autoOffMin = this->autoOffMin; 
	this->config->tmpl = this->tmpl;

	for (int i = 0; i < EEPROM_SIZE; i++)
		EEPROM.write(i, this->eeprom_data[i]);
	EEPROM.commit();
}


//---------------------------------------------------------------------------------------
// reset
//
// Sets default values in EEPROM buffer and member variables.
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void ConfigClass::reset()
{
	this->config->magic = 0xDEADBEEF;
	this->config->bg =
	{	0, 0, 0};
	this->bg = this->config->bg;

	this->config->fg =
	{	255, 255, 255};
	this->fg = this->config->fg;

	this->config->s =
	{	32, 0, 21};
	this->s = this->config->s;

	this->config->heartbeat = true;
	this->heartbeat = this->config->heartbeat;

	this->defaultMode = DisplayMode::plain;
	this->config->mode = (uint32_t) this->defaultMode;
	this->timeZone = 0;

	this->config->ntpserver[0] = 129;
	this->config->ntpserver[1] = 6;
	this->config->ntpserver[2] = 15;
	this->config->ntpserver[3] = 28;
	this->ntpserver[0] = this->config->ntpserver[0];
	this->ntpserver[1] = this->config->ntpserver[1];
	this->ntpserver[2] = this->config->ntpserver[2];
	this->ntpserver[3] = this->config->ntpserver[3];

	this->config->showItIs = this->showItIs = true;
	this->config->minuteType = this->minuteType = 0;
	this->config->fgRainbow = this->fgRainbow = false;
	
	this->config->rainbowSpeed = 0; this->rainbowSpeed = 0;
	this->config->autoOnOff = this->autoOnOff = false;
	this->config->autoOnHour = this->autoOnHour = 0;
	this->config->autoOnMin = this->autoOnMin = 0;
	this->config->autoOffHour = this->autoOffHour = 0; 
	this->config->autoOffMin = this->autoOffMin = 0; 
	this->config->tmpl = this->tmpl = 0;
}

//---------------------------------------------------------------------------------------
// load
//
// Reads the content of the EEPROM into the EEPROM buffer and copies the values to the
// public member variables. Resets (and saves) the values to their defaults if the
// EEPROM data is not initialized.
//
// -> --
// <- --
//---------------------------------------------------------------------------------------
void ConfigClass::load()
{
	Serial.println("Reading EEPROM config");
	for (int i = 0; i < EEPROM_SIZE; i++)
		this->eeprom_data[i] = EEPROM.read(i);
	if (this->config->magic != 0xDEADBEEF)
	{
		Serial.println("EEPROM config invalid, writing default values");
		this->reset();
		this->save();
	}
	this->bg = this->config->bg;
	this->fg = this->config->fg;
	this->s = this->config->s;
	this->defaultMode = (DisplayMode) this->config->mode;
	this->heartbeat = this->config->heartbeat;
	this->timeZone = this->config->timeZone;
	this->minuteType = this->config->minuteType;
	this->fgRainbow = this->config->fgRainbow;
	for (int i = 0; i < 4; i++)
		this->ntpserver[i] = this->config->ntpserver[i];
	
	this->rainbowSpeed = this->config->rainbowSpeed;
	this->autoOnOff = this->config->autoOnOff;
	this->autoOnHour = this->config->autoOnHour;
	this->autoOnMin = this->config->autoOnMin;
	this->autoOffHour = this->config->autoOffHour; 
	this->autoOffMin = this->config->autoOffMin; 
	this->tmpl = this->config->tmpl;
}
