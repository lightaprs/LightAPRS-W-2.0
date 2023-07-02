#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <math.h>
#include <stdio.h>
#include <avr/dtostrf.h>
#include <ZeroAPRS.h>                       //https://github.com/hakkican/ZeroAPRS
#include <ZeroSi4463.h>                     //https://github.com/hakkican/ZeroSi4463
#include <TinyGPS++.h>                      //https://github.com/mikalhart/TinyGPSPlus
#include <GEOFENCE.h>                       // Modified version of https://github.com/TomasTT7/TT7F-Float-Tracker/blob/master/Software/ARM_GEOFENCE.c
#include <Adafruit_SleepyDog.h>             //https://github.com/adafruit/Adafruit_SleepyDog
#include <Adafruit_BMP085.h>                //https://github.com/adafruit/Adafruit-BMP085-Library
#include <si5351.h>                         //https://github.com/etherkit/Si5351Arduino
#include <JTEncode.h>                       //https://github.com/etherkit/JTEncode (JT65/JT9/JT4/FT8/WSPR/FSQ Encoder Library)
#include <TimeLib.h>                        //https://github.com/PaulStoffregen/Time
#include <Adafruit_ZeroTimer.h>             //https://github.com/adafruit/Adafruit_ZeroTimer
#include <MemoryFree.h>;

#define Si5351Pwr     A3
#define TCXO_Pwr      A4
#define BattPin       A5
#define PTT_PIN       3
#define GpsPwr        7
#define Si446x_nIRQ   9
#define Si446x_SDN    4
#define Si446x_nSEL   8

//macros
#define Si5351ON    digitalWrite(Si5351Pwr, HIGH)//NPN
#define Si5351OFF   digitalWrite(Si5351Pwr, LOW)
#define GpsON       digitalWrite(GpsPwr, LOW)//PNP
#define GpsOFF      digitalWrite(GpsPwr, HIGH);
#define PttON       digitalWrite(PTT_PIN, HIGH)
#define PttOFF      digitalWrite(PTT_PIN, LOW)
#define Si4463ON    digitalWrite(Si446x_SDN, LOW)
#define Si4463OFF   digitalWrite(Si446x_SDN, HIGH)
#define TcxoON      digitalWrite(TCXO_Pwr, HIGH)
#define TcxoOFF     digitalWrite(TCXO_Pwr, LOW)

//#define DEVMODE // Development mode. Uncomment to enable for debugging.

//******************************  APRS CONFIG **********************************
char    CallSign[7]="NOCALL";//DO NOT FORGET TO CHANGE YOUR CALLSIGN
int8_t  CallNumber=11; //11; //SSID http://www.aprs.org/aprs11/SSIDs.txt
char    Symbol='O'; // 'O' for balloon, '>' for car, for more info : http://www.aprs.org/symbols/symbols-new.txt
bool    alternateSymbolTable = false ; //false = '/' , true = '\'

char    comment[50] = "LightAPRS-W 2.0";// Max 50 char
char    StatusMessage[50] = "LightAPRS-W 2.0 by TA2NHP & TA2MUN";

uint32_t GEOFENCE_APRS_frequency      = 144390000 ;//default frequency before geofencing. This variable will be updated based on GPS location.
int32_t APRS_Freq_Correction          = -1200;     //Hz. vctcxo frequency correction for si4463

//*****************************************************************************

uint16_t  BeaconWait=50;  //seconds sleep for next beacon (HF or VHF). This is an optimized value, do not change this if possible.
uint16_t  BattWait=60;    //seconds sleep if super capacitors/batteries are below BattMin (important if power source is solar panel) 
float     BattMin=3.3;    // min Volts to wake up.
float     GpsMinVolt=4.5; //min Volts for GPS to wake up. (important if power source is solar panel) 
float     WsprBattMin=4.0;//min Volts for HF (WSPR) radio module to transmit (TX) ~10 mW
float     HighVolt=6.0; //GPS is always on if the voltage exceeds this value to protect solar caps from overcharge

//******************************  HF (WSPR) CONFIG *************************************

char hf_call[7] = "NOCALL";// DO NOT FORGET TO CHANGE YOUR CALLSIGN

//#define WSPR_DEFAULT_FREQ       10140200UL //30m band
#define WSPR_DEFAULT_FREQ       14097100UL //20m band
//#define WSPR_DEFAULT_FREQ       18106100UL //17M band
//#define WSPR_DEFAULT_FREQ       21096100UL //15m band
//#define WSPR_DEFAULT_FREQ       24926100UL //12M band
//#define WSPR_DEFAULT_FREQ       28126100UL //10m band
//for all bands -> http://wsprnet.org/drupal/node/7352


