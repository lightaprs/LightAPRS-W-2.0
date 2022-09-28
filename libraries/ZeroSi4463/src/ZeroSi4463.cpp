/*
 * ZeroSi4463 is a simple OOK TX arduino library for Si4463.
 * The ZoroSi4463 library was developed only for LightAPRS hardware.
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

#include "ZeroSi4463.h"
#include <SPI.h>
// Generated with wireless development suite by silicon labs
#include "Zero_radio_config_Si4463.h"

const uint8_t Si4463_CONFIGURATION_DATA[] = RADIO_CONFIGURATION_DATA_ARRAY;

Si4463::Si4463(uint8_t nIRQPin, uint8_t sdnPin, uint8_t nSELPin)
{
  _nIRQPin = nIRQPin;
  _sdnPin = sdnPin;
  _nSELPin = nSELPin;
}
void Si4463::spiInit()
{
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(6);
  SPI.setDataMode(SPI_MODE0);
}

void Si4463::pinInit()
{
  pinMode(_sdnPin, OUTPUT);
  digitalWrite(_sdnPin, HIGH);
  pinMode(_nIRQPin, INPUT);
  pinMode(_nSELPin, OUTPUT);
  digitalWrite(_nSELPin, HIGH);
}

bool Si4463::init()
{
  pinInit();
  spiInit();
  uint8_t buf[20];
  powerOnReset();
  setConfig(Si4463_CONFIGURATION_DATA, sizeof(Si4463_CONFIGURATION_DATA));
  filter_coeffs();
  // set max tx power
  setTxPower(127);
  // check if Si4463 works
  if (!checkDevice())
  {
    return false;
  }
  return true;
}

void Si4463::powerOnReset()
{
  uint8_t buf[] = {RF_POWER_UP};
  digitalWrite(_sdnPin, HIGH);
  delay(100);
  digitalWrite(_sdnPin, LOW);
  delay(20);						// wait for Si4463 stable

  // send power up command
  digitalWrite(_nSELPin, LOW);
  spiWriteBuf(sizeof(buf), buf);
  digitalWrite(_nSELPin, HIGH);

  delay(200);
}

void Si4463::setModemOOK(void) {
  uint8_t buf[] = { 0x20, 0x01, 0x00, 0x89};
  setCommand(4, Si4463_CMD_SET_PROPERTY  , buf);
}

void Si4463::setConfig(const uint8_t* parameters, uint16_t paraLen)
{
  // command buf starts with length of command in RADIO_CONFIGURATION_DATA_ARRAY
  uint8_t cmdLen;
  uint8_t command;
  uint16_t pos;
  uint8_t buf[30];

  // power up command had already send
  paraLen = paraLen - 1;
  cmdLen = parameters[0];
  pos = cmdLen + 1;

  while (pos < paraLen)
  {
    cmdLen = parameters[pos++] - 1;		// get command lend
    command = parameters[pos++];		// get command
    memcpy(buf, parameters + pos, cmdLen);		// get parameters

    setCommand(cmdLen, command, buf);
    pos = pos + cmdLen;
  }
}

bool Si4463::checkDevice()
{
  uint8_t buf[9];
  uint16_t partInfo;
  if (!getCommand(9, Si4463_CMD_PART_INFO, buf))		// read part info to check if 4463 works
    return false;

  partInfo = buf[2] << 8 | buf[3];
  if (partInfo == 0x4463 || partInfo == 0x4464){} else return false;
  
}

bool Si4463::waitnIRQ()
{
  return !digitalRead(_nIRQPin);		// inquire interrupt
}

void Si4463::enterTxMode()
{
  uint8_t buf[] = {0x00, 0x30, 0x00, 0x00};
  buf[0] = Si4463_FREQ_CHANNEL;
  setCommand(4, Si4463_CMD_START_TX , buf);
}

bool Si4463::enterStandbyMode()
{
  uint8_t data = 0x01;
  return setCommand(1, Si4463_CMD_CHANGE_STATE, &data);
}

bool Si4463::clrInterrupts()
{
  uint8_t buf[] = { 0x00, 0x00, 0x00 };
  return setCommand(sizeof(buf), Si4463_CMD_GET_INT_STATUS, buf);
}

void Si4463::fifoReset()
{
  uint8_t data = 0x03;
  setCommand(sizeof(data), Si4463_CMD_FIFO_INFO, &data);
}

bool Si4463::setGPIOMode(uint8_t GPIO0Mode, uint8_t GPIO1Mode)
{
  uint8_t buf[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  buf[0] = GPIO0Mode;
  buf[1] = GPIO1Mode;

  return setCommand(sizeof(buf), Si4463_CMD_GPIO_PIN_CFG, buf);
}

bool Si4463::setTxPower(uint8_t power)
{
  if (power > 127)	// max is 127
    return false;

  uint8_t buf[4] = {0x08, 0x00, 0x00, 0x3d};
  buf[1] = power;

  return setProperties(Si4463_PROPERTY_PA_MODE, sizeof(buf), buf);

}

bool Si4463::setCommand(uint8_t length, uint8_t command, uint8_t* paraBuf)
{
  if (!checkCTS())
    return false;

  digitalWrite(_nSELPin, LOW);
  spiByte(command);				// send command
  spiWriteBuf(length, paraBuf);	// send parameters
  digitalWrite(_nSELPin, HIGH);

  return true;
}

bool Si4463::getCommand(uint8_t length, uint8_t command, uint8_t* paraBuf)
{
  if (!checkCTS())
    return false;

  digitalWrite(_nSELPin, LOW);
  spiByte(command);				// set command to read
  digitalWrite(_nSELPin, HIGH);

  if (!checkCTS())					// check if Si4463 is ready
    return false;

  digitalWrite(_nSELPin, LOW);
  spiByte(Si4463_CMD_READ_BUF);	// turn to read command mode
  spiReadBuf(length, paraBuf);		// read parameters
  digitalWrite(_nSELPin, HIGH);
}

bool Si4463::setProperties(uint16_t startProperty, uint8_t length , uint8_t* paraBuf)
{
  uint8_t buf[4];

  if (!checkCTS())
    return false;

  buf[0] = Si4463_CMD_SET_PROPERTY;
  buf[1] = startProperty >> 8;   		// GROUP
  buf[2] = length;                	// NUM_PROPS
  buf[3] = startProperty & 0xff; 		// START_PROP

  digitalWrite(_nSELPin, LOW);
  spiWriteBuf(4, buf);					// set start property and read length
  spiWriteBuf(length, paraBuf);		// set parameters
  digitalWrite(_nSELPin, HIGH);

  return true;
}

bool Si4463::getProperties(uint16_t startProperty, uint8_t length , uint8_t* paraBuf)
{
  if (!checkCTS())
    return false;

  uint8_t buf[4];
  buf[0] = Si4463_CMD_GET_PROPERTY;
  buf[1] = startProperty >> 8;   		// GROUP
  buf[2] = length;               		// NUM_PROPS
  buf[3] = startProperty & 0xff; 		// START_PROP

  digitalWrite(_nSELPin, LOW);
  spiWriteBuf(4, buf);					// set start property and read length
  digitalWrite(_nSELPin, HIGH);

  if (!checkCTS())
    return false;

  digitalWrite(_nSELPin, LOW);
  spiByte(Si4463_CMD_READ_BUF);		// turn to read command mode
  spiReadBuf(length, paraBuf);			// read parameters
  digitalWrite(_nSELPin, HIGH);
  return true;
}

bool Si4463::checkCTS()
{
  uint16_t timeOutCnt;
  timeOutCnt = Si4463_CTS_TIMEOUT;
  while (timeOutCnt--)				// cts counter
  {
    digitalWrite(_nSELPin, LOW);
    spiByte(Si4463_CMD_READ_BUF);	// send READ_CMD_BUFF command
    if (spiByte(0) == Si4463_CTS_REPLY)	// read CTS
    {
      digitalWrite(_nSELPin, HIGH);
      return true;
    }
    digitalWrite(_nSELPin, HIGH);
  }
  return	false;
}

void Si4463::spiWriteBuf(uint8_t writeLen, uint8_t* writeBuf)
{
  while (writeLen--)
    spiByte(*writeBuf++);
}

void Si4463::spiReadBuf(uint8_t readLen, uint8_t* readBuf)
{
  while (readLen--)
    *readBuf++ = spiByte(0);
}

uint8_t Si4463::spiByte(uint8_t writeData)
{
  uint8_t readData;
  readData = SPI.transfer(writeData);
  return readData;
}

bool Si4463::filter_coeffs(void)
{ 
  uint8_t buf[9] = {0xAD, 0x75, 0xFB, 0x9A, 0x8E, 0xC8, 0x07, 0x21, 0x19}; // UBSEDS (FIR python)
  return setProperties(Si4463_PROPERTY_MODEM_TX_FILTER_COEFF, sizeof(buf), buf);
}

bool Si4463::setFrequency(uint32_t freq)
{
  uint8_t outdiv = 4;
  uint8_t band = 0;
  if (freq < 705000000UL) {
    outdiv = 6;
    band = 1;
  };
  if (freq < 525000000UL) {
    outdiv = 8;
    band = 2;
  };
  if (freq < 353000000UL) {
    outdiv = 12;
    band = 3;
  };
  if (freq < 239000000UL) {
    outdiv = 16;
    band = 4;
  };
  if (freq < 177000000UL) {
    outdiv = 24;
    band = 5;
  };

  uint32_t f_pfd = 2 * RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ / outdiv;
  uint32_t n = ((uint32_t)(freq / f_pfd)) - 1;
  float ratio = (float)freq / (float)f_pfd;
  float rest = ratio - (float)n;
  uint32_t m = (uint32_t)(rest * 524288UL);
  uint32_t m2 = m / 0x10000;
  uint32_t m1 = (m - m2 * 0x10000) / 0x100;
  uint32_t m0 = (m - m2 * 0x10000 - m1 * 0x100);

  uint8_t buf[1] = {0b1000 + band};
  setProperties(Si4463_PROPERTY_MODEM_CLKGEN_BAND, sizeof(buf), buf);

  uint8_t buf2[4] = {n, m2, m1, m0};
  return setProperties(Si4463_PROPERTY_FREQ_CONTROL_INTE, sizeof(buf2), buf2);

}
