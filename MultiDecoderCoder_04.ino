/* Ce code ne fonctionne qu'avec un Arduino Uno,Pro Mini ou Pro Micro */

/* L'option JetiEx ne fonctionne qu'avec un Pro Micro */
/* L'option Failsafe ne fonctionne qu'avec un Pro Mini */

/* Décodeur pour commander jusqu'à 16 servos à partir d'un signal type:
 - PPM basé sur les librairies RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 
 - SBUS basé sur les librairies FUTABA_SBUS https://github.com/mikeshub/FUTABA_SBUS/tree/master/FUTABA_SBUS
    Un inverseur du signal est nécessaire (http://www.ernstc.dk/arduino/sbus.html)
 - IBUS basé sur la librairie https://github.com/aanon4/FlySkyIBus 
 - DSMX basé sur la librairie https://github.com/Quarduino/SpektrumSatellite
 - SRXL basé sur les librairies RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 
 - SUMD basé sur l'exemple https://github.com/gregd72002/sumd2ppm
 - JETIEX basé sur la librairie https://github.com/Sepp62/JetiExBus
*/

/*
 * Codeur permettant de convertir 8 sorties servos PWM en:
 - PPM basé sur les librairies RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 
 - SBUS basé sur les librairies RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 
 - IBUS
 */
 

//#define SERIAL_SUM_PPM         PITCH,YAW,THROTTLE,ROLL,AUX1,AUX2,AUX3,AUX4,8,9,10,11 //For Graupner/Spektrum
//#define SERIAL_SUM_PPM         ROLL,PITCH,THROTTLE,YAW,AUX1,AUX2,AUX3,AUX4,8,9,10,11 //For Robe/Hitec/Futaba
//#define SERIAL_SUM_PPM         ROLL,PITCH,YAW,THROTTLE,AUX1,AUX2,AUX3,AUX4,8,9,10,11 //For Multiplex
//#define SERIAL_SUM_PPM         PITCH,ROLL,THROTTLE,YAW,AUX1,AUX2,AUX3,AUX4,8,9,10,11 //For some Hitec/Sanwa/Others


#include <Rcul.h>
#include <TinyPinChange.h>
#include <SoftRcPulseOut.h>
#include <SoftRcPulseIn.h>
#include <TinyPpmReader.h>
#include <TinyPpmGen.h>
#include <EEPROM.h>
#include <SBusTx.h>
#include <SumdRx.h>
#include <Streaming.h>

float VERSION_DECODER = 0.4;

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

//#define DEBUGSBUS


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

int buf[25];
int voie[18];
int memread;
int cpt;
int s1,s2,s3,s4,s5,s6,s7,s8;
int s9,s10,s11,s12,s13,s14,s15,s16;

#include <FlySkyIBus.h>

//#include <SpektrumSattelite.h>
//SpektrumSattelite Dsmx;
#include <DSMRX.h>
DSM2048 Dsmx;

#include <SrxlRx.h>


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

//SUMD
uint8_t rssi;
uint8_t rx_count;
uint16_t channel_count;
uint16_t channels[16];

#if defined(__AVR_ATmega32U4__)
//JetiEx
#include <JetiExBusProtocol.h>
JetiExBusProtocol exBus;
enum
{
  ID_VOLTAGE = 1
};
JETISENSOR_CONST sensors[] PROGMEM =
{
  // id             name          unit         data type             precision 
  { ID_VOLTAGE,    "Voltage",    "V",         JetiSensor::TYPE_14b, 1 },
  { 0 } // end of array
};
#endif

void setup()
{
  float v = vcc.Read_Volts();
  if (v <=4) LowPower = true;
  
  out(LED);             // set LED as an output
  in(FAILSAFE_BUTTON);
  pullup(FAILSAFE_BUTTON);
  
  Serial.begin(115200); 
  while (!Serial);// wait for serial port to connect.
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
    while(millis() - startedWaiting <= 3000) //waiting 3s return key
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
          TinyPpmReader.attach(SIGNAL_INPUT_PIN); // Attach TinyPpmReader to SIGNAL_INPUT_PIN pin 
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
          (pulsetype == 0?TinyPpmGen.begin(TINY_PPM_GEN_POS_MOD, CHANNEL_NB):TinyPpmGen.begin(TINY_PPM_GEN_NEG_MOD, CHANNEL_NB));// Change TINY_PPM_GEN_POS_MOD to TINY_PPM_GEN_NEG_MOD for NEGative PPM modulation
        }
      }
      break;
    case 2:
      blinkNTime(2,125,250);
      Serial << F("SBUS mode in use") << endl;
      if (RunConfig == false)
      {
        Serial.flush();delay(500); // wait for last transmitted data to be sent
        Serial.begin(100000, SERIAL_8E2);// Choose your serial first: SBUS works at 100 000 bauds 
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
        Serial.end();
        IBus.begin(Serial);
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
        Serial.begin(115200);
        SrxlRx.serialAttach(&Serial);
      }
      break;
    case 6:
      blinkNTime(6,125,250);
      Serial << F("SUMD mode in use") << endl;
      if (RunConfig == false)
      {
        Serial.flush();
        Serial.begin(115200);
        for (uint8_t i=0;i<CHANNEL_NB;i++)
        {
          channels[i] = 1500;
        }
      }
      break;