// Supported modes, default HF mode is WSPR
enum mode {MODE_JT9, MODE_JT65, MODE_JT4, MODE_WSPR, MODE_FSQ_2, MODE_FSQ_3,
  MODE_FSQ_4_5, MODE_FSQ_6, MODE_FT8};
  
enum mode cur_mode = MODE_WSPR; //default HF mode

//supported other modes freq for 20m
#define JT9_DEFAULT_FREQ        14078700UL
#define FT8_DEFAULT_FREQ        14075000UL
#define JT65_DEFAULT_FREQ       14078300UL
#define JT4_DEFAULT_FREQ        14078500UL
#define FSQ_DEFAULT_FREQ        7105350UL     // Base freq is 1350 Hz higher than dial freq in USB

//*******************************************************************************


//******************************  APRS SETTINGS *********************************

//do not change WIDE path settings below if you don't know what you are doing :) 
uint8_t   Wide1=1; // 1 for WIDE1-1 path
uint8_t   Wide2=1; // 1 for WIDE2-1 path

/**
Airborne stations above a few thousand feet should ideally use NO path at all, or at the maximum just WIDE2-1 alone.  
Due to their extended transmit range due to elevation, multiple digipeater hops are not required by airborne stations.  
Multi-hop paths just add needless congestion on the shared APRS channel in areas hundreds of miles away from the aircraft's own location.  
NEVER use WIDE1-1 in an airborne path, since this can potentially trigger hundreds of home stations simultaneously over a radius of 150-200 miles. 
 */
uint8_t pathSize=2; // 2 for WIDE1-N,WIDE2-N ; 1 for WIDE2-N
boolean autoPathSizeHighAlt = true; //force path to WIDE2-N only for high altitude (airborne) beaconing (over 1.000 meters (3.280 feet)) 
boolean beaconViaARISS = true; //there are no iGates in some regions (such as North Africa,  Oceans, etc) so try to beacon via ARISS (International Space Station) https://www.amsat.org/amateur-radio-on-the-iss/

// Send aprs high precision position extension (adds 5 bytes to beacon messaage)
boolean send_aprs_enhanced_precision = true;
boolean  aliveStatus = true; //for tx status message on first wake-up just once.
boolean radioSetup = false; //do not change this, temp value
static char telemetry_buff[100];// telemetry buffer
uint16_t TxCount = 1; //increase +1 after every APRS transmission

//*******************************************************************************

//******************************  HF SETTINGS   *********************************

#define JT9_TONE_SPACING        174          // ~1.74 Hz
#define JT65_TONE_SPACING       269          // ~2.69 Hz
#define JT4_TONE_SPACING        437          // ~4.37 Hz
#define WSPR_TONE_SPACING       146          // ~1.46 Hz
#define FSQ_TONE_SPACING        879          // ~8.79 Hz
#define FT8_TONE_SPACING        625          // ~6.25 Hz

#define JT9_DELAY               576          // Delay value for JT9-1
#define JT65_DELAY              371          // Delay in ms for JT65A
#define JT4_DELAY               229          // Delay value for JT4A
#define WSPR_DELAY              683          // Delay value for WSPR
#define FSQ_2_DELAY             500          // Delay value for 2 baud FSQ
#define FSQ_3_DELAY             333          // Delay value for 3 baud FSQ
#define FSQ_4_5_DELAY           222          // Delay value for 4.5 baud FSQ
#define FSQ_6_DELAY             167          // Delay value for 6 baud FSQ
#define FT8_DELAY               159          // Delay value for FT8

#define HF_CORRECTION              -13000       // Change this for your ref osc
  
// Global variables
unsigned long hf_freq;
char hf_message[13] = "NOCALL AA00";//for non WSPR modes, you don't have to change this, updated by hf_call and GPS location
char hf_loc[] = "AA00";             //for WSPR, updated by GPS location. You don't have to change this.
uint8_t dbm = 10;
uint8_t tx_buffer[255];
uint8_t symbol_count;
uint16_t tone_delay, tone_spacing;
volatile bool proceed = false;


//*******************************************************************************


//******************************  GPS SETTINGS   *********************************
int16_t   GpsResetTime=1800; // timeout for reset if GPS is not fixed

// GEOFENCE 
uint32_t GEOFENCE_no_tx               = 0; //do not change this, temp value. 
boolean arissModEnabled = false; //do not change this, temp value. 

boolean GpsFirstFix=false; //do not change this
boolean ublox_high_alt_mode_enabled = false; //do not change this
int16_t GpsInvalidTime=0; //do not change this

//********************************************************************************

Si5351 si5351(0x60);
TinyGPSPlus gps;
Adafruit_BMP085 bmp;
JTEncode jtencode;
Si4463 si4463(Si446x_nIRQ, Si446x_SDN, Si446x_nSEL);
Adafruit_ZeroTimer zerotimer = Adafruit_ZeroTimer(3);


