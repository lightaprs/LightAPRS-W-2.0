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
#include "ZeroAPRS.h"

typedef struct {
  char CALL[7];
  char DST[7];
  char PATH1[7];
  char PATH2[7];
  uint8_t PATH_SIZE=0;
  char TIMESTAMP[7];
  char LAT[9];
  char LON[10];
  char SYMBOLTABLE='/';
  char SYMBOL='>';
  char TRACK[200];
}APRS_Packet;

typedef struct{
    char call[6];
    uint8_t ssid;
} AX25Call;

APRS_Packet APRS;
AX25Call APRS_Call;
AX25Call APRS_Path1;
AX25Call APRS_Path2;

uint8_t flip_freq;
volatile unsigned long ddsAccu;
volatile unsigned long ddsWord;
volatile unsigned long ddsWord0 = 391700860;
volatile unsigned long ddsWord1 = 718118244;
volatile unsigned long ddsWord2 = 277454776;
volatile int sinusPtr;
volatile int countPtr;
unsigned char stuff, flag, fcsflag;
unsigned short crc;
bool state = 0;
uint8_t gain = 4;

const static uint8_t sinusTable[512] PROGMEM = {128, 129, 131, 132, 134, 135, 137, 138, 140, 141, 143, 145, 146, 148, 149, 151, 152, 154, 155, 157,
                                                158, 160, 161, 163, 164, 166, 167, 169, 170, 172, 173, 175, 176, 178, 179, 180, 182, 183, 185, 186,
                                                187, 189, 190, 191, 193, 194, 195, 197, 198, 199, 201, 202, 203, 204, 206, 207, 208, 209, 210, 212,
                                                213, 214, 215, 216, 217, 218, 219, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 230, 231, 232,
                                                233, 234, 235, 236, 236, 237, 238, 239, 240, 240, 241, 242, 242, 243, 244, 244, 245, 245, 246, 247,
                                                247, 248, 248, 249, 249, 249, 250, 250, 251, 251, 251, 252, 252, 252, 253, 253, 253, 253, 254, 254,
                                                254, 254, 254, 254, 254, 254, 254, 254, 255, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 253,
                                                253, 253, 253, 252, 252, 252, 251, 251, 251, 250, 250, 249, 249, 249, 248, 248, 247, 247, 246, 245,
                                                245, 244, 244, 243, 242, 242, 241, 240, 240, 239, 238, 237, 236, 236, 235, 234, 233, 232, 231, 230,
                                                230, 229, 228, 227, 226, 225, 224, 223, 222, 221, 219, 218, 217, 216, 215, 214, 213, 212, 210, 209,
                                                208, 207, 206, 204, 203, 202, 201, 199, 198, 197, 195, 194, 193, 191, 190, 189, 187, 186, 185, 183,
                                                182, 180, 179, 178, 176, 175, 173, 172, 170, 169, 167, 166, 164, 163, 161, 160, 158, 157, 155, 154,
                                                152, 151, 149, 148, 146, 145, 143, 141, 140, 138, 137, 135, 134, 132, 131, 129, 127, 126, 124, 123,
                                                121, 120, 118, 117, 115, 114, 112, 110, 109, 107, 106, 104, 103, 101, 100, 98, 97, 95, 94, 92, 91, 
                                                89, 88, 86, 85, 83, 82, 80, 79, 77, 76, 75, 73, 72, 70, 69, 68, 66, 65, 64, 62, 61, 60, 58, 57, 56,
                                                54, 53, 52, 51, 49, 48, 47, 46, 45, 43, 42, 41, 40, 39, 38, 37, 36, 34, 33, 32, 31, 30, 29, 28, 27, 
                                                26, 25, 25, 24, 23, 22, 21, 20, 19, 19, 18, 17, 16, 15, 15, 14, 13, 13, 12, 11, 11, 10, 10, 9, 8, 8,
                                                7, 7, 6, 6, 6, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                                1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 6, 7, 7, 8, 8, 9, 10, 10, 11, 11, 12, 13, 
                                                13, 14, 15, 15, 16, 17, 18, 19, 19, 20, 21, 22, 23, 24, 25, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 
                                                36, 37, 38, 39, 40, 41, 42, 43, 45, 46, 47, 48, 49, 51, 52, 53, 54, 56, 57, 58, 60, 61, 62, 64, 65, 
                                                66, 68, 69, 70, 72, 73, 75, 76, 77, 79, 80, 82, 83, 85, 86, 88, 89, 91, 92, 94, 95, 97, 98, 100, 101, 
                                                103, 104, 106, 107, 109, 110, 112, 114, 115, 117, 118, 120, 121, 123, 124, 126};

