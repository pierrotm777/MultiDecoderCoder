/* Ce code ne fonctionne qu'avec un Arduino Uno,Pro Mini ou Pro Micro */

/* L'option JetiEx ne fonctionne qu'avec un Pro Micro */
/* L'option Failsafe ne fonctionne qu'avec un Pro Mini */

/* Décodeur pour commander jusqu'à 16 servos à partir d'un signal type:
 - PPM basé sur les librairies RC Navy https://github.com/RC-Navy/DigisparkArduinoIntegration/tree/master/libraries/DigisparkTinyCppmReader
 - SBUS basé sur les librairies RC Navy https://github.com/RC-Navy/DigisparkArduinoIntegration/tree/master/libraries/RcBusRx
    Un inverseur du signal est nécessaire (http://www.ernstc.dk/arduino/sbus.html)
 - IBUS basé sur la librairie Rc Navy  https://github.com/RC-Navy/DigisparkArduinoIntegration/tree/master/libraries/RcBusRx
 - SRXL basé sur les librairies RC Navy https://github.com/RC-Navy/DigisparkArduinoIntegration/tree/master/libraries/RcBusRx
 - SUMD basé sur l'exemple Rc Navy https://github.com/RC-Navy/DigisparkArduinoIntegration/tree/master/libraries/RcBusRx
 - DSMX basé sur la librairie https://github.com/Quarduino/SpektrumSatellite
 - JETIEX basé sur la librairie Rc Navy https://github.com/RC-Navy/DigisparkArduinoIntegration/tree/master/libraries/RcBusRx
 - MULTIWII basé sur la librairie https://github.com/fdivitto/MSP
*/

/*
 * Codeur permettant de convertir 8 sorties servos PWM en:
 - PPM basé sur les librairies RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 
 - SBUS basé sur les librairies RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 
 - IBUS
 */
 

#include <Rcul.h>
#include <TinyPinChange.h>
#include <SoftRcPulseOut.h>
#include <SoftRcPulseIn.h>
#include <TinyCppmReader.h>// configurer le le pour atmega328
#include <TinyCppmGen.h>
#include <EEPROM.h>
#include <RcBusRx.h>
#include <SBusTx.h>
#include <Streaming.h>

float VERSION_DECODER = 0.6;

#include <Vcc.h>
const float VccMin   = 0.0;           // Minimum expected Vcc level, in Volts.
const float VccMax   = 5.0;           // Maximum expected Vcc level, in Volts.
const float VccCorrection = 1.0/1.0;  // Measured Vcc by multimeter divided by reported Vcc
Vcc vcc(VccCorrection);
bool LowPower = false;

/* Macro function to declare an output pin */
#define out(x)      _out(x)
#define _out(bit,port)  DDR##port |= (1 << bit)
/* Macro function to declare an input pin */
#define in(x)     _in(x)
#define _in(bit,port) DDR##port &= ~(1 << bit)
/* Macro function to set an output pin high */
#define on(x)     _on(x)
#define _on(bit,port) PORT##port |= (1 << bit)
/* Macro function to set an output pin low */
#define off(x)      _off(x)
#define _off(bit,port)  PORT##port &= ~(1 << bit)
/* Macro function to set internal pullup resistor of input pin (same as "on" macro)*/
#define pullup(x)   _on(x)
/* Macro function to get state of input pin */
#define get(x)      _get(x)
#define _get(bit,port)  (PIN##port & (1 << bit))
/* Macro function to toggle an output pin */
#define flip(x)     _flip(x)
#define _flip(bit,port) PORT##port ^= (1 << bit)

uint32_t LedStartMs=millis();
//boolean  LedState=LOW;
unsigned long startedWaiting = millis();
unsigned long started1s = millis();
bool InFailsafeMode = true;
bool InputSignalExist = false;

#define LED_SIGNAL_FOUND      250
#define LED_SIGNAL_NOTFOUND   1000
#define LED    				        5,B // declare LED in PCB5 (D13)
#define FAILSAFE_BUTTON       3,C // button on A3
/*
 Hardware Wiring:
 ==============
                            
 .--------.                                  
 |        |
 |        |                          
 |    GND |------- GND         
 |        |                  
       RX |------- SIGNAL_INPUT_PIN
 |        |                        
 |     BP |------- Failsafe button (nécessite trop de mémoire).
 '--------'                                 
                                   
  Pro Micro                                
                    
*/

//0 PD2 Pro Micro (use also for Serial Configuration)
uint8_t SIGNAL_INPUT_PIN = 0; //PPM,SBUS,IBUS,DSMX,RXL,SUMD and JETIEx input

uint8_t CHANNEL_NB = 8;     //8 ou 16

TinyCppmReader TinyCppmReader; 

SoftRcPulseOut myservo1;
SoftRcPulseOut myservo2;
SoftRcPulseOut myservo3;
SoftRcPulseOut myservo4;
SoftRcPulseOut myservo5;
SoftRcPulseOut myservo6;
SoftRcPulseOut myservo7;
SoftRcPulseOut myservo8;

SoftRcPulseOut myservo9;
SoftRcPulseOut myservo10;
SoftRcPulseOut myservo11;
SoftRcPulseOut myservo12;
SoftRcPulseOut myservo13;
SoftRcPulseOut myservo14;
SoftRcPulseOut myservo15;
SoftRcPulseOut myservo16;

#include <FlySkyIBus.h>