#if defined(__AVR_ATmega32U4__)
    case 7:
      blinkNTime(7,125,250);
      Serial << F("JETIEx mode in use") << endl;
      if (RunConfig == false)
      {
        Serial.flush();
        exBus.SetDeviceId(0x76, 0x32); // 0x3276
        exBus.Start("EX Bus", sensors, 0 ); // com port: 1..3 for Teeny, 0 or 1 for AtMega328PB UART0/UART1, others: not used 
      }
      break;
#endif
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
      //f set Failsafe values
      
      //e reset EEPROM (command hidden)   
  }

  if ((RunConfig == false) && (LowPower == false))
  { if (type == 0)
    {  
      if (mode == 1)//PPM
      {
        if (TinyPpmReader.isSynchro())
        {
          //Idx=TinyPpmReader.width_us(1);Serial.print(F("Ch1"));Serial.print(F("="));Serial.print(Idx);Serial.println(F(" us"));
          myservo1.write_us(TinyPpmReader.width_us(1));
          myservo2.write_us(TinyPpmReader.width_us(2));
          myservo3.write_us(TinyPpmReader.width_us(3));
          myservo4.write_us(TinyPpmReader.width_us(4));
          myservo5.write_us(TinyPpmReader.width_us(5));
          myservo6.write_us(TinyPpmReader.width_us(6));
          myservo7.write_us(TinyPpmReader.width_us(7));
          myservo8.write_us(TinyPpmReader.width_us(8));
          if (CHANNEL_NB == 16)
          {
            myservo9.write_us(TinyPpmReader.width_us(9));
            myservo10.write_us(TinyPpmReader.width_us(10));
            myservo11.write_us(TinyPpmReader.width_us(11));
            myservo12.write_us(TinyPpmReader.width_us(12));
            myservo13.write_us(TinyPpmReader.width_us(13));
            myservo14.write_us(TinyPpmReader.width_us(14));
            myservo15.write_us(TinyPpmReader.width_us(15));
            myservo16.write_us(TinyPpmReader.width_us(16));          
          }
          SoftRcPulseOut::refresh(1);
          
          // Blink each 250ms if PPM found on pin 2
          if(millis()-LedStartMs>=LED_SIGNAL_FOUND)
          {
            flip(LED);
            LedStartMs=millis(); // Restart the Chrono for the LED 
          }   
        }
        else
        {
          // Blink each 1s if PPM not found on pin 2
          if(millis()-LedStartMs>=LED_SIGNAL_NOTFOUND)
          {
            flip(LED);
            LedStartMs=millis(); // Restart the Chrono for the LED 
          }
#if !defined(__AVR_ATmega32U4__)
//          if (failsafe == 1)
//            readFailsafeValues();
#endif
        }  
      }//PPM
     
      if (mode == 2)//SBUS
      { 
        recupSbusdata();// conversion des données SBUS pour les 16 Voies
        while ( Serial.available() ) { // tant qu'un octet arrive sur le SBUS
          int val = Serial.read();  // lecture de l'octet
          if  (( memread == 0 ) and ( val == 15)) { // detection de la fin et debut de la trame SBUS (une trame fini par 0 et commence par 15)
            cpt = 0; // remise a zero du compteur dans la trame
          }
          memread = val; // memorisation de la dernière valeur reçu
          buf[cpt] = val; // stock la valeur reçu dans le buffer
          cpt +=1;        // incrémente le compteur
          if (cpt == 26) {cpt=0;} // au cas ou on aurait pas reçu les caractères de synchro on reset le compteur
        } // fin du while

      
        //SbusDebug();
        //ser = map(voie[3], 0, 2048, 0, 180);      // ecriture dans de la voie 3 pour la sortie servo 
        s1 = map(voie[1], -100, +100, 900, 2100);
        s2 = map(voie[2], -100, +100, 900, 2100);
        s3 = map(voie[3], -100, +100, 900, 2100);
        s4 = map(voie[4], -100, +100, 900, 2100);
        s5 = map(voie[5], -100, +100, 900, 2100);
        s6 = map(voie[6], -100, +100, 900, 2100);
        s7 = map(voie[7], -100, +100, 900, 2100);
        s8 = map(voie[8], -100, +100, 900, 2100);
        myservo1.write_us(s1);
        myservo2.write_us(s2);
        myservo3.write_us(s3);
        myservo4.write_us(s4);
        myservo5.write_us(s5);
        myservo6.write_us(s6);
        myservo7.write_us(s7);
        myservo8.write_us(s8);
        if (CHANNEL_NB == 16){    
          s9 = map(voie[9], -100, +100, 900, 2100);
          s10 = map(voie[10], -100, +100, 900, 2100);
          s11 = map(voie[11], -100, +100, 900, 2100);
          s12 = map(voie[12], -100, +100, 900, 2100);
          s13 = map(voie[13], -100, +100, 900, 2100);
          s14 = map(voie[14], -100, +100, 900, 2100);
          s15 = map(voie[15], -100, +100, 900, 2100);
          s16 = map(voie[16], -100, +100, 900, 2100);
          myservo9.write_us(s9);
          myservo10.write_us(s10);
          myservo11.write_us(s11);
          myservo12.write_us(s12);
          myservo13.write_us(s13);
          myservo14.write_us(s14);
          myservo15.write_us(s15);
          myservo16.write_us(s16);
        }
        SoftRcPulseOut::refresh(1);
      }//sbus
      
      if (mode == 3)//IBUS
      {
        IBus.loop();
        if (IBus.readChannel(0) > 0)
        {
          myservo1.write_us(IBus.readChannel(0));
          myservo2.write_us(IBus.readChannel(1));
          myservo3.write_us(IBus.readChannel(2));
          myservo4.write_us(IBus.readChannel(3));
          myservo5.write_us(IBus.readChannel(4));
          myservo6.write_us(IBus.readChannel(5));
          myservo7.write_us(IBus.readChannel(6));
          myservo8.write_us(IBus.readChannel(7));
          if (CHANNEL_NB == 16)
          {
            myservo9.write_us(IBus.readChannel(8));
            myservo10.write_us(IBus.readChannel(9));
            myservo11.write_us(IBus.readChannel(10));
            myservo12.write_us(IBus.readChannel(11));
            myservo13.write_us(IBus.readChannel(12));
            myservo14.write_us(IBus.readChannel(13));
            myservo15.write_us(IBus.readChannel(14));
            myservo16.write_us(IBus.readChannel(15));                  
          }
          SoftRcPulseOut::refresh(1);
  
          // Blink each 250ms if SBUS found on Rx pin
          if(millis()-LedStartMs>=LED_SIGNAL_FOUND)
          {
            flip(LED);
            LedStartMs=millis(); // Restart the Chrono for the LED 
          }  
        }
        else
        {
          // Blink each 1s if SBUS not found on Rx pin
          if(millis()-LedStartMs>=LED_SIGNAL_NOTFOUND)
          {
            flip(LED);
            LedStartMs=millis(); // Restart the Chrono for the LED 
          }
#if !defined(__AVR_ATmega32U4__)          
//          if (failsafe == 1)
//            readFailsafeValues();
#endif  
        }  
      }
  
 
      if (mode == 4)//DSMX
      {
        if (Dsmx.gotNewFrame()) 
        {
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
          // Blink each 250ms if DSMX found on Rx pin
          if(millis()-LedStartMs>=LED_SIGNAL_FOUND)
          {
            flip(LED);
            LedStartMs=millis(); // Restart the Chrono for the LED 
          }  
          //Serial.print("Fade count = ");
          //Serial.println(rx.getFadeCount());
        }
        else  if (Dsmx.timedOut(micros())) 
        {
          //Serial.println("*** TIMED OUT ***");
         // Blink each 1000ms if DSMX not found on Rx pin
          if(millis()-LedStartMs>=LED_SIGNAL_NOTFOUND)
          {
            flip(LED); 
            LedStartMs=millis(); // Restart the Chrono for the LED 
          }
        }
      }//DSMX
  
      if (mode == 5)//SRXL (Multiplex)
      { 
        SrxlRx.process();
        if(SrxlRx.isSynchro()) // One SRLX frame just arrived 
        {
          myservo1.write_us(SrxlRx.width_us(1));
          myservo2.write_us(SrxlRx.width_us(2));
          myservo3.write_us(SrxlRx.width_us(3));
          myservo4.write_us(SrxlRx.width_us(4));
          myservo5.write_us(SrxlRx.width_us(5));
          myservo6.write_us(SrxlRx.width_us(6));
          myservo7.write_us(SrxlRx.width_us(7));
          myservo8.write_us(SrxlRx.width_us(8));
          if (CHANNEL_NB == 16){    
            myservo9.write_us(SrxlRx.width_us(9));
            myservo10.write_us(SrxlRx.width_us(10));
            myservo11.write_us(SrxlRx.width_us(11));
            myservo12.write_us(SrxlRx.width_us(12));
            myservo13.write_us(SrxlRx.width_us(13));
            myservo14.write_us(SrxlRx.width_us(14));
            myservo15.write_us(SrxlRx.width_us(15));
            myservo16.write_us(SrxlRx.width_us(16));
          }    
          SoftRcPulseOut::refresh(1);
          // Blink each 250ms if SRXL found on Rx pin
          if(millis()-LedStartMs>=LED_SIGNAL_FOUND)
          {
            flip(LED); 
            LedStartMs=millis(); // Restart the Chrono for the LED 
          }  
        }
        else
        {
          // Blink each 1s if SRXL not found on Rx pin
          if(millis()-LedStartMs>=LED_SIGNAL_NOTFOUND)
          {
            flip(LED);
            LedStartMs=millis(); // Restart the Chrono for the LED 
          }
#if !defined(__AVR_ATmega32U4__)
//          if (failsafe == 1)
//            readFailsafeValues();
#endif
        }
  
      }//srxl

      if (mode == 6)//SUMD Graupner
      {
        if (!Serial.available()) return;
        uint8_t c = Serial.read();
        int ret = sumd_decode(c, &rssi, &rx_count, &channel_count, channels, CHANNEL_NB);
        if (ret) return;  //sumd_decode returns 0 on decoded packet
        for (uint8_t i=channel_count;i<CHANNEL_NB;i++) //unused channels
        {
          channels[i] = 1500;
        }
        myservo1.write_us(channels[1]);
        myservo2.write_us(channels[2]);
        myservo3.write_us(channels[3]);
        myservo4.write_us(channels[4]);
        myservo5.write_us(channels[5]);
        myservo6.write_us(channels[6]);
        myservo7.write_us(channels[7]);
        myservo8.write_us(channels[8]);
        if (CHANNEL_NB == 16){    
          myservo9.write_us(channels[9]);
          myservo10.write_us(channels[10]);
          myservo11.write_us(channels[11]);
          myservo12.write_us(channels[12]);
          myservo13.write_us(channels[13]);
          myservo14.write_us(channels[14]);
          myservo15.write_us(channels[15]);
          myservo16.write_us(channels[16]);   
        }
        SoftRcPulseOut::refresh(1);
        // Blink each 250ms if SUMD found on Rx pin
        if(millis()-LedStartMs>=LED_SIGNAL_FOUND)
        {
          flip(LED);
          LedStartMs=millis(); // Restart the Chrono for the LED 
        }  
      }//SUMD
     
#if defined(__AVR_ATmega32U4__)
      if (mode == 7)//JETI EX
      {
//        char bufJetiEx[30];
//        if (false)
//        // if ( exBus.HasNewChannelData() )
//        {
//          for (uint8_t i = 0; i < exBus.GetNumChannels(); i++)
//          {
//            sprintf(buf, "chan-%d: %d", i, exBus.GetChannel(i));
//            //Serial.println(buf);
//          }
//        }
        // run protocol state machine
        exBus.DoJetiExBus();
        myservo1.write_us(exBus.GetChannel(0));
        myservo2.write_us(exBus.GetChannel(1));
        myservo3.write_us(exBus.GetChannel(2));
        myservo4.write_us(exBus.GetChannel(3));
        myservo5.write_us(exBus.GetChannel(4));
        myservo6.write_us(exBus.GetChannel(5));
        myservo7.write_us(exBus.GetChannel(6));
        myservo8.write_us(exBus.GetChannel(7));
        if (CHANNEL_NB == 16){    
          myservo9.write_us(exBus.GetChannel(8));
          myservo10.write_us(exBus.GetChannel(9));
          myservo11.write_us(exBus.GetChannel(10));
          myservo12.write_us(exBus.GetChannel(11));
          myservo13.write_us(exBus.GetChannel(12));
          myservo14.write_us(exBus.GetChannel(13));
          myservo15.write_us(exBus.GetChannel(14));
          myservo16.write_us(exBus.GetChannel(15));   
        }
        SoftRcPulseOut::refresh(1);
        // Blink each 250ms if JETIEx found on Rx pin
        if(millis()-LedStartMs>=LED_SIGNAL_FOUND)
        {
          flip(LED);
          LedStartMs=millis(); // Restart the Chrono for the LED 
        }          
      }//JETIEX
#endif
      
    }//type 0

    if (type == 1)
    {
      switch (mode)
      {
        case 1://PPM out       
          TinyPpmGen.setChWidth_us(1, pwm1.width_us()); 
          TinyPpmGen.setChWidth_us(2, pwm2.width_us());
          TinyPpmGen.setChWidth_us(3, pwm3.width_us()); 
          TinyPpmGen.setChWidth_us(4, pwm4.width_us());
          TinyPpmGen.setChWidth_us(5, pwm5.width_us()); 
          TinyPpmGen.setChWidth_us(6, pwm6.width_us()); 
          TinyPpmGen.setChWidth_us(7, pwm7.width_us()); 
          TinyPpmGen.setChWidth_us(8, pwm8.width_us()); 
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
#if defined(__AVR_ATmega32U4__)
        Serial << F("j set JETIEX mode") << endl << endl;
#else
        Serial << F("f set Failsafe values") << endl << endl;
#endif
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
#if defined(__AVR_ATmega32U4__)
      case 'j':
        Serial << F("Set in JETIEX mode") << endl;
        EEPROM.write(1,7);  
      break;
#else
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
#endif
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

void recupSbusdata(void){
// récuperation des données SBUS et conversion dans le tableau des voies
// les données SBUS sont sur 8 bits et les données des voies sont sur 11 bits
// il faut donc jouer a cheval sur les octets pour calculer les voies.

  voie[1]  = ((buf[1]|buf[2]<< 8) & 0x07FF);
  voie[2]  = ((buf[2]>>3|buf[3]<<5) & 0x07FF);
  voie[3]  = ((buf[3]>>6|buf[4]<<2|buf[5]<<10) & 0x07FF);
  voie[4]  = ((buf[5]>>1|buf[6]<<7) & 0x07FF);
  voie[5]  = ((buf[6]>>4|buf[7]<<4) & 0x07FF);
  voie[6]  = ((buf[7]>>7|buf[8]<<1|buf[9]<<9) & 0x07FF);
  voie[7]  = ((buf[9]>>2|buf[10]<<6) & 0x07FF);
  voie[8]  = ((buf[10]>>5|buf[11]<<3) & 0x07FF);
 
  voie[9]  = ((buf[12]|buf[13]<< 8) & 0x07FF);
  voie[10]  = ((buf[13]>>3|buf[14]<<5) & 0x07FF);
  voie[11] = ((buf[14]>>6|buf[15]<<2|buf[16]<<10) & 0x07FF);
  voie[12] = ((buf[16]>>1|buf[17]<<7) & 0x07FF);
  voie[13] = ((buf[17]>>4|buf[18]<<4) & 0x07FF);
  voie[14] = ((buf[18]>>7|buf[19]<<1|buf[20]<<9) & 0x07FF);
  voie[15] = ((buf[20]>>2|buf[21]<<6) & 0x07FF);
  voie[16] = ((buf[21]>>5|buf[22]<<3) & 0x07FF);
 
  ((buf[23]) & 1 )      ? voie[17] = 2047 : voie[17] = 0 ;
  ((buf[23] >> 1) & 1 ) ? voie[18] = 2047 : voie[17] = 0 ;
 
  // detection du failsafe
  if ((buf[23] >> 3) & 1) {
    voie[0] = 0; // Failsafe
  }
  else
  {
    voie[0] = 1; // Normal
  }
  for(int x = 1; x<19 ; x++)
  {
    voie[x]= (lround(voie[x]/9.92) - 100);
  }

  if ((voie[1] > -100) && (voie[1] < 100))
  {
    // Blink each 250ms if SBUS found on Rx pin
    if(millis()-LedStartMs>=LED_SIGNAL_FOUND)
    {
      flip(LED);
      LedStartMs=millis(); // Restart the Chrono for the LED 
    }  
  }
  else
  {
    // Blink each 1s if SBUS not found on Rx pin
    if(millis()-LedStartMs>=LED_SIGNAL_NOTFOUND)
    {
      flip(LED);
      LedStartMs=millis(); // Restart the Chrono for the LED 
    }  
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
