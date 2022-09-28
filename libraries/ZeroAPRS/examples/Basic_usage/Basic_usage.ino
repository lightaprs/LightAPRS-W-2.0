/*
 * ZoroAPRS is a simple aprs library with DAC for samd21 based arduino boards.
 * The ZoroAPRS library was developed only for LightAPRS hardware.
 * 
 * Copyright (C) 2019 HAKKI CAN (TA2NHP) <hkkcan@gmail.com> www.hakkican.com
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <ZeroAPRS.h>

void setup() {
  SerialUSB.begin(57600);
  APRS_init();  
  APRS_setCallsign("NOCALL", 11);
  APRS_setDestination("APRS", 0); 
  APRS_setPath1("WIDE1", 1);
  APRS_setPath2("WIDE2", 1);  
  APRS_setPathSize(2);
  APRS_useAlternateSymbolTable(false);
  APRS_setSymbol('O');
  APRS_setTimeStamp(0, 0, 0);
  APRS_setLat("4851.20N");
  APRS_setLon("00220.92E");
}

void loop() {
  APRS_sendLoc("Comment Text");
  SerialUSB.println("Location sent.");
  delay(5000);
  APRS_sendStatus("Status Text");
  SerialUSB.println("Status sent.");
  delay(5000);
}