void setup() {
  Watchdog.enable(30000);
  Watchdog.reset();
  // While the energy rises slowly with the solar panel, 
  // using the analog reference low solves the analog measurement errors.
  analogReference(AR_INTERNAL1V65);
  pinMode(PTT_PIN, OUTPUT);
  pinMode(Si5351Pwr, OUTPUT);
  pinMode(TCXO_Pwr, OUTPUT);
  pinMode(GpsPwr, OUTPUT);
  pinMode(BattPin, INPUT);
  pinMode(Si446x_SDN, OUTPUT);
  
  GpsOFF;
  Si5351OFF;  
  PttOFF;
  Si4463OFF;
  TcxoOFF;

  Serial.begin(9600);//GPS
  SerialUSB.begin(115200);
  // Wait up to 5 seconds for serial to be opened, to allow catching
  // startup messages on native USB boards (that do not reset when
  // serial is opened).
  Watchdog.reset();  
  unsigned long start = millis();
  while (millis() - start < 5000 && !SerialUSB){;}
  Watchdog.reset(); 

  SerialUSB.println(F("Starting"));

  APRS_init();
  APRS_setCallsign(CallSign, CallNumber);
  APRS_setDestination("APLIGA", 0);
  APRS_setPath1("WIDE1", Wide1);
  APRS_setPath2("WIDE2", Wide2);
  APRS_setPathSize(2);
  APRS_useAlternateSymbolTable(alternateSymbolTable);
  APRS_setSymbol(Symbol);
  APRS_setPathSize(pathSize);

  Wire.begin();
  bmp.begin();

  SerialUSB.println(F(""));
  SerialUSB.print(F("APRS (VHF) CallSign: "));
  SerialUSB.print(CallSign);
  SerialUSB.print(F("-"));
  SerialUSB.println(CallNumber);

  SerialUSB.print(F("WSPR (HF) CallSign: "));
  SerialUSB.println(hf_call);
  SerialUSB.println(F(""));
  
}

void loop() {
  Watchdog.reset();


if (((readBatt() > BattMin) && GpsFirstFix) || ((readBatt() > GpsMinVolt) && !GpsFirstFix)) {

    if (aliveStatus) {

      sendStatus();	  
      aliveStatus = false;

      while (readBatt() < BattMin) {
        sleepSeconds(BattWait); 
      }   
    }
    
      updateGpsData(1000);
      gpsDebug();

      if(gps.location.isValid() && gps.location.age()<1000){
        GpsInvalidTime=0;
      }else{
        GpsInvalidTime++;
        if(GpsInvalidTime > GpsResetTime){
          GpsOFF; 
          ublox_high_alt_mode_enabled = false; //gps sleep mode resets high altitude mode.
          Watchdog.reset();
          delay(1000);
          GpsON;
          GpsInvalidTime=0;     
        }
      }

      if ((gps.location.age() < 1000 || gps.location.isUpdated()) && gps.location.isValid()) {
        if (gps.satellites.isValid() && gps.satellites.value() > 3) {
          GpsFirstFix = true;
          if(readBatt() < HighVolt){
             GpsOFF; 
             ublox_high_alt_mode_enabled = false; //gps sleep mode resets high altitude mode.
          }      
          GpsInvalidTime=0;

          // Checks if there is an HF (WSPR) TX window is soon, if not then send APRS beacon
          if (!((minute() % 10 == 3 || minute() % 10 == 7) &&  second()>50 && readBatt() > WsprBattMin && timeStatus() == timeSet)){
             updateTelemetry();
            //APRS frequency isn't the same for the whole world. (for pico balloon only)
            if (!radioSetup || TxCount == 200) {
              configureFreqbyLocation();
            }

            if(!arissModEnabled && autoPathSizeHighAlt && gps.altitude.feet()>3000){
              //force to use high altitude settings (WIDE2-n)
              APRS_setPathSize(1);
            } else {
              //use default settings  
              APRS_setPathSize(pathSize);
            }

            //in some countries Airborne APRS is not allowed. (for pico balloon only)
            if (isAirborneAPRSAllowed()) {
              sendLocation();             

            }
            freeMem();
            SerialUSB.flush();

          }   

          // preparations for HF starts one minute before TX time at minute 3, 7, 13, 17, 23, 27, 33, 37, 43, 47, 53 or 57. No APRS TX during this period...
          if (readBatt() > WsprBattMin && timeStatus() == timeSet && ((minute() % 10 == 3) || (minute() % 10 == 7)) ) { 
            GridLocator(hf_loc, gps.location.lat(), gps.location.lng());
            sprintf(hf_message,"%s %s",hf_call,hf_loc);
            
            #if defined(DEVMODE)
            SerialUSB.println(F("Digital HF Mode Prepearing"));
            SerialUSB.print(F("Grid Locator: "));
            SerialUSB.println(hf_loc);
            #endif
            
            //HF transmission starts at minute 4, 8, 14, 18, 24, 28, 34, 38, 44, 48, 54 or 58 
            while (((minute() % 10 != 4) || (minute() % 10 != 8)) && second() != 0) {
              Watchdog.reset();
              delay(1);
            }
            #if defined(DEVMODE)
            SerialUSB.println(F("Digital HF Mode Sending..."));
            #endif          
            encode();
            //HFSent=true;

            #if defined(DEVMODE)
            SerialUSB.println(F("Digital HF Mode Sent"));
            #endif             
  
          } else {
            sleepSeconds(BeaconWait);
          }   
        }else {
          #if defined(DEVMODE)
          SerialUSB.println(F("Not enough sattelites"));
          #endif
        }
      }

    
  } else {
    sleepSeconds(BattWait);
  }
}