void APRS_init() {
  while (DAC->STATUS.bit.SYNCBUSY);
  DAC->DATA.reg = 0; //DAC out 0
  while (DAC->STATUS.bit.SYNCBUSY);
  DAC->CTRLA.bit.ENABLE = 0x01; //DAC Enable
  APRS_tcConfigure(13200);
  sinusPtr = 0;
  countPtr = 0;
}

void APRS_setGain(uint8_t s) {
    if (s > 0 && s < 5) {//1-4
        gain = s;
    }
}

void APRS_setCallsign(char *call, uint8_t ssid) {
  memset(APRS_Call.call, 0, 6);
  sprintf(APRS_Call.call, "%s", call);
  APRS_Call.ssid=ssid;
}

void APRS_setDestination(char *call, uint8_t ssid) {
  memset(APRS.DST, 0, 7);
  for (int i = 0; i < 6; i++) {
    if (strlen(call)<=i )
      APRS.DST[i] = ' '  << 1;
    else      
      APRS.DST[i] = call[i] << 1;
  }
 APRS.DST[6] = ('0' + ssid) << 1;
}

void APRS_setPath1(char *call, uint8_t ssid) {
  memset(APRS_Path1.call, 0, 6);
  sprintf(APRS_Path1.call, "%s", call);
  APRS_Path1.ssid=ssid;
}

void APRS_setPath2(char *call, uint8_t ssid) {
  memset(APRS_Path2.call, 0, 6);
  sprintf(APRS_Path2.call, "%s", call);
  APRS_Path2.ssid=ssid;
}

void APRS_setPathSize(uint8_t pathSize) {
  APRS.PATH_SIZE = pathSize;
}

void APRS_setTimeStamp(uint8_t hh, uint8_t mm, uint8_t ss) {
  memset(APRS.TIMESTAMP, 0, 7);
  sprintf(APRS.TIMESTAMP, "%02d%02d%02d", hh, mm, ss);
  APRS.TIMESTAMP[6] = 'h';
}

void APRS_setLat(char *lat) {
  memset(APRS.LAT, 0, 9);
  int i = 0;
  while (i < 8 && lat[i] != 0) {
    APRS.LAT[i] = lat[i];
    i++;
  }
}

void APRS_setLon(char *lon) {
  memset(APRS.LON, 0, 10);
  int i = 0;
  while (i < 9 && lon[i] != 0) {
    APRS.LON[i] = lon[i];
    i++;
  }
}

void APRS_useAlternateSymbolTable(bool use) {
  if (use) {
    APRS.SYMBOLTABLE = '\\';
  } else {
    APRS.SYMBOLTABLE = '/';
  }
}

void APRS_setSymbol(char sym) {
  APRS.SYMBOL = sym;
}

void APRS_PrepeareCallsign() {
  memset(APRS.CALL, 0, 7);
  for (int i = 0; i <6; i++) {
    if (strlen(APRS_Call.call)<=i)
       APRS.CALL[i] = ' ' << 1;
    else
    APRS.CALL[i] = APRS_Call.call[i] << 1;
     
  }
  if (APRS.PATH_SIZE==0){
    APRS.CALL[6] = ('0' + APRS_Call.ssid) << 1 | 1; 
  }else{
    APRS.CALL[6] = ('0' + APRS_Call.ssid) << 1;   
 }
}