//#include <SpektrumSattelite.h>
//SpektrumSattelite Dsmx;
#include <DSMRX.h>
DSM2048 Dsmx;

//MultiWII
#include<MSP.h>
MSP msp;


boolean RunConfig = false;
uint8_t type, mode, nboutput, reverse, failsafe, pulsetype;


/***********/
/*  Coders */
/***********/
//PPM

SoftRcPulseIn pwm1;
SoftRcPulseIn pwm2;
SoftRcPulseIn pwm3;
SoftRcPulseIn pwm4;
SoftRcPulseIn pwm5;
SoftRcPulseIn pwm6;
SoftRcPulseIn pwm7;
SoftRcPulseIn pwm8;

//IBUS
#define IBUS_FRAME_LENGTH 0x20                                  // iBus packet size (2 byte header, 14 channels x 2 bytes, 2 byte checksum)
#define IBUS_COMMAND40 0x40                                     // Command is always 0x40
#define IBUS_MAXCHANNELS 14                                     // iBus has a maximum of 14 channels
#define IBUS_DEFAULT_VALUE (uint16_t)1500
enum syncStates {
  CALC_CHANNELS, // Channel calculation
  PROCESS_PULSE, // Process pulse data
};
volatile enum syncStates syncState = CALC_CHANNELS;
byte serial_buffer[IBUS_FRAME_LENGTH] = { 0 };
int buffer_index = 0;
#define PPM_MAX_CHANNELS 10
volatile uint16_t ppm_channel_data[PPM_MAX_CHANNELS] = { 0 };


