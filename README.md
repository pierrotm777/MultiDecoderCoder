# MultiDecoderCoder
/* This code is running only with Arduino Uno,Pro Mini ou Pro Micro */

/* Option JetiEx is running only with Pro Micro */
/* Option Failsafe is running only with Pro Mini */

/* Decoder for maxi 16 servos for signals protols:
 - PPM based on libraries RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 
 - SBUS based on libraries FUTABA_SBUS https://github.com/mikeshub/FUTABA_SBUS/tree/master/FUTABA_SBUS
    A signal's reverse is needed (http://www.ernstc.dk/arduino/sbus.html)
 - IBUS based on librarie https://github.com/aanon4/FlySkyIBus 
 - DSMX based on librarie https://github.com/Quarduino/SpektrumSatellite
 - SRXL based on libraries RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 
 - SUMD based on exemple https://github.com/gregd72002/sumd2ppm
 - JETIEX based on librarie https://github.com/Sepp62/JetiExBus
*/

/*
 * Coder convert 8 output servos PWM in:
 - PPM based on libraries RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 
 - SBUS based on libraries RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 
 - IBUS
 */