void APRS_PrepearePath1() {
  memset(APRS.PATH1, 0, 7);
  for (int i = 0 ; i < 6; i++) {
    if (strlen(APRS_Path1.call)<=i )
      APRS.PATH1[i] = ' '  << 1;
    else
    APRS.PATH1[i] = APRS_Path1.call[i] << 1;      
  }

 if (APRS.PATH_SIZE==1){
    APRS.PATH1[6] = ('0' + APRS_Path1.ssid) << 1 | 1; 
  }else{
    APRS.PATH1[6] = ('0' + APRS_Path1.ssid) << 1;   
 }
}

void APRS_PrepearePath2() {
  memset(APRS.PATH2, 0, 7);
  for (int i = 0; i <= 6; i++) {
    if (strlen(APRS_Path2.call)<=i )
    APRS.PATH2[i] = ' '  << 1;  
    else
      APRS.PATH2[i] = APRS_Path2.call[i] << 1;
  }
    APRS.PATH2[6] = ('0' + APRS_Path2.ssid) << 1 | 1;  
}

void APRS_PrepeareLoc(char *comment) {
  uint8_t pos = 0;
  memset(APRS.TRACK, 0, 200); 
  APRS_PrepeareCallsign(); 
  APRS_PrepearePath1();
  APRS_PrepearePath2();
  sprintf(APRS.TRACK + pos, "%s", APRS.DST); pos += 7;
  sprintf(APRS.TRACK + pos, "%s", APRS.CALL); pos += 7;
  if (APRS.PATH_SIZE > 0) {
    sprintf(APRS.TRACK + pos, "%s", APRS.PATH1);
    pos += 7;
  }
  if (APRS.PATH_SIZE > 1) {
    sprintf(APRS.TRACK + pos, "%s", APRS.PATH2);
    pos += 7;
  }
  APRS.TRACK[pos++] = 0x03;
  APRS.TRACK[pos++] = 0xf0;
  if (APRS.TIMESTAMP[0] != '\0') {
	APRS.TRACK[pos++] = '/';  
    sprintf(APRS.TRACK + pos, "%s", APRS.TIMESTAMP);
    pos += 7;
  }else APRS.TRACK[pos++] = '!';
  sprintf(APRS.TRACK + pos, "%s", APRS.LAT); pos += 8;
  APRS.TRACK[pos++] = APRS.SYMBOLTABLE;
  sprintf(APRS.TRACK + pos, "%s", APRS.LON); pos += 9;  
  APRS.TRACK[pos++] = APRS.SYMBOL;
  
  sprintf(APRS.TRACK + pos, "%s", comment);
  //SerialUSB.println(APRS.TRACK);
  //SerialUSB.flush();
}

void APRS_PrepeareStatus(char *msg){
  uint8_t pos = 0;
  memset(APRS.TRACK, 0, 200); 
  APRS_PrepeareCallsign(); 
  APRS_PrepearePath1();
  APRS_PrepearePath2();
  sprintf(APRS.TRACK + pos, "%s", APRS.DST); pos += 7;
  sprintf(APRS.TRACK + pos, "%s", APRS.CALL); pos += 7;
  if (APRS.PATH_SIZE > 0) {
    sprintf(APRS.TRACK + pos, "%s", APRS.PATH1);
    pos += 7;
  }
  if (APRS.PATH_SIZE > 1) {
    sprintf(APRS.TRACK + pos, "%s", APRS.PATH2);
    pos += 7;
  }
  APRS.TRACK[pos++] = 0x03;
  APRS.TRACK[pos++] = 0xf0;
  APRS.TRACK[pos++] = '>';
  sprintf(APRS.TRACK + pos, "%s", msg);
  //SerialUSB.println(APRS.TRACK);
  //SerialUSB.flush();
}

void APRS_sendLoc(char *comment) {
  APRS_PrepeareLoc(comment);
  APRS_sendpacket();
}

void APRS_sendStatus(char *msg) {
  APRS_PrepeareStatus(msg);
  APRS_sendpacket();
}
/********************************************************
   FM modulation
 ********************************************************/
