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

#if (ARDUINO + 1) >= 100
#  include <Arduino.h>
#else
#  include <WProgram.h>
#endif

#define headFlag	30
#define tailFlag	30

void APRS_init();
void APRS_setCallsign(char *call, uint8_t ssid);
void APRS_setDestination(char *call, uint8_t ssid);
void APRS_setPath1(char *call, uint8_t ssid);
void APRS_setPath2(char *call, uint8_t ssid);
void APRS_setPathSize(uint8_t pathSize);
void APRS_setTimeStamp(uint8_t hh, uint8_t mm,uint8_t ss);
void APRS_setLat(char *lat);
void APRS_setLon(char *lon);
void APRS_setGain(uint8_t s);
void APRS_useAlternateSymbolTable(bool use);
void APRS_setSymbol(char sym);
void APRS_PrepeareCallsign();
void APRS_PrepearePath1();
void APRS_PrepearePath2();
void APRS_PrepeareLoc(char *comment);
void APRS_PrepeareStatus(char *msg);
void APRS_sendLoc(char *comment);
void APRS_sendStatus(char *msg);
void APRS_sendpacket();
void APRS_sinus();
void APRS_send_bit(int tempo);
void APRS_sendbyte (unsigned char inbyte);
void APRS_fcsbit(unsigned short tbyte);
void APRS_flipout(void);
void APRS_tcConfigure(int sampleRate);
bool APRS_tcIsSyncing(void);
void APRS_tcStartCounter(void);
void APRS_tcReset(void);
void APRS_tcDisable(void);
void TC5_Handler (void);