void setup()
{
  float v = vcc.Read_Volts();
  if (v <=4) LowPower = true;
  
  out(LED);             // set LED as an output
  in(FAILSAFE_BUTTON);
  pullup(FAILSAFE_BUTTON);
  
  while (!Serial);// wait for serial port to connect.  
  Serial.begin(115200); 

  Serial << F("Version:")<< EEPROMReadFloat(500) << endl;
  if (EEPROMReadFloat(500) != VERSION_DECODER)
  {
    //waitMs(500);
    Serial <<F("Def ault mode set: PPM,8 outputs,9-16,failsafe off")<< endl;
    EEPROM.write(0,0);// Decoder
    EEPROM.write(1,1);// PPM
    EEPROM.write(2,0);// 8 servos
    EEPROM.write(3,0);// define channel 9 to 16 on D2-D9
    EEPROM.write(5,0);// failsafe Off
    EEPROMWriteFloat(500,VERSION_DECODER);
  }
  
  if (RunConfig == false)
  {
    String sdata="";
    Serial << F("Wait Return");
    byte ch;
    while(millis() - startedWaiting <= 5000) //waiting 5s return key
    { 
      /* Check 1s */
      if(millis()-started1s>=1000)
      {
        Serial << F(".");started1s=millis();
      }
      if(Serial.available() > 0)
      {
        ch = Serial.read();
        sdata += (char)ch;
        if (ch=='\r')
        {
          sdata.trim(); // Process command in sdata.
          sdata = ""; // Clear the string ready for the next command.
          RunConfig = true;
          break;
        }        
      }   
    }
  }

//*********************
//  RunConfig = true;
//*********************

  (RunConfig == true?Serial << endl << endl << F("Configuration mode is actived") << endl:Serial << endl << endl << F("Starting without configuration") << endl);
  
  type = EEPROM.read(0);// 0 Decoder, 1 Coder
  mode = EEPROM.read(1);// 1 PPM, 2 SBUS, 3 IBUS ...
  nboutput = EEPROM.read(2);// 8 or 16 servos
  reverse = EEPROM.read(3);// define channel 9 to 16 on D2-D9 or D10-A4
  pulsetype = EEPROM.read(4);//define pulse neg or pos for output ppm
  failsafe = EEPROM.read(5);

  //usbDebug = EEPROM.read(4);//debug on/off  
    
  if (type == 0)
  {
    Serial << F("Decoder in use") << endl;
    (failsafe == 0?Serial << F("Failsafe is Off") << endl:Serial << F("Failsafe is On") << endl);
  }  
  else
  {
    Serial << F("Coder in use") << endl;
  }
  
  if (nboutput == 0)
  {
    CHANNEL_NB = 8;
    (reverse == 0?Serial << F("8 outputs mode (Use D2-D9)") << endl:Serial << F("8 outputs mode (Use D10-A4)") << endl);
  }
  else
  {
    CHANNEL_NB = 16;
    if (reverse == 0)
    {
      
      Serial << F("16 outputs mode") << endl;
      Serial << F("Use D2-D9 for first 8 outputs") << endl;
      Serial << F("Use D10-A4 for second 8 outputs") << endl;
    }
    else
    {
      Serial << F("16 outputs mode") << endl;
      Serial << F("Use D10-A4 for first 8 outputs") << endl;
      Serial << F("Use D2-D9 for second 8 outputs") << endl;
    }    
  }
 
  switch (mode)
  {
    case 1:
      blinkNTime(1,125,250);
      Serial << F("PPM mode in use");if (type == 0)Serial << endl;
      if (type== 1) (pulsetype == 0?Serial << F(" with POSITIVE pulses") << endl:Serial << F(" with NEGATIVE pulses") << endl);
      if (RunConfig == false)
      {
        if (type==0)
        {
          Serial.end();
          TinyCppmReader.attach(SIGNAL_INPUT_PIN); // Attach TinyPpmReader to SIGNAL_INPUT_PIN pin 
        }
        else
        {
//       PPM output pin is imposed by hardware and is target dependant:
//(The user has to define Timer and Channel to use in TinyPpmGen.h file of the library)
//
//           - ATmega328P (Arduino UNO):
//           TIMER(0), CHANNEL(A) -> OC0A -> PD6 -> Pin#6
//           TIMER(0), CHANNEL(B) -> OC0B -> PD5 -> Pin#5
//          >>>>> TIMER(2), CHANNEL(A) -> OC2A -> PB3 -> Pin#11 here is used as output <<<<<
//           TIMER(2), CHANNEL(B) -> OC2B -> PD3 -> Pin#3 

//           - ATmega32U4 (Arduino Leonardo, Micro and Pro Micro):
//           TIMER(0), CHANNEL(A) -> OC0A -> PB7 -> Pin#11 (/!\ pin not available on connector of Pro Micro /!\)
//           TIMER(0), CHANNEL(B) -> OC0B -> PD0 -> Pin#3
          (pulsetype == 0?TinyCppmGen.begin(TINY_CPPM_GEN_POS_MOD, CHANNEL_NB):TinyCppmGen.begin(TINY_CPPM_GEN_NEG_MOD, CHANNEL_NB));// Change TINY_PPM_GEN_POS_MOD to TINY_PPM_GEN_NEG_MOD for NEGative PPM modulation
        }
      }
      break;
    case 2:
      blinkNTime(2,125,250);
      Serial << F("SBUS mode in use") << endl;
      if (RunConfig == false)
      {
        Serial.flush();delay(500);
        Serial.begin(SBUS_RX_SERIAL_CFG);
        RcBusRx.serialAttach(&Serial);        
        RcBusRx.setProto(RC_BUS_RX_SBUS);
        if (type == 1)
        {
          SBusTx.serialAttach(&Serial, SBUS_TX_NORMAL_TRAME_RATE_MS); // Attach the SBUS generator to the Serial with SBUS_TX_NORMAL_TRAME_RATE_MS or SBUS_TX_HIGH_SPEED_TRAME_RATE_MS 
        }
      }
      break;
    case 3:
      blinkNTime(3,125,250);
      Serial << F("IBUS mode in use") << endl;
      if (RunConfig == false)
      {
        Serial.flush();
        Serial.begin(IBUS_RX_SERIAL_CFG);
        RcBusRx.serialAttach(&Serial);        
        RcBusRx.setProto(RC_BUS_RX_IBUS);
      }
      break;
    case 4:
      blinkNTime(4,125,250);
      Serial << F("DSMX mode in use") << endl;
      if (RunConfig == false)
      {
        Serial.flush(); // wait for last transmitted data to be sent
        Serial.begin(115200);
      }
      break;
    case 5:
      blinkNTime(5,125,250);
      Serial << F("SRLX mode in use") << endl;
      if (RunConfig == false)
      {
        Serial.flush(); // wait for last transmitted data to be sent
        Serial.begin(SRXL_RX_SERIAL_CFG);
        RcBusRx.serialAttach(&Serial);        
        RcBusRx.setProto(RC_BUS_RX_SRXL);
      }
      break;
    case 6:
      blinkNTime(6,125,250);
      Serial << F("SUMD mode in use") << endl;
      if (RunConfig == false)
      {
        Serial.flush();
        Serial.begin(SUMD_RX_SERIAL_CFG);
        RcBusRx.serialAttach(&Serial);        
        RcBusRx.setProto(RC_BUS_RX_SUMD);
      }
      break;
    case 7:
      blinkNTime(7,125,250);
      Serial << F("JETIEx mode in use") << endl;
      if (RunConfig == false)
      {
        Serial.flush();
        Serial.begin(JETI_RX_SERIAL_CFG);
        RcBusRx.serialAttach(&Serial);        
        RcBusRx.setProto(RC_BUS_RX_JETI);
      }
      break;
    case 8:
      blinkNTime(8,125,250);
      Serial << F("MultiWii mode in use") << endl;
      if (RunConfig == false)
      {
        Serial.flush();
        Serial.begin(115200);
        msp.begin(Serial);
      }
      break;      
  }

  if (reverse == 0)
  {
    myservo1.attach(2);//PD2
    myservo2.attach(3);//PD3
    myservo3.attach(4);//PD4
    myservo4.attach(5);//PD5
    myservo5.attach(6);//PD6
    myservo6.attach(7);//PB7
    myservo7.attach(8);//PB0
    myservo8.attach(9);//PB1
    if (CHANNEL_NB == 16){
      myservo9.attach(10);//PB2
      myservo10.attach(11);//PB3
      myservo11.attach(12);//PB4
      myservo12.attach(A0);//PC0
      myservo13.attach(A1);//PC1
      myservo14.attach(A2);//PC2
      myservo15.attach(A4);//PC3
      myservo16.attach(A5);//PC4
    }
  }
  else
  {
    myservo1.attach(10);//PB2
    myservo2.attach(11);//PB3
    myservo3.attach(12);//PB4
    myservo4.attach(A0);//PC0
    myservo5.attach(A1);//PC1
    myservo6.attach(A2);//PC2
    myservo7.attach(A4);//PC3
    myservo8.attach(A5);//PC4
    if (CHANNEL_NB == 16){
      myservo9.attach(2);//PD2
      myservo10.attach(3);//PD3
      myservo11.attach(4);//PD4
      myservo12.attach(5);//PD5
      myservo13.attach(6);//PD6
      myservo14.attach(7);//PB7
      myservo15.attach(8);//PB0
      myservo16.attach(9);//PB1
    }
 
  }

  if (type == 1)
  {
     pwm1.attach(2);//PD2
     pwm2.attach(3);//PD3
     pwm3.attach(4);//PD4
     pwm4.attach(5);//PD5
     pwm5.attach(6);//PD6
     pwm6.attach(7);//PB7
     pwm7.attach(8);//PB0
     pwm8.attach(9);//PB1
  }
  
  //waitMs(1000);
#if !defined(__AVR_ATmega32U4__)
  //writeFailsafeTest();
#endif
}//setup