void sleepSeconds(int sec) {
  PttOFF;
  Si4463OFF;
  Si5351OFF;
  TcxoOFF;
  SerialUSB.flush();
  for (int i = 0; i < sec; i++) {
    if (GpsFirstFix){//sleep gps after first fix
      if (readBatt() < HighVolt){
        GpsOFF;
        ublox_high_alt_mode_enabled = false;
      }
    }else{
      if (readBatt() < BattMin){
        GpsOFF;
        ublox_high_alt_mode_enabled = false;
      }
    } 
     
    
    Watchdog.reset();
    delay(1000);
    
  }
  Watchdog.reset();
}

boolean isAirborneAPRSAllowed() {

  float tempLat = gps.location.lat();
  float tempLong = gps.location.lng();

  GEOFENCE_position(tempLat, tempLong);

  boolean airborne = true;

  if (GEOFENCE_no_tx == 1) {
    airborne = false;
  }

  return airborne;
}

boolean inARISSGeoFence(float tempLat, float tempLong) {
  boolean ariss = false;
  //North Africa
  if(tempLat>0 && tempLat<32 && tempLong>0 && tempLong<32){ariss = true;}
  //North Pacific
  if(tempLat>28 && tempLat<50 && tempLong>-180 && tempLong<-130){ariss = true;}
  //North Atlantic
  if(tempLat>25 && tempLat<42 && tempLong>-60 && tempLong<-33){ariss = true;} 
 
  return ariss;
}

void configureFreqbyLocation() {

  float tempLat = gps.location.lat();
  float tempLong = gps.location.lng();


  if(beaconViaARISS && inARISSGeoFence(tempLat, tempLong)) {
    APRS_setPath1("ARISS", Wide1);
    APRS_setPath2("WIDE2", Wide2);
    APRS_setPathSize(2);
    GEOFENCE_APRS_frequency=145825000;
    arissModEnabled = true;
  } else {

    GEOFENCE_position(tempLat,tempLong);      
    arissModEnabled = false;
  }  
  radioSetup = true;
}


void updatePosition(int high_precision, char *dao) {
  // Convert and set latitude NMEA string Degree Minute Hundreths of minutes ddmm.hh[S,N].
  char latStr[10];
  RawDegrees rawDeg = gps.location.rawLat();
  uint32_t min_nnnnn;
  char lat_dao = 0;
  min_nnnnn = rawDeg.billionths * 0.006;
  if ( ((min_nnnnn / (high_precision ? 1 : 100)) % 10) >= 5 && min_nnnnn < (6000000 - ((high_precision ? 1 : 100)*5)) ) {
    // round up. Avoid overflow (59.999999 should never become 60.0 or more)
    min_nnnnn = min_nnnnn + (high_precision ? 1 : 100)*5;
  }
  sprintf(latStr, "%02u%02u.%02u%c", (unsigned int ) (rawDeg.deg % 100), (unsigned int ) ((min_nnnnn / 100000) % 100), (unsigned int ) ((min_nnnnn / 1000) % 100), rawDeg.negative ? 'S' : 'N');
  if (dao)
    dao[0] = (char) ((min_nnnnn % 1000) / 11) + 33;
  APRS_setLat(latStr);

  // Convert and set longitude NMEA string Degree Minute Hundreths of minutes ddmm.hh[E,W].
  char lonStr[10];
  rawDeg = gps.location.rawLng();
  min_nnnnn = rawDeg.billionths * 0.006;
  if ( ((min_nnnnn / (high_precision ? 1 : 100)) % 10) >= 5 && min_nnnnn < (6000000 - ((high_precision ? 1 : 100)*5)) ) {
    min_nnnnn = min_nnnnn + (high_precision ? 1 : 100)*5;
  }
  sprintf(lonStr, "%03u%02u.%02u%c", (unsigned int ) (rawDeg.deg % 1000), (unsigned int ) ((min_nnnnn / 100000) % 100), (unsigned int ) ((min_nnnnn / 1000) % 100), rawDeg.negative ? 'W' : 'E');
  if (dao) {
    dao[1] = (char) ((min_nnnnn % 1000) / 11) + 33;
    dao[2] = 0;
  }

  APRS_setLon(lonStr);
  APRS_setTimeStamp(gps.time.hour(), gps.time.minute(),gps.time.second());
}


