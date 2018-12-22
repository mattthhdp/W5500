/*
 Name:		W5500.ino
 Created:	12/19/2018 7:14:14 AM
 Author:	mattthhdp
*/


#include "configuration.h"
//#include <Ethernet.h>
byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 }; //physical mac address


int output_pin[20] = { A0, A1, A2, A3, A4, A5,0,1,2,3,4,5,6,7,8,9,10,11,12,13 };
int output_state[20] = { 0,  0,  0,  0,  0,  0,  0,  0,0,0,0,0,0,0,0,0,0,0,0,0 };
const int output_number_pin = 20;


void setup() 
	{
		enable_and_reset_all_outputs();
		
		sprintf(MQTTBROKER, "Test ip");
	}
void loop() 
	{
	//turn_output_off_on();
	sprintf(MQTTBROKER, "Test ip");
	}


void enable_and_reset_all_outputs()
{
	for (int i = 0; i < output_number_pin; i++)
	{
		pinMode(output_pin[i], OUTPUT);
		digitalWrite(output_pin[i], output_state[i]);
		output_state[i] = 0;
	}
}



void turn_output_off(int output_number)
{
	char message[25];
	byte output_index = output_number - 1;

	if (output_number == 0)
	{
		for (int i = 0; i < 8; i++)
		{
			digitalWrite(output_pin[i], LOW);
			output_state[i] = 0;
		}
		sprintf(message, "Turning OFF all outputs");
	}
	else if (output_number < 9) {
		digitalWrite(output_pin[output_index], LOW);
		output_state[output_index] = 0;
		sprintf(message, "Turning OFF output %d", output_number);
	}

	Serial.println(message);
}
void turn_output_on(int output_number)
{
	char message[25];
	byte output_index = output_number - 1;

	if (output_number == 0)
	{
		for (int i = 0; i < 8; i++)
		{
			digitalWrite(output_pin[i], HIGH);
			output_state[output_index] = 1;
		}
		sprintf(message, "Turning ON all outputs");
	}
	else if (output_number < 9) {
		digitalWrite(output_pin[output_index], HIGH);
		output_state[output_index] = 1;
		sprintf(message, "Turning ON output %d", output_number);
	}

	Serial.println(message);
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message  [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
	}
	Serial.println();

	byte output_number = payload[0] - '0';
	byte output_state = payload[2] - '0';
	Serial.print("Output: ");
	Serial.println(output_number);
	Serial.print("State: ");
	Serial.println(output_state);

	switch (output_state)
	{
	case 0:
		turn_output_off(output_number);
		break;
	case 1:
		turn_output_on(output_number);
		break;
	}
}