void loop()
{
  if (LowPower == true)// if Vcc < 4v
  {
    // Blink each 250ms if PPM found on pin 2
    blinkNTime(5,LED_SIGNAL_FOUND,LED_SIGNAL_FOUND);
    waitMs(1000);
    blinkNTime(1,LED_SIGNAL_FOUND,LED_SIGNAL_FOUND);
    waitMs(1000);
  }
  
  if (RunConfig == true)
  {
      (type==0?handleSerialDecoder():handleSerialCoder());
      
      //h Help
      //q quit
      //0 set Decoder mode
      //1 set Coder mode
      //n set 8/16 outputs mode
      //r first 8 outputs changed
      //p set PPM mode
      //s set SBUS mode
      //i set IBUS mode
      //d set DSMX mode
      //m set SRLX mode
      //u set SUMD mode
      //j set JETIEx mode
      //k set MULTIWII mode
      //f set Failsafe values
      
      //e reset EEPROM (command hidden)   
  }

  if ((RunConfig == false) && (LowPower == false))
  { if (type == 0)
    {  
      if (mode == 1)//PPM
      {
        if (TinyCppmReader.isSynchro())
        {
          InputSignalExist = true;
          //Idx=TinyPpmReader.width_us(1);Serial.print(F("Ch1"));Serial.print(F("="));Serial.print(Idx);Serial.println(F(" us"));
          myservo1.write_us(TinyCppmReader.width_us(1));
          myservo2.write_us(TinyCppmReader.width_us(2));
          myservo3.write_us(TinyCppmReader.width_us(3));
          myservo4.write_us(TinyCppmReader.width_us(4));
          myservo5.write_us(TinyCppmReader.width_us(5));
          myservo6.write_us(TinyCppmReader.width_us(6));
          myservo7.write_us(TinyCppmReader.width_us(7));
          myservo8.write_us(TinyCppmReader.width_us(8));
          if (CHANNEL_NB == 16)
          {
            myservo9.write_us(TinyCppmReader.width_us(9));
            myservo10.write_us(TinyCppmReader.width_us(10));
            myservo11.write_us(TinyCppmReader.width_us(11));
            myservo12.write_us(TinyCppmReader.width_us(12));
            myservo13.write_us(TinyCppmReader.width_us(13));
            myservo14.write_us(TinyCppmReader.width_us(14));
            myservo15.write_us(TinyCppmReader.width_us(15));
            myservo16.write_us(TinyCppmReader.width_us(16));          
          }
          SoftRcPulseOut::refresh(1);
            
        }
        else
        {
          InputSignalExist = false;
//          if (failsafe == 1)
//            readFailsafeValues();
        }  
      }//PPM
     
      if (mode == 2/*SBUS*/ || mode == 3/*IBUS*/ || mode == 5/*SRXL*/ || mode == 6/*SUMD*/ || mode == 7/*JETI*/)
      { 

        RcBusRx.process(); /* Don't forget to call the SBusRx.process()! */
        if(RcBusRx.isSynchro()) /* One SBUS frame just arrived */
        {
          InputSignalExist = true;
          myservo1.write_us(RcBusRx.width_us(1));
          myservo2.write_us(RcBusRx.width_us(2));
          myservo3.write_us(RcBusRx.width_us(3));
          myservo4.write_us(RcBusRx.width_us(4));
          myservo5.write_us(RcBusRx.width_us(5));
          myservo6.write_us(RcBusRx.width_us(6));
          myservo7.write_us(RcBusRx.width_us(7));
          myservo8.write_us(RcBusRx.width_us(8));
          if (CHANNEL_NB == 16){
            myservo9.write_us(RcBusRx.width_us(9));
            myservo10.write_us(RcBusRx.width_us(10));
            myservo11.write_us(RcBusRx.width_us(11));
            myservo12.write_us(RcBusRx.width_us(12));
            myservo13.write_us(RcBusRx.width_us(13));
            myservo14.write_us(RcBusRx.width_us(14));
            myservo15.write_us(RcBusRx.width_us(15));
            myservo16.write_us(RcBusRx.width_us(16));
          }
          SoftRcPulseOut::refresh(1);      
        }
        else
        {
          InputSignalExist = false;
        }

      }//mode == 2/*SBUS*/ || mode == 3/*IBUS*/ || mode == 5/*SRXL*/ || mode == 6/*SUMD*/

      if (mode == 4)//DSMX
      {
        if (Dsmx.gotNewFrame()) 
        {
          InputSignalExist = true;
          uint16_t ch[CHANNEL_NB];
          Dsmx.getChannelValues(ch, CHANNEL_NB);
          myservo1.write_us(ch[0]);
          myservo2.write_us(ch[1]);
          myservo3.write_us(ch[2]);
          myservo4.write_us(ch[3]);
          myservo5.write_us(ch[4]);
          myservo6.write_us(ch[5]);
          myservo7.write_us(ch[6]);
          myservo8.write_us(ch[7]);
          if (CHANNEL_NB == 16)
          {
            myservo9.write_us(ch[8]);
            myservo10.write_us(ch[9]);
            myservo11.write_us(ch[10]);
            myservo12.write_us(ch[11]);
            myservo13.write_us(ch[12]);
            myservo14.write_us(ch[13]);
            myservo15.write_us(ch[14]);
            myservo16.write_us(ch[15]);  
          }
          SoftRcPulseOut::refresh(1);
          //Serial.print("Fade count = ");
          //Serial.println(rx.getFadeCount());
        }
        else  if (Dsmx.timedOut(micros())) 
        {
          InputSignalExist = false;
        }
      }//DSMX

      if (mode == 7)//MULTIWII
      {
        msp_rc_t rc;
        if (msp.request(MSP_RC, &rc, sizeof(rc))) {
          
//          uint16_t roll     = rc.channelValue[0];
//          uint16_t pitch    = rc.channelValue[1];
//          uint16_t yaw      = rc.channelValue[2];
//          uint16_t throttle = rc.channelValue[3];
          myservo1.write_us(rc.channelValue[0]);
          myservo2.write_us(rc.channelValue[1]);
          myservo3.write_us(rc.channelValue[2]);
          myservo4.write_us(rc.channelValue[3]);
          myservo5.write_us(rc.channelValue[4]);
          myservo6.write_us(rc.channelValue[5]);
          myservo7.write_us(rc.channelValue[6]);
          myservo8.write_us(rc.channelValue[7]);
          if (CHANNEL_NB == 16)
          {
            myservo9.write_us(rc.channelValue[8]);
            myservo10.write_us(rc.channelValue[9]);
            myservo11.write_us(rc.channelValue[10]);
            myservo12.write_us(rc.channelValue[11]);
            myservo13.write_us(rc.channelValue[12]);
            myservo14.write_us(rc.channelValue[13]);
            myservo15.write_us(rc.channelValue[14]);
            myservo16.write_us(rc.channelValue[15]);  
          }
          SoftRcPulseOut::refresh(1);
        }              
      }
       
    }//type 0

    if (type == 1)
    {
      switch (mode)
      {
        case 1://PPM out       
          TinyCppmGen.setChWidth_us(1, pwm1.width_us()); 
          TinyCppmGen.setChWidth_us(2, pwm2.width_us());
          TinyCppmGen.setChWidth_us(3, pwm3.width_us()); 
          TinyCppmGen.setChWidth_us(4, pwm4.width_us());
          TinyCppmGen.setChWidth_us(5, pwm5.width_us()); 
          TinyCppmGen.setChWidth_us(6, pwm6.width_us()); 
          TinyCppmGen.setChWidth_us(7, pwm7.width_us()); 
          TinyCppmGen.setChWidth_us(8, pwm8.width_us()); 
          break;
        case 2://SBUS out
          SBusTx.width_us(1, pwm1.width_us());
          SBusTx.width_us(2, pwm2.width_us());
          SBusTx.width_us(3, pwm3.width_us());
          SBusTx.width_us(4, pwm4.width_us());
          SBusTx.width_us(5, pwm5.width_us());
          SBusTx.width_us(6, pwm6.width_us());
          SBusTx.width_us(7, pwm7.width_us());
          SBusTx.width_us(8, pwm8.width_us());
          SBusTx.sendChannels();
          break; 
        case 3://IBUS out
          WriteSerialToIBUS();
          break;   
      }
    }//type 1


//    while((get(FAILSAFE_BUTTON) == LOW) && (InFailsafeMode == true) && (failsafe == 1))
//    {
//      InFailsafeMode = false;
//      //write a value from 0 to 180
//      EEPROMWriteInt(100,myservo1.read());//PD2
//      EEPROMWriteInt(102,myservo2.read());//PD3
//      EEPROMWriteInt(104,myservo3.read());//PD4
//      EEPROMWriteInt(106,myservo4.read());//PD5
//      EEPROMWriteInt(108,myservo5.read());//PD6
//      EEPROMWriteInt(110,myservo6.read());//PB7
//      EEPROMWriteInt(112,myservo7.read());//PB0
//      EEPROMWriteInt(114,myservo8.read());//PB1
//      EEPROMWriteInt(116,myservo9.read());//PB2
//      EEPROMWriteInt(118,myservo10.read());//PB3
//      EEPROMWriteInt(120,myservo11.read());//PB4
//      EEPROMWriteInt(122,myservo12.read());//PC0
//      EEPROMWriteInt(124,myservo13.read());//PC1
//      EEPROMWriteInt(126,myservo14.read());//PC2
//      EEPROMWriteInt(128,myservo15.read());//PC3
//      EEPROMWriteInt(130,myservo16.read());//PC4
//    }
    
  }//runconfig


  if(InputSignalExist == true)
  {
    // Blink each 250ms if IBUS found on Rx pin
    if(millis()-LedStartMs>=LED_SIGNAL_FOUND)
    {
      flip(LED);
      LedStartMs=millis(); // Restart the Chrono for the LED 
    }              
  }
  else
  {
    // Blink each 1s if IBUS not found on Rx pin
    if(millis()-LedStartMs>=LED_SIGNAL_NOTFOUND)
    {
      flip(LED);
      LedStartMs=millis(); // Restart the Chrono for the LED 
    }            
  }

  
}//loop