void updateTelemetry() {
  sprintf(telemetry_buff, "%03d", gps.course.isValid() ? (int)gps.course.deg() : 0);
  telemetry_buff[3] = '/';
  sprintf(telemetry_buff + 4, "%03d", gps.speed.isValid() ? (int)gps.speed.knots() : 0);
  telemetry_buff[7] = '/';
  telemetry_buff[8] = 'A';
  telemetry_buff[9] = '=';
  //sprintf(telemetry_buff + 10, "%06lu", (long)gps.altitude.feet());

  //fixing negative altitude values causing display bug on aprs.fi
  float tempAltitude = gps.altitude.feet();

  if (tempAltitude>0){
    //for positive values
    sprintf(telemetry_buff + 10, "%06lu", (long)tempAltitude);
  } else{
    //for negative values
    sprintf(telemetry_buff + 10, "%06d", (long)tempAltitude);
    } 
  
  telemetry_buff[16] = ' ';
  sprintf(telemetry_buff + 17, "%03d", TxCount);
  telemetry_buff[20] = 'T';
  telemetry_buff[21] = 'x';
  telemetry_buff[22] = 'C';
  Si5351ON;//little hack to prevent a BMP180 related issue 
  delay(1);
  telemetry_buff[23] = ' '; float tempC = bmp.readTemperature();
  dtostrf(tempC, 6, 2, telemetry_buff + 24);
  telemetry_buff[30] = 'C';
  telemetry_buff[31] = ' '; float pressure = bmp.readPressure() / 100.0; //Pa to hPa
  dtostrf(pressure, 7, 2, telemetry_buff + 32);
  Si5351OFF; 
  telemetry_buff[39] = 'h';
  telemetry_buff[40] = 'P';
  telemetry_buff[41] = 'a';
  telemetry_buff[42] = ' ';
  dtostrf(readBatt(), 5, 2, telemetry_buff + 43);
  telemetry_buff[48] = 'V';
  telemetry_buff[49] = ' ';
  sprintf(telemetry_buff + 50, "%02d", gps.satellites.isValid() ? (int)gps.satellites.value() : 0);
  telemetry_buff[52] = 'S';
  telemetry_buff[53] = ' ';

  sprintf(telemetry_buff + 54, "%s", comment);   

  // APRS PRECISION AND DATUM OPTION http://www.aprs.org/aprs12/datum.txt ; this extension should be added at end of beacon message.
  // We only send this detailed info if it's likely we're interested in, i.e. searching for landing position
  if (send_aprs_enhanced_precision && gps.altitude.feet() < 10000L && strlen(telemetry_buff) < sizeof(telemetry_buff) - 1 - 5 - 1) /* room for " !wAB!\0" */ {
    char dao[3];
    updatePosition(1, dao);
    sprintf(telemetry_buff + strlen(telemetry_buff), " !w%s!", dao);
  } else {
    updatePosition(0, NULL);
  }

#if defined(DEVMODE)
  SerialUSB.println(telemetry_buff);
#endif

}

void sendLocation() {

#if defined(DEVMODE)
  SerialUSB.println(F("Location sending with comment"));
#endif

  TcxoON;
  delay(10);
  Si4463ON;
  delay(20);            // wait for si4463 stable
  if (!si4463.init())
  {
    #if defined(DEVMODE)
    SerialUSB.println("Si4463 init fail!");  
    #endif  
    Si4463OFF;
    TcxoOFF;

  } else {
    #if defined(DEVMODE)
    SerialUSB.println("Si4463 Init OK");
    #endif  
    si4463.setFrequency(GEOFENCE_APRS_frequency+ APRS_Freq_Correction);
    si4463.setModemOOK();
    si4463.enterTxMode();
    analogWrite(A0, 128);
    PttON;
    delay(500);
    APRS_sendLoc(telemetry_buff);
    delay(10);
    PttOFF;
    si4463.enterStandbyMode();
    Si4463OFF;
    TcxoOFF;
    SerialUSB.print(F("APRS Location sent (Freq: "));
    SerialUSB.print(GEOFENCE_APRS_frequency);
    SerialUSB.print(F(") - "));    
    SerialUSB.println(TxCount);
    TxCount++;

  }

}

