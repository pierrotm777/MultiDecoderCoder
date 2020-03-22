//////////////////// Mavlink to PPM Sum //////////////////////////
/*  
  Gregory Dymarek / 1st November 2016

  Designed & tested on TeensyLC

  This is a transparent RC proxy that converts incoming Graupner SUMD channles into into PPM signal;

*/
//////////////////////////////////////////////////////////////////


#include "PulsePosition.h"
#include "sumd.h"

//////////////////////CONFIGURATION///////////////////////////////
#define DEBUG_SPEED 115200 
#define SUMD_SPEED 115200 //115200
#define LED_PIN 13
#define PPM_PIN 20  //set PPM out pin. For TeensyLC: 6, 9, 10, 20, 22, 23
#define SUMD_TIMEOUT_MS  1000
#define PPM_FAILSAFE 800 //signal to send on SUMD_TIMEOUT; has to be between TX_MINIMUM_SIGNAL and TX_MAXIMUM_SIGNAL
#define PPM_DEFAULT 1500 //default signal for unused channels

#define SUMD Serial //source
#define DEBUG Serial1
////////////////////////////////////
#define PPM_MAX_CHANNELS PULSEPOSITION_MAXCHANNELS

PulsePositionOutput ppmOut;

uint16_t i;
uint8_t rssi;
uint8_t rx_count;
uint16_t channel_count;
uint16_t channels[PPM_MAX_CHANNELS];

unsigned long t = millis();

void toggleLED(uint16_t ms) {
  digitalWrite(LED_PIN, HIGH);
  delay(ms);
  digitalWrite(LED_PIN, LOW);
}

void setup()
{
  DEBUG.begin(DEBUG_SPEED);
  pinMode(LED_PIN, OUTPUT);

  SUMD.begin(SUMD_SPEED);

  ppmOut.begin(PPM_PIN); 

  for (i=0;i<PPM_MAX_CHANNELS;i++)
    channels[i] = PPM_FAILSAFE;

  toggleLED(1000);

  DEBUG.println("");
  DEBUG.println("Started.");  
}


void loop() {

  if ((millis()-t)>SUMD_TIMEOUT_MS) {
    for (i=0;i<PPM_MAX_CHANNELS;i++)
      ppmOut.write(i+1,PPM_FAILSAFE);
    delay(20); 
  } 

  if (!SUMD.available()) return;

  uint8_t c = SUMD.read();

  int ret = sumd_decode(c, &rssi, &rx_count, &channel_count, channels, PPM_MAX_CHANNELS);
  if (ret) return;  //sumd_decode returns 0 on decoded packet

  for (i=channel_count;i<PPM_MAX_CHANNELS;i++) //unused channels 
      channels[i] = PPM_DEFAULT;
  
  //we have a valid sumd
  digitalWrite(LED_PIN, HIGH);
  t = millis(); //reset failsafe timer

  for (i=0;i<PPM_MAX_CHANNELS;i++)
    ppmOut.write(i+1,channels[i]);

  digitalWrite(LED_PIN, LOW);

}