void handleSerialDecoder() {
  // we only care about two characters to change the pwm
  if (Serial.available() > 0) {
    switch (Serial.read()) {
      case 'h':
        Serial << F("h Help") << endl;
        Serial << F("q quit") << endl;
        Serial << F("0 set Decoder mode") << endl;
        Serial << F("1 set Coder mode") << endl;
        Serial << F("n set 8/16 outputs mode") << endl;
        Serial << F("r first 8 outputs changed") << endl;
        Serial << F("p set PPM mode") << endl;
        Serial << F("s set SBUS mode") << endl;
        Serial << F("i set IBUS mode") << endl;
        Serial << F("d set DSMX mode") << endl;
        Serial << F("m set SRLX mode") << endl;
        Serial << F("u set SUMD mode") << endl;
        Serial << F("j set JETIEX mode") << endl;
        Serial << F("k set MULTIWII mode") << endl;
        Serial << F("f set Failsafe values") << endl;
      break;
      case 'q':
        Serial << F("Exit  configuration mode") << endl;
        delay(200);
        RunConfig = false; 
      break;
      break;
      case '0':
        Serial << F("0 set Decoder mode") << endl << endl;
        EEPROM.write(0,0);
        EEPROM.write(1,1);// force PPM mode
        EEPROM.write(2,0);// force 8 ouputs mode
        EEPROM.write(3,0);// define channel 9 to 16 on D2-D9 or D10-A4
        EEPROM.write(5,0);//failsafe off
        type = EEPROM.read(0);
        mode = EEPROM.read(1);
        reverse = EEPROM.read(3);
        failsafe = EEPROM.read(5);
      break;
      case '1':
        Serial << F("1 set Coder mode") << endl << endl;
        EEPROM.write(0,1);
        EEPROM.write(1,1);// force PPM mode
        EEPROM.write(2,0);// force 8 ouputs mode
        EEPROM.write(3,0);// define channel 9 to 16 on D2-D9 or D10-A4
        EEPROM.write(5,0);//failsafe off
        type = EEPROM.read(0);
        mode = EEPROM.read(1);
        reverse = EEPROM.read(3);
        failsafe = EEPROM.read(5);
      break;
      case 'n'://select 8 or 16 outputs
        if (EEPROM.read(2) == 0)
        {
          EEPROM.write(2,1);Serial << F("16 outputs mode") << endl;
        }
        else
        {
          EEPROM.write(2,0);Serial << F("8 outputs mode") << endl;
        }
      break;
      case 'r'://change 8 first outputs
        if (EEPROM.read(3) == 0)
        {
          EEPROM.write(3,1);Serial << F("Use D10-A3 for first 8 outputs") << endl;
        }
        else
        {
          EEPROM.write(3,0);Serial << F("Use D2-D9 for first 8 outputs") << endl;
        }
        Serial << endl; 
      break;      
      case 'p':
        Serial << F("Set in PPM mode") << endl;
        EEPROM.write(1,1);  
      break;
      case 's':
        Serial << F("Set in SBUS mode") << endl;
        EEPROM.write(1,2);  
      break;
      case 'i':
        Serial << F("Set in IBUS mode") << endl;
        EEPROM.write(1,3);  
      break;
      case 'd':
        Serial << F("Set in DSMX mode") << endl;
        EEPROM.write(1,4);  
      break;
      case 'm':
        Serial << F("Set in SRLX mode") << endl;
        EEPROM.write(1,5);  
      break;
      case 'u':
        Serial << F("Set in SUMD mode") << endl;
        EEPROM.write(1,6);  
      break;
      case 'j':
        Serial << F("Set in JETIEX mode") << endl;
        EEPROM.write(1,7);  
      break;
      case 'k':
        Serial << F("Set in MULTIWII mode") << endl;
        EEPROM.write(1,8);  
      break;      
      case 'f':
        if (EEPROM.read(5) == 0)
        {
          EEPROM.write(5,1);Serial << F("Failsafe mode is On") << endl;
          uint8_t g = 1;
          for (uint8_t f = 0; f <= 30; f +=2)
          {
            Serial << F("Servo") << g << F("=") << EEPROMReadInt(100+f) << endl;
            g +=1;//Serial.print(F("(address"));Serial.print(100+f);Serial.println(F(")"));
          }
          Serial << endl;
        }
        else
        {
          EEPROM.write(5,0);Serial << F("Failsafe mode is Off") << endl;
        }
      break;
      case 'e':
        Serial << endl;
        for (int i = 0 ; i < EEPROM.length() ; i++) {
          EEPROM.write(i, 0);
        }
        Serial << F("Reset EEPROM Done") << endl << endl;
      break;
    }

  }
}