void sendStatus() {
  TcxoON;
  delay(10);
  Si4463ON;
  delay(20);            // wait for si4463 stable
  if (!si4463.init())
  {
    #if defined(DEVMODE)
    SerialUSB.println("Si4463 init fail!");  
    #endif  
    Si4463OFF;
    TcxoOFF;

  } else {
    #if defined(DEVMODE)
    SerialUSB.println("Si4463 Init OK");
    #endif  
    si4463.setFrequency(GEOFENCE_APRS_frequency+ APRS_Freq_Correction);
    si4463.setModemOOK();
    si4463.enterTxMode();
    analogWrite(A0, 128);
    PttON;
    delay(500);
    APRS_sendStatus(StatusMessage);
    delay(10);
    PttOFF;
    si4463.enterStandbyMode();
    Si4463OFF;
    TcxoOFF;
    SerialUSB.print(F("Status sent (Freq: "));
    SerialUSB.print(GEOFENCE_APRS_frequency);
    SerialUSB.print(F(") - "));
    SerialUSB.println(TxCount);
    TxCount++;
  }

}

static void updateGpsData(int ms)
{
  Watchdog.reset();
  GpsON;
  while (!Serial) {delay(1);} // wait for serial port to connect.  
  if(!ublox_high_alt_mode_enabled){
    //enable ublox high altitude mode
    setGPS_DynamicModel6();
    #if defined(DEVMODE)
      SerialUSB.println(F("ublox DynamicModel6 enabled..."));
    #endif      
    ublox_high_alt_mode_enabled = true;      
  }
  
  unsigned long start = millis();
  unsigned long bekle=0;
  do
  {
    while (Serial.available()>0) {
      char c;
      c=Serial.read();
      gps.encode(c);
      bekle= millis();
    }
    
    if (bekle!=0 && bekle+10<millis())break;
  } while (millis() - start < ms);

  if (gps.time.isValid())
  {
    setTime(gps.time.hour(), gps.time.minute(), gps.time.second(), NULL, NULL, NULL);     
  }
}

//following GPS code from : https://github.com/HABduino/HABduino/blob/master/Software/habduino_v4/habduino_v4.ino
void setGPS_DynamicModel6()
{  
  int gps_set_sucess=0;
  uint8_t setdm6[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06,
  0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC };

  while(!gps_set_sucess)
  {
    #if defined(DEVMODE)
      SerialUSB.println(F("ublox DynamicModel6 try..."));
    #endif 
    sendUBX(setdm6, sizeof(setdm6)/sizeof(uint8_t));
    gps_set_sucess=getUBX_ACK(setdm6);
  }
}

void sendUBX(uint8_t *MSG, uint8_t len) {
  Serial.write(0xFF);
  delay(500);
  for(int i=0; i<len; i++) {
    Serial.write(MSG[i]);
  }
}

boolean getUBX_ACK(uint8_t *MSG) {
  uint8_t b;
  uint8_t ackByteID = 0;
  uint8_t ackPacket[10];
  unsigned long startTime = millis();
  boolean status =false;

  // Construct the expected ACK packet
  ackPacket[0] = 0xB5; // header
  ackPacket[1] = 0x62; // header
  ackPacket[2] = 0x05; // class
  ackPacket[3] = 0x01; // id
  ackPacket[4] = 0x02; // length
  ackPacket[5] = 0x00;
  ackPacket[6] = MSG[2]; // ACK class
  ackPacket[7] = MSG[3]; // ACK id
  ackPacket[8] = 0; // CK_A
  ackPacket[9] = 0; // CK_B

  // Calculate the checksums
  for (uint8_t ubxi=2; ubxi<8; ubxi++) {
    ackPacket[8] = ackPacket[8] + ackPacket[ubxi];
    ackPacket[9] = ackPacket[9] + ackPacket[8];
  }

  while (1) {
    // Test for success
    if (ackByteID > 9) {
      // All packets in order!
      status= true;
      break;
    }

    // Timeout if no valid response in 3 seconds
    if (millis() - startTime > 3000) {
      status= false;
      break;
    }

    // Make sure data is available to read
    if (Serial.available()) {
      b = Serial.read();

      // Check that bytes arrive in sequence as per expected ACK packet
      if (b == ackPacket[ackByteID]) {
        ackByteID++;
      }
      else {
        ackByteID = 0; // Reset and look again, invalid order
      }
    }
  }
  return status;
}

void gpsDebug() {
#if defined(DEVMODE)
  SerialUSB.println();
  SerialUSB.println(F("Sats HDOP Latitude   Longitude   Fix  Date       Time     Date Alt    Course Speed Card Chars Sentences Checksum"));
  SerialUSB.println(F("          (deg)      (deg)       Age                      Age  (m)    --- from GPS ----  RX    RX        Fail"));
  SerialUSB.println(F("-----------------------------------------------------------------------------------------------------------------"));

  printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
  printInt(gps.hdop.value(), gps.hdop.isValid(), 5);
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  printInt(gps.location.age(), gps.location.isValid(), 5);
  printDateTime(gps.date, gps.time);
  printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
  printFloat(gps.course.deg(), gps.course.isValid(), 7, 2);
  printFloat(gps.speed.kmph(), gps.speed.isValid(), 6, 2);
  printStr(gps.course.isValid() ? TinyGPSPlus::cardinal(gps.course.value()) : "*** ", 6);

  printInt(gps.charsProcessed(), true, 6);
  printInt(gps.sentencesWithFix(), true, 10);
  printInt(gps.failedChecksum(), true, 9);
  SerialUSB.println();

#endif
}

