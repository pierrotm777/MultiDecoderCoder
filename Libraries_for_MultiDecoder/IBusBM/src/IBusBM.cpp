/*
 * Interface to the RC IBus protocol
 * 
 * Based on original work from: https://gitlab.com/timwilkinson/FlySkyIBus
 * Extended to also handle sensors/telemetry data to be sent back to the transmitter,
 * interrupts driven and other features.
 *
 * This lib requires a hardware UART for communication
 * Another version using software serial is here https://github.com/Hrastovc/iBUStelemetry
 * 
 * Explaination of sensor/ telemetry prtocol here: 
 * https://github.com/betaflight/betaflight/wiki/Single-wire-FlySky-(IBus)-telemetry
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either  
 * version 2.1 of the License, or (at your option) any later version.
 *   
 * Created 12 March 2019 Bart Mellink
 * Updated 4 April 2019 to support ESP32
 */

#include <Arduino.h>
#include "IBusBM.h"

// pointer to the first class instance to be used to call the loop() method from timer interrupt
// will be initiated by class constructor, then daisy channed to other class instances if we have more than one
IBusBM* IBusBMfirst = NULL;


// Interrupt on timer0 - called every 1 ms
// we call the IBusSensor.loop() here, so we are certain we respond to sensor requests in a timely matter
#ifdef ARDUINO_ARCH_AVR
SIGNAL(TIMER0_COMPA_vect) {
  if (IBusBMfirst) IBusBMfirst->loop();  // gets new servo values if available and process any sensor data
}
#elif defined _VARIANT_ARDUINO_STM32_
void  onTimer(stimer_t *htim) {
  if (IBusBMfirst) IBusBMfirst->loop();  // gets new servo values if available and process any sensor data
}
#else
void  onTimer() {
  if (IBusBMfirst) IBusBMfirst->loop();  // gets new servo values if available and process any sensor data
}
#endif

/*
 *  supports max 14 channels in this lib (with messagelength of 0x20 there is room for 14 channels)

  Example set of bytes coming over the iBUS line for setting servos: 
    20 40 DB 5 DC 5 54 5 DC 5 E8 3 D0 7 D2 5 E8 3 DC 5 DC 5 DC 5 DC 5 DC 5 DC 5 DA F3
  Explanation
    Protocol length: 20
    Command code: 40 
    Channel 0: DB 5  -> value 0x5DB
    Channel 1: DC 5  -> value 0x5Dc
    Channel 2: 54 5  -> value 0x554
    Channel 3: DC 5  -> value 0x5DC
    Channel 4: E8 3  -> value 0x3E8
    Channel 5: D0 7  -> value 0x7D0
    Channel 6: D2 5  -> value 0x5D2
    Channel 7: E8 3  -> value 0x3E8
    Channel 8: DC 5  -> value 0x5DC
    Channel 9: DC 5  -> value 0x5DC
    Channel 10: DC 5 -> value 0x5DC
    Channel 11: DC 5 -> value 0x5DC
    Channel 12: DC 5 -> value 0x5DC
    Channel 13: DC 5 -> value 0x5DC
    Checksum: DA F3 -> calculated by adding up all previous bytes, total must be FFFF
 */


void IBusBM::begin(HardwareSerial& serial, int8_t timerid, int8_t rxPin, int8_t txPin) {
  #ifdef ARDUINO_ARCH_ESP32
    serial.begin(115200, SERIAL_8N1, rxPin, txPin);
  #else
    serial.begin(115200, SERIAL_8N1);
  #endif

  this->stream = &serial;
  this->state = DISCARD;
  this->last = millis();
  this->ptr = 0;
  this->len = 0;
  this->chksum = 0;
  this->lchksum = 0;

  // we need to process the iBUS sensor protocol handler frequently enough (at least once each ms) to ensure the response data
  // from the sensor is sent on time to the receiver
  // if timerid==IBUSBM_NOTIMER the user is responsible for calling the loop function
  this->IBusBMnext = IBusBMfirst;

  if (!IBusBMfirst && timerid != IBUSBM_NOTIMER) {
    #ifdef ARDUINO_ARCH_AVR
      // on AVR architectures Timer0 is already used for millis() - we'll just interrupt somewhere in the middle and call the TIMER0_COMPA_vect interrupt
      OCR0A = 0xAF;
      TIMSK0 |= _BV(OCIE0A);
    #else
      // on other architectures we need to use a time
      #if defined(ARDUINO_ARCH_ESP32) 
        hw_timer_t * timer = NULL;
        timer = timerBegin(timerid, F_CPU / 1000000L, true); // defaults to timer_id = 0; divider=80 (1 ms); countUp = true;
        timerAttachInterrupt(timer, &onTimer, true); // edge = true
        timerAlarmWrite(timer, 1000, true);  //1 ms
        timerAlarmEnable(timer);
      #elif defined(_VARIANT_ARDUINO_STM32_)
	      TIM_TypeDef * TIMER = TIM1; // Select timer, TODO convert (int8_t timerid) into: (TIM_TypeDef * TIMER = TIMx)
        static stimer_t TimHandle; // Handler for stimer
	      TimHandle.timer = TIMER; // Set TIMx instance.
	      TimerHandleInit(&TimHandle, 1000 - 1, ((uint32_t)(getTimerClkFreq(TIMER) / (1000000)) - 1)); // Set TIMx timer to 1ms 
	      attachIntHandle(&TimHandle, onTimer); // Attach onTimer interupt routine 
      #else
        // It should not be too difficult to support additional architectures as most have timer functions, but I only tested AVR and ESP32
        #error "Only support for AVR, ESP32 and STM32 architectures."
      #endif
    #endif
  }
  IBusBMfirst = this; 
}