void handleSerialCoder() {
  // we only care about two characters to change the pwm
  if (Serial.available() > 0) {
    switch (Serial.read()) {
      case 'h':
        Serial << F("h Help") << endl;
        Serial << F("q quit") << endl;
        Serial << F("0 set Decoder mode") << endl;
        Serial << F("1 set Coder mode") << endl;
        //Serial.println(F("n set 8/16 outputs mode"));
        //Serial.println(F("r first 8 outputs changed"));
        Serial << F("p set PPM pos/neg pulse type") << endl;
        Serial << F("s set SBUS mode") << endl;
        Serial << F("i set IBUS mode") << endl << endl;
        //Serial.println(F("d set DSMX mode"));
        //Serial.println(F("m set SRLX mode"));
        //Serial.println(F("f set Failsafe values"));
        //Serial.println();
      break;
      case 'q':
        Serial << F("Exit setting mode") << endl;
        delay(200);
        RunConfig = false; 
      break;
      case '0':
        Serial << F("0 set Decoder mode") << endl << endl;
        EEPROM.write(0,0);
        EEPROM.write(1,1);// force PPM mode
        EEPROM.write(2,0);// force 8 ouputs mode
        EEPROM.write(3,0);// define channel 9 to 16 on D2-D9 or D10-A4
        EEPROM.write(5,0);//failsafe off
        type = EEPROM.read(0);
        mode = EEPROM.read(1);
        reverse = EEPROM.read(3);
        failsafe = EEPROM.read(5);
      break;
      case '1':
        Serial << F("1 set Coder mode") << endl << endl;
        EEPROM.write(0,1);
        EEPROM.write(1,1);EEPROM.write(4,0);// force PPM mode and positive pulse
        EEPROM.write(2,0);// force 8 ouputs mode
        EEPROM.write(3,0);// define channel 9 to 16 on D2-D9 or D10-A4
        EEPROM.write(5,0);//failsafe off
        type = EEPROM.read(0);
        mode = EEPROM.read(1);
        reverse = EEPROM.read(3);
        failsafe = EEPROM.read(5);
      break;
      case 'n'://select 8 or 16 outputs
        if (EEPROM.read(2) == 0)
        {
          EEPROM.write(2,1);Serial << F("16 outputs mode") << endl;
        }
        else
        {
          EEPROM.write(2,0);Serial << F("8 outputs mode") << endl;
        }
      break;
      case 'r'://change 8 first outputs
        if (EEPROM.read(3) == 0)
        {
          EEPROM.write(3,1);Serial << F("Use D10-A3 for first 8 outputs") << endl;
        }
        else
        {
          EEPROM.write(3,0);Serial << F("Use D2-D9 for first 8 outputs") << endl << endl;
        }
      break;      
      case 'p':
        if (EEPROM.read(4) == 0)
        {
          EEPROM.write(4,1);Serial << F("Set in PPM negative pulse mode") << endl;
        }
        else
        {
          EEPROM.write(4,0);Serial << F("Set in PPM positive pulse mode") << endl;
        }
        pulsetype = EEPROM.read(4);
      break;
      case 's':
        Serial << F("Set in SBUS mode") << endl << endl;
        EEPROM.write(1,2);  
      break;
      case 'i':
        Serial << F("Set in IBUS mode") << endl << endl;
        EEPROM.write(1,3);  
      break;
/*      case 'd':
        Serial.print(F("Set in DSMX mode"));
        Serial.println();
        EEPROM.write(1,4);  
      break;
      case 'm':
        Serial.print(F("Set in SRLX mode"));
        Serial.println();
        EEPROM.write(1,5);  
      break;
      case 'f':
        if (EEPROM.read(5) == 0)
        {
          EEPROM.write(5,1);Serial.println(F("Failsafe mode is On"));
          uint8_t g = 1;
          for (uint8_t f = 0; f <= 30; f +=2)
          {
            Serial.print(F("Servo"));Serial.print(g);Serial.print(F("="));Serial.println(EEPROMReadInt(100+f));g +=1;//Serial.print(F("(address"));Serial.print(100+f);Serial.println(F(")"));
          }
          Serial.println();
        }
        else
        {
          EEPROM.write(5,0);Serial.println(F("Failsafe mode is Off"));
        }
      break;

      case 'e':
        Serial.println(F("Reset EEPROM mode"));
        for (int i = 0 ; i < EEPROM.length() ; i++) {
          EEPROM.write(i, 0);
        }
        Serial.println(F("Done."));Serial.println();
      break;*/
    }

  }
}

