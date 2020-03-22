# MultiDecoderCoder
 This code is running only with Arduino Uno,Pro Mini ou Pro Micro 
	- Option JetiEx is running only with Pro Micro
	- Option Failsafe is running only with Pro Mini

 Decoder for maxi 16 servos for signals protols:
 - PPM based on libraries RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 
 - SBUS based on libraries FUTABA_SBUS https://github.com/mikeshub/FUTABA_SBUS/tree/master/FUTABA_SBUS
	A signal's reverse is needed (http://www.ernstc.dk/arduino/sbus.html)
 - IBUS based on librarie https://github.com/aanon4/FlySkyIBus 
 - DSMX based on librarie https://github.com/Quarduino/SpektrumSatellite
 - SRXL based on libraries RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 
 - SUMD based on exemple https://github.com/gregd72002/sumd2ppm
 - JETIEX based on librarie https://github.com/Sepp62/JetiExBus

 Coder convert 8 outputs servos PWM in:
 - PPM based on libraries RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 
 - SBUS based on libraries RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 


 How to use it:
 When you use your module the first time, you need to configure it.
 For that, you need to connect FTDI module to your Pro Mini board or use the USB port of your Pro Micro board.
 Use the IDE console (115200 bauds and NL and CR).
 You have 3 secondes for use the ENTER key.
 After that, without new settings, the code start in PPM mode.
   
 The console will return:
 
	Version:0.40
	Wait Return...
	Starting without configuration
	Decoder in use
	Failsafe is Off
	8 outputs mode (Use D2-D9)
	PPM mode in use

 If you use the ENTER key before the 3s, you enter in the Configuration mode.
 
	Version:0.40
	Wait Return.
	Configuration mode is actived
	Decoder in use
	Failsafe is Off
	8 outputs mode (Use D2-D9)
	PPM mode in use

 Enter 'h' key (without cote) for the help.
 
	h Help
	q quit
	0 set Decoder mode
	1 set Coder mode
	n set 8/16 outputs mode
	r first 8 outputs changed
	p set PPM mode
	s set SBUS mode
	i set IBUS mode
	d set DSMX mode
	m set SRLX mode
	u set SUMD mode
	j set JETIEx mode
	f set Failsafe values

