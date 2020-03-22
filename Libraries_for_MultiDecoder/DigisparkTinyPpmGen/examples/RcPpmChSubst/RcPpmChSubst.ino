/*
RcPpmChSubst sketch
by RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 2015
This sketch reads an RC PPM frame, extracts the numbers of channels and their pulse witdhs and substitutes the 4th channel with a fix pulse width (2000 us).

.----------------.   .---------------.
| Trainer Output |-->| TinyPpmReader |
|                |   '---------------'
|                |         |
| RC Transmitter |         | Forward channels 1 to 3 and set 4th channel width to 2000 us
|                |         V
|                |   .---------------.
| Trainer Intput |<--|  TinyPpmGen   |
'----------------'   '---------------'

This sketch can work with a Digispark pro, Digispark, Arduino Leonardo, Arduino Micro, Arduino Pro Micro and Arduino UNO.
The PPM input shall support pin change interrupt.

PPM output pin is imposed by hardware and is target dependant:
(The user has to define Timer and Channel to use in TinyPpmGen.h file of the library)

       - ATtiny167 (Digispark pro):
         TIMER(0), CHANNEL(A) -> OC0A -> PA2 -> Pin#8

       - ATtiny85 (Digispark):
         TIMER(0), CHANNEL(A) -> OC0A -> PB0 -> Pin#0
         TIMER(0), CHANNEL(B) -> OC0B -> PB1 -> Pin#1
         TIMER(1), CHANNEL(A) -> OC1A -> PB1 -> Pin#1
         
       - ATtiny84 (Ext. Clock. 16MHz) -> Fuses: LF:0xFE, HF:0xDF, EF: 0xFF 
         TIMER(0), CHANNEL(A) -> OC0A -> PB2 -> Pin#5 | Digital 8 : D8
         TIMER(0), CHANNEL(B) -> OC0B -> PA7 -> Pin#6 | Digital 7 : D7

         - ATmega328P (Arduino UNO):
         TIMER(0), CHANNEL(A) -> OC0A -> PD6 -> Pin#6
         TIMER(0), CHANNEL(B) -> OC0B -> PD5 -> Pin#5
         TIMER(2), CHANNEL(A) -> OC2A -> PB3 -> Pin#11
         TIMER(2), CHANNEL(B) -> OC2B -> PD3 -> Pin#3

         - ATmega32U4 (Arduino Leonardo, Micro and Pro Micro):
         TIMER(0), CHANNEL(A) -> OC0A -> PB7 -> Pin#11 (/!\ pin not available on connector of Pro Micro /!\)
         TIMER(0), CHANNEL(B) -> OC0B -> PD0 -> Pin#3

This example code is in the public domain.
*/
#include <Rcul.h>

#include <TinyPinChange.h>
#include <TinyPpmReader.h>

#include <TinyPpmGen.h>

#define PPM_INPUT_PIN  2
#define CHANNEL_NB     4

void setup()
{
  TinyPpmReader.attach(PPM_INPUT_PIN); /* Attach TinyPpmReader to PPM_INPUT_PIN pin */
  TinyPpmGen.begin(TINY_PPM_GEN_POS_MOD, CHANNEL_NB); /* Change TINY_PPM_GEN_POS_MOD to TINY_PPM_GEN_NEG_MOD for NEGative PPM modulation */
}

void loop()
{
  if((TinyPpmReader.detectedChannelNb() >= CHANNEL_NB) && TinyPpmReader.isSynchro())
  {
    TinyPpmGen.setChWidth_us(1, TinyPpmReader.width_us(1)); /* RC Channel#1: forward rx value */
    TinyPpmGen.setChWidth_us(2, TinyPpmReader.width_us(2)); /* RC Channel#2: forward rx value */
    TinyPpmGen.setChWidth_us(3, TinyPpmReader.width_us(3)); /* RC Channel#3 forward rx value: */
    TinyPpmGen.setChWidth_us(4, 2000); /* RC Channel#4: replace rx pulse width with 2000 us */
  }
}