void waitMs(unsigned long timetowait)
{
  static unsigned long beginTime = 0;
  static unsigned long lastTime = 0;
  beginTime = millis();
  lastTime = beginTime + timetowait;
  do
  {
  }
  while (lastTime > millis());
}


// used for flashing a pin
void blinkNTime(int count, int onInterval, int offInterval)
{
  byte i;
  for (i = 0; i < count; i++) 
  {
    waitMs(offInterval);
    on(LED);      //     turn on LED//digitalWrite(LED_PIN,HIGH);
    waitMs(onInterval);
    off(LED);      //     turn on LED//digitalWrite(LED_PIN,LOW);  
  }
}

void EEPROMWriteInt(int address, int value)
{
  byte two = (value & 0xFF);
  byte one = ((value >> 8) & 0xFF);
  
  EEPROM.update(address, two);
  EEPROM.update(address + 1, one);
}
 
int EEPROMReadInt(int address)
{
  long two = EEPROM.read(address);
  long one = EEPROM.read(address + 1);
 
  return ((two << 0) & 0xFFFFFF) + ((one << 8) & 0xFFFFFFFF);
}

float EEPROMReadFloat(unsigned int addr)
{
  union
  {
    byte b[4];
    float f;
  } data;
  for(int i = 0; i < 4; i++)
  {
    data.b[i] = EEPROM.read(addr+i);
  }
  return data.f;
}
void EEPROMWriteFloat(unsigned int addr, float x)
{
  union
  {
    byte b[4];
    float f;
  } data;
  data.f = x;
  for(int i = 0; i < 4; i++)
  {
    EEPROM.write(addr+i, data.b[i]);
  }
}
//void SbusDebug()
//{
//    // debug lecture des 16 + 2 voies récuperer sur la sortie serie
//   for(int x = 0 ; x < 19 ; x++)
//   {
//    if ( x == 0 ) 
//    {
//      if (voie[0] == 0) 
//      {
//        portOne.print("FS,");  // Failsafe
//      }
//      else
//      {
//        portOne.print("NO,");  // Normal
//      }
//    }
//    else
//    { 
//      portOne.print(voie[x],DEC);
//      portOne.print(",");
//    } // x == 0
//   }// for
//   portOne.println();
//}

