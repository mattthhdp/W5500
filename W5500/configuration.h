// configuration.h

#ifndef _CONFIGURATION_h
#define _CONFIGURATION_h



// WiFi setup
#define WIFISSID       "Your SSID"
#define WIFIPASSWORD   "Your Password"
#define MQTTBROKER     "192.168.1.111"


#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#endif