void APRS_send_bit(int tempo)
{
  countPtr = 0;
  while (countPtr < tempo) {}
}

void APRS_sinus()
{
  ddsAccu = ddsAccu + ddsWord;
  sinusPtr = ddsAccu >> 23;
  while (DAC->STATUS.reg & DAC_STATUS_SYNCBUSY);
  DAC->DATA.reg = pgm_read_byte(&(sinusTable[sinusPtr]))*gain;
  countPtr++;
}

void APRS_flipout(void)
{
  stuff = 0;
  flip_freq ^= 1;
  if (flip_freq == 0) ddsWord = ddsWord1; else ddsWord = ddsWord0;

}

void APRS_fcsbit(unsigned short tbyte)
{
  crc ^= tbyte;
  if (crc & 1)
    crc = (crc >> 1) ^ 0x8408;
  else
    crc = crc >> 1;
}

void APRS_sendbyte (unsigned char inbyte)
{
  unsigned char k, bt;
  for (k = 0; k < 8; k++)
  { 
    bt = inbyte & 0x01;
    if ((fcsflag == 0) & (flag == 0)) (APRS_fcsbit(bt));
    if (bt == 0) (APRS_flipout());
    else {
      stuff++;
      if ((flag == 0) & (stuff == 5))
      { 
        APRS_send_bit(11);
        APRS_flipout();
      }
    }
    inbyte = inbyte >> 1;
    APRS_send_bit(11);
  }
}

void APRS_sendpacket()
{
  uint8_t size_array = strlen(APRS.TRACK);
  APRS_tcStartCounter();
  unsigned char i;
  crc = 0xffff;
  stuff = 0;
  flip_freq = 1;
  ddsWord = ddsWord1;
  APRS_send_bit(11);
  flag = 1;
  fcsflag = 0;
  for (i = 0; i < headFlag; i++) APRS_sendbyte(0x7E);	  //Sends 30 flag bytes.
  flag = 0; 
  for (i = 0; i < size_array; i++) APRS_sendbyte(APRS.TRACK[i]); //send the packet bytes
  fcsflag = 1;
  APRS_sendbyte((crc ^ 0xff));
  crc >>= 8;
  APRS_sendbyte((crc ^ 0xff));
  fcsflag = 0;
  flag = 1;
  for (i = 0; i < tailFlag; i++) APRS_sendbyte(0x7E);	  //Sends 30 flag bytes  
  analogWrite(A0, 0);
  APRS_tcDisable();
}
/********************************************************
   Timer 5
 ********************************************************/
void TC5_Handler (void) {
  APRS_sinus();
  TC5->COUNT16.INTFLAG.bit.MC0 = 1;
}

void APRS_tcConfigure(int sampleRate)
{
  GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5)) ;
  while (GCLK->STATUS.bit.SYNCBUSY);

  APRS_tcReset();

  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1 | TC_CTRLA_ENABLE;
  TC5->COUNT16.CC[0].reg = (uint16_t) (SystemCoreClock / sampleRate - 1);
  while (APRS_tcIsSyncing());

  NVIC_DisableIRQ(TC5_IRQn);
  NVIC_ClearPendingIRQ(TC5_IRQn);
  NVIC_SetPriority(TC5_IRQn, 0);
  NVIC_EnableIRQ(TC5_IRQn);

  TC5->COUNT16.INTENSET.bit.MC0 = 1;
  while (APRS_tcIsSyncing()); 
}

bool APRS_tcIsSyncing()
{
  return TC5->COUNT16.STATUS.reg & TC_STATUS_SYNCBUSY;
}

void APRS_tcStartCounter()
{
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
  while (APRS_tcIsSyncing()); 
}

void APRS_tcReset()
{
  TC5->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  while (APRS_tcIsSyncing());
  while (TC5->COUNT16.CTRLA.bit.SWRST);
}

void APRS_tcDisable()
{
  TC5->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (APRS_tcIsSyncing());
}