#if !defined(__AVR_ATmega32U4__)
void writeFailsafeTest()
{
  //write a value from 0 to 180
  for (uint8_t f = 100; f <= 130; f +=2)
  {
    EEPROMWriteInt(f,10);
  }
}

void readFailsafeValues()
{
    //write a value from 0 to 180
    myservo1.write(EEPROMReadInt(100));//PD2
    myservo2.write(EEPROMReadInt(102));//PD3
    myservo3.write(EEPROMReadInt(104));//PD4
    myservo4.write(EEPROMReadInt(106));//PD5
    myservo5.write(EEPROMReadInt(108));//PD6
    myservo6.write(EEPROMReadInt(110));//PB7
    myservo7.write(EEPROMReadInt(112));//PB0
    myservo8.write(EEPROMReadInt(114));//PB1
    myservo9.write(EEPROMReadInt(116));//PB2
    myservo10.write(EEPROMReadInt(118));//PB3
    myservo11.write(EEPROMReadInt(120));//PB4
    myservo12.write(EEPROMReadInt(122));//PC0
    myservo13.write(EEPROMReadInt(124));//PC1
    myservo14.write(EEPROMReadInt(126));//PC2
    myservo15.write(EEPROMReadInt(128));//PC3
    myservo16.write(EEPROMReadInt(130));//PC4
}
#endif

/***********/
/*  Coders */
/***********/
void WriteSerialToIBUS()
{

  uint16_t ibus_checksum = ((uint16_t)0xFFFF);
  
  //ppm ti ibus
//  for (uint8_t Idx = 0; Idx<=CHANNEL_NB; Idx++)
//  {
//    ppm_channel_data[Idx] = TinyPpmReader.width_us(Idx+1);
//  }

  //pwm to ibus
  ppm_channel_data[0] = pwm1.width_us();
  ppm_channel_data[1] = pwm2.width_us();
  ppm_channel_data[2] = pwm3.width_us();
  ppm_channel_data[3] = pwm4.width_us();
  ppm_channel_data[4] = pwm5.width_us();
  ppm_channel_data[5] = pwm6.width_us();
  ppm_channel_data[6] = pwm7.width_us();
  ppm_channel_data[7] = pwm8.width_us();
  
  switch (syncState) {
    case CALC_CHANNELS:
      //      if (channelCount <= 0 ) {
      //        Serial.println(F("Calculating Channels"));
      //      }
      //      else {
      //        Serial.println("Channel Count: " + ((String)channelCount));
      //      }
      break;
    case PROCESS_PULSE:
  
      //      Serial.println("Channel Count: " + ((String)currChannelCount));
  
      buffer_index = 0;
  
      // Write the IBus buffer length
      serial_buffer[buffer_index++] = (byte)IBUS_FRAME_LENGTH;
      // Write the IBus Command 0x40
      serial_buffer[buffer_index++] = (byte)IBUS_COMMAND40;
  
      // Write the IBus channel data to the buffer
      for (int i = 0; i < min(CHANNEL_NB, IBUS_MAXCHANNELS); i++) {
        serial_buffer[buffer_index++] = (byte)(ppm_channel_data[i] & 0xFF);
        serial_buffer[buffer_index++] = (byte)((ppm_channel_data[i] >> 8) & 0xFF);
      }
  
      // Fill the remaining buffer channels with the default value
      if (CHANNEL_NB < IBUS_MAXCHANNELS) {
        for (int i = 0; i < IBUS_MAXCHANNELS - CHANNEL_NB; i++) {
          serial_buffer[buffer_index++] = (byte)(IBUS_DEFAULT_VALUE & 0xFF);
          serial_buffer[buffer_index++] = (byte)((IBUS_DEFAULT_VALUE >> 8) & 0xFF);
        }
      }
  
      // Calculate the IBus checksum
      for (int i = 0; i < buffer_index; i++) {
        ibus_checksum -= (uint16_t)serial_buffer[i];
      }
  
      // Write the IBus checksum to the buffer
      serial_buffer[buffer_index++] = (byte)(ibus_checksum & 0xFF);
      serial_buffer[buffer_index++] = (byte)((ibus_checksum >> 8) & 0xFF);
  
      // Write the buffer to the Serial pin
      Serial.write(serial_buffer, buffer_index);
  
      buffer_index = 0;
  
      break;
    default:
      break;
  }

}
