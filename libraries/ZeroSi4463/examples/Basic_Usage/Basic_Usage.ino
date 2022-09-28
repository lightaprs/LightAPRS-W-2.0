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
 
#include <ZeroSi4463.h>

#define GPIO0_PIN     3
#define TCXO_Pwr      A4
#define Si446x_nIRQ   9
#define Si446x_SDN    4
#define Si446x_nSEL   8

Si4463 si4463(Si446x_nIRQ, Si446x_SDN, Si446x_nSEL);

void setup() {
  SerialUSB.begin(57600);
  //while(!SerialUSB);

  pinMode(GPIO0_PIN, OUTPUT);
  digitalWrite(GPIO0_PIN, LOW);

  pinMode(TCXO_Pwr, OUTPUT);
  digitalWrite(TCXO_Pwr, HIGH);
  delay(10);  

  digitalWrite(Si446x_SDN, LOW);
  delay(20);  // wait for si4463 stable
  if (!si4463.init()) {
    SerialUSB.println("Init fail!");
    digitalWrite(Si446x_SDN, HIGH);
    while (1);
  } else {
    SerialUSB.println("Init OK");
    si4463.setFrequency(144800000UL);
    si4463.setModemOOK();
    si4463.enterTxMode();
  }
}

void loop() {
  digitalWrite(GPIO0_PIN, HIGH);
  SerialUSB.println("ON");
  delay(500);
  digitalWrite(GPIO0_PIN, LOW);
  SerialUSB.println("OFF");
  delay(1000);
}