// called from timer interrupt or mannually by user (if IBUSBM_NOTIMER set in begin())
void IBusBM::loop(void) {

  // if we have multiple instances of IBusBM, we (recursively) call the other instances loop() function
  if (IBusBMnext) IBusBMnext->loop(); 

  // only process data already in our UART receive buffer 
  while (stream->available() > 0) {
    // only consider a new data package if we have not heard anything for >3ms
    uint32_t now = millis();
    if (now - last >= PROTOCOL_TIMEGAP){
      state = GET_LENGTH;
    }
    last = now;
    
    uint8_t v = stream->read();
    switch (state) {
      case GET_LENGTH:
        if (v <= PROTOCOL_LENGTH && v > PROTOCOL_OVERHEAD) {
          ptr = 0;
          len = v - PROTOCOL_OVERHEAD;
          chksum = 0xFFFF - v;
          state = GET_DATA;
        } else {
          state = DISCARD;
        }
        break;

      case GET_DATA:
        buffer[ptr++] = v;
        chksum -= v;
        if (ptr == len) {
          state = GET_CHKSUML;
        }
        break;
        
      case GET_CHKSUML:
        lchksum = v;
        state = GET_CHKSUMH;
        break;

      case GET_CHKSUMH:
        // Validate checksum
        if (chksum == (v << 8) + lchksum) {
          // Checksum is all fine Execute command - 
          uint8_t adr = buffer[0] & 0x0f;
          if (buffer[0]==PROTOCOL_COMMAND40) {
            // Valid servo command received - extract channel data
            for (uint8_t i = 1; i < PROTOCOL_CHANNELS * 2 + 1; i += 2) {
              channel[i / 2] = buffer[i] | (buffer[i + 1] << 8);
            }
            cnt_rec++;
          } else if (adr<=NumberSensors && adr>0 && len==1) {
            // all sensor data commands go here
            // we only process the len==1 commands (=message length is 4 bytes incl overhead) to prevent the case the
            // return messages from the UART TX port loop back to the RX port and are processed again. This is extra
            // precaution as it will also be prevented by the PROTOCOL_TIMEGAP required
            delayMicroseconds(100);
            switch (buffer[0] & 0x0f0) {
              case PROTOCOL_COMMAND_DISCOVER: // 0x80, discover sensor
                cnt_poll++;  
                // echo discover command: 0x04, 0x81, 0x7A, 0xFF 
                stream->write(0x04);
                stream->write(PROTOCOL_COMMAND_DISCOVER + adr);
                chksum = 0xFFFF - (0x04 + PROTOCOL_COMMAND_DISCOVER + adr);
                break;
              case PROTOCOL_COMMAND_TYPE: // 0x90, send sensor type
                // echo sensortype command: 0x06 0x91 0x00 0x02 0x66 0xFF 
                stream->write(0x06);
                stream->write(PROTOCOL_COMMAND_TYPE + adr);
                stream->write(sensorType[adr]);
                stream->write(0x02); // always this value - unkwown
                chksum = 0xFFFF - (0x06 + PROTOCOL_COMMAND_TYPE + adr + sensorType[adr] + 2);
                break;
              case PROTOCOL_COMMAND_VALUE: // 0xA0, send sensor data
                cnt_sensor++;
                // echo sensor value command: 0x06 0x91 0x00 0x02 0x66 0xFF 
                stream->write(0x06);
                stream->write(PROTOCOL_COMMAND_VALUE + adr);
                stream->write(sensorValue[adr] & 0x0ff);
                stream->write(sensorValue[adr] >> 8); 
                chksum = 0xFFFF - (0x06 + PROTOCOL_COMMAND_VALUE + adr + (sensorValue[adr]>>8) + (sensorValue[adr]&0x0ff));
                break;
              default:
                adr=0; // unknown command, prevent sending chksum
                break;
            }
            if (adr>0) {
              stream->write(chksum & 0x0ff);
              stream->write(chksum >> 8);              
            }
          }
        }
        state = DISCARD;
        break;

      case DISCARD:
      default:
        break;
    }
  }
}

uint16_t IBusBM::readChannel(uint8_t channelNr) {
  if (channelNr < PROTOCOL_CHANNELS) {
    return channel[channelNr];
  } else {
    return 0;
  }
}

uint8_t IBusBM::addSensor(uint8_t type) {
  // add a sensor, return sensor number
  if (NumberSensors < SENSORMAX) {
    NumberSensors++;
    sensorType[NumberSensors] = type;
  }
  return NumberSensors;
}

void IBusBM::setSensorMeasurement(uint8_t adr, uint16_t value){
   if (adr<=NumberSensors)
     sensorValue[adr] = value;
}