static void printFloat(float val, bool valid, int len, int prec)
{
#if defined(DEVMODE)
  if (!valid)
  {
    while (len-- > 1)
      SerialUSB.print('*');
    SerialUSB.print(' ');
  }
  else
  {
    SerialUSB.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i = flen; i < len; ++i)
      SerialUSB.print(' ');
  }
#endif
}

static void printInt(unsigned long val, bool valid, int len)
{
#if defined(DEVMODE)
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  SerialUSB.print(sz);
#endif
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
#if defined(DEVMODE)
  if (!d.isValid())
  {
    SerialUSB.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    SerialUSB.print(sz);
  }

  if (!t.isValid())
  {
    SerialUSB.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    SerialUSB.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
#endif
}

static void printStr(const char *str, int len)
{
#if defined(DEVMODE)
  int slen = strlen(str);
  for (int i = 0; i < len; ++i)
    SerialUSB.print(i < slen ? str[i] : ' ');
#endif
}


float readBatt() {
  float R1 = 560000.0; // 560K
  float R2 = 100000.0; // 100K
  float value = 0.0f;

  do {    
    value =analogRead(BattPin);
    value +=analogRead(BattPin);
    value +=analogRead(BattPin);
    value = value / 3.0f;
    value = (value * 1.69) / 1024.0f;
    value = value / (R2/(R1+R2));
  } while (value > 20.0);
  return value ;
}

void TC3_Handler() {
  Adafruit_ZeroTimer::timerHandler(3);
}

void TimerCallback0(void)
{
 proceed = true;
}


void zeroTimerSetPeriodMs(float ms){

  float freq = 1000/ms; 

  uint16_t divider  = 1;
  uint16_t compare = 0;
  tc_clock_prescaler prescaler = TC_CLOCK_PRESCALER_DIV1;

  if ((freq < 24000000) && (freq > 800)) {
    divider = 1;
    prescaler = TC_CLOCK_PRESCALER_DIV1;
    compare = 48000000/freq;
  } else if (freq > 400) {
    divider = 2;
    prescaler = TC_CLOCK_PRESCALER_DIV2;
    compare = (48000000/2)/freq;
  } else if (freq > 200) {
    divider = 4;
    prescaler = TC_CLOCK_PRESCALER_DIV4;
    compare = (48000000/4)/freq;
  } else if (freq > 100) {
    divider = 8;
    prescaler = TC_CLOCK_PRESCALER_DIV8;
    compare = (48000000/8)/freq;
  } else if (freq > 50) {
    divider = 16;
    prescaler = TC_CLOCK_PRESCALER_DIV16;
    compare = (48000000/16)/freq;
  } else if (freq > 12) {
    divider = 64;
    prescaler = TC_CLOCK_PRESCALER_DIV64;
    compare = (48000000/64)/freq;
  } else if (freq > 3) {
    divider = 256;
    prescaler = TC_CLOCK_PRESCALER_DIV256;
    compare = (48000000/256)/freq;
  } else if (freq >= 0.75) {
    divider = 1024;
    prescaler = TC_CLOCK_PRESCALER_DIV1024;
    compare = (48000000/1024)/freq;
  } 

  zerotimer.enable(false);
  zerotimer.configure(prescaler,       // prescaler
          TC_COUNTER_SIZE_16BIT,       // bit width of timer/counter
          TC_WAVE_GENERATION_MATCH_PWM // frequency or PWM mode
          );

  zerotimer.setCompare(0, compare);
  zerotimer.setCallback(true, TC_CALLBACK_CC_CHANNEL0, TimerCallback0);
  zerotimer.enable(true);
}

void encode()
{
  Watchdog.reset();
  analogWrite(A0, 0);
  TcxoON;
  delay(10);
  Si5351ON;
  delay(20);
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, HF_CORRECTION);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA); // Set for max power if desired
  si5351.output_enable(SI5351_CLK0, 0);
  uint8_t i;

  switch(cur_mode)
  {
  case MODE_JT9:
    hf_freq = JT9_DEFAULT_FREQ;
    symbol_count = JT9_SYMBOL_COUNT; // From the library defines
    tone_spacing = JT9_TONE_SPACING;
    tone_delay = JT9_DELAY;
    break;
  case MODE_JT65:
    hf_freq = JT65_DEFAULT_FREQ;
    symbol_count = JT65_SYMBOL_COUNT; // From the library defines
    tone_spacing = JT65_TONE_SPACING;
    tone_delay = JT65_DELAY;
    break;
  case MODE_JT4:
    hf_freq = JT4_DEFAULT_FREQ;
    symbol_count = JT4_SYMBOL_COUNT; // From the library defines
    tone_spacing = JT4_TONE_SPACING;
    tone_delay = JT4_DELAY;
    break;
  case MODE_WSPR:
    hf_freq = WSPR_DEFAULT_FREQ;
    symbol_count = WSPR_SYMBOL_COUNT; // From the library defines
    tone_spacing = WSPR_TONE_SPACING;
    tone_delay = WSPR_DELAY;
    break;
  case MODE_FT8:
    hf_freq = FT8_DEFAULT_FREQ;
    symbol_count = FT8_SYMBOL_COUNT; // From the library defines
    tone_spacing = FT8_TONE_SPACING;
    tone_delay = FT8_DELAY;
    break;
  case MODE_FSQ_2:
    hf_freq = FSQ_DEFAULT_FREQ;
    tone_spacing = FSQ_TONE_SPACING;
    tone_delay = FSQ_2_DELAY;
    break;
  case MODE_FSQ_3:
    hf_freq = FSQ_DEFAULT_FREQ;
    tone_spacing = FSQ_TONE_SPACING;
    tone_delay = FSQ_3_DELAY;
    break;
  case MODE_FSQ_4_5:
    hf_freq = FSQ_DEFAULT_FREQ;
    tone_spacing = FSQ_TONE_SPACING;
    tone_delay = FSQ_4_5_DELAY;
    break;
  case MODE_FSQ_6:
    hf_freq = FSQ_DEFAULT_FREQ;
    tone_spacing = FSQ_TONE_SPACING;
    tone_delay = FSQ_6_DELAY;
    break;
  }
  set_tx_buffer();

  // Now transmit the channel symbols
  if(cur_mode == MODE_FSQ_2 || cur_mode == MODE_FSQ_3 || cur_mode == MODE_FSQ_4_5 || cur_mode == MODE_FSQ_6)
  {
    uint8_t j = 0;
    while(tx_buffer[j++] != 0xff);
    symbol_count = j - 1;
  }

  // Reset the tone to the base frequency and turn on the output
  si5351.output_enable(SI5351_CLK0, 1);
  
  zeroTimerSetPeriodMs(tone_delay); 
  
  for(i = 0; i < symbol_count; i++)
  {
      si5351.set_freq((hf_freq * 100) + (tx_buffer[i] * tone_spacing), SI5351_CLK0);
      proceed = false;
      while (!proceed);
      Watchdog.reset();
  }
  zerotimer.enable(false);
  // Turn off the output
  si5351.output_enable(SI5351_CLK0, 0);
  Si5351OFF;
  TcxoOFF;
  Watchdog.reset();
}

void set_tx_buffer()
{
  // Clear out the transmit buffer
  memset(tx_buffer, 0, 255);

  // Set the proper frequency and timer CTC depending on mode
  switch(cur_mode)
  {
  case MODE_JT9:
    jtencode.jt9_encode(hf_message, tx_buffer);
    break;
  case MODE_JT65:
    jtencode.jt65_encode(hf_message, tx_buffer);
    break;
  case MODE_JT4:
    jtencode.jt4_encode(hf_message, tx_buffer);
    break;
  case MODE_WSPR:
    jtencode.wspr_encode(hf_call, hf_loc, dbm, tx_buffer);
    break;
  case MODE_FT8:
    jtencode.ft8_encode(hf_message, tx_buffer);
    break;
  case MODE_FSQ_2:
  case MODE_FSQ_3:
  case MODE_FSQ_4_5:
  case MODE_FSQ_6:
    jtencode.fsq_dir_encode(hf_call, "n0call", ' ', "hello world", tx_buffer);
    break;
  }
}

void GridLocator(char *dst, float latt, float lon) {
  int o1, o2;
  int a1, a2;
  float remainder;
  // longitude
  remainder = lon + 180.0;
  o1 = (int)(remainder / 20.0);
  remainder = remainder - (float)o1 * 20.0;
  o2 = (int)(remainder / 2.0);
  // latitude
  remainder = latt + 90.0;
  a1 = (int)(remainder / 10.0);
  remainder = remainder - (float)a1 * 10.0;
  a2 = (int)(remainder);

  dst[0] = (char)o1 + 'A';
  dst[1] = (char)a1 + 'A';
  dst[2] = (char)o2 + '0';
  dst[3] = (char)a2 + '0';
  dst[4] = (char)0;
}

void freeMem() {
#if defined(DEVMODE)
  SerialUSB.print(F("Free RAM: ")); SerialUSB.print(freeMemory(), DEC); SerialUSB.println(F(" byte"));
#endif

}
