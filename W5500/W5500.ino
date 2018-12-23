/*
 Name:		W5500.ino
 Created:	12/19/2018 7:14:14 AM
 Author:	mattthhdp
*/


#include "configuration.h"
#include <SPI.h>                  // For networking
#include <Ethernet2.h>             // For networking
#include <PubSubClient.h>         // For MQTT

//Configuration //
#define Enable_Dhcp               true   // true/false
IPAddress ip(192, 168, 1, 35);           //Static Adress if Enable_Dhcp = false 

static uint8_t mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE };  // Set if there is no Mac_room

// MQTT Settings //
IPAddress broker(192, 168, 1, 116);        // MQTT broker
const char* statusTopic = "events";    // MQTT topic to publish status reports
char messageBuffer[100];
char topicBuffer[100];
char clientBuffer[50];
char command_topic[50];

//Relay Pinout //
int output_pin[16] = { 2,3,4,5,6,7,8,9,14,15,16,17,18,19 }; //SPI = 10,11,12,13		//0,1 rx,tx for usb
int output_state[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
const int output_number_pin = 16;

#pragma region Setup

void callback(char* topic, byte* payload, unsigned int length)
{
	Serial.print("Message arrived [");
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

// Instantiate MQTT client
//PubSubClient client(broker, 1883, callback);
EthernetClient ethclient;
PubSubClient client(ethclient);

void reconnect() {
	// Loop until we're reconnected
	while (!client.connected()) {
		Serial.print("Attempting MQTT connection to : ");
		Serial.println(broker);
		// Attempt to connect
		String clientString = "Reconnecting Arduino-" + String(Ethernet.localIP());
		clientString.toCharArray(clientBuffer, clientString.length() + 1);
		if (client.connect(clientBuffer)) {
			Serial.println("connected");
			// Once connected, publish an announcement...
			clientString.toCharArray(clientBuffer, clientString.length() + 1);
			client.publish(statusTopic, clientBuffer);
			Serial.print("Publishing to : ");
			Serial.println(statusTopic);

			client.subscribe(command_topic);
			Serial.print("Subscribing to ");
			Serial.println(command_topic);
		}
		else {
			Serial.print("failed, rc=");
			Serial.print(client.state());

#pragma region Debugging MQTT error

			if (client.state() == -1)
			{
				Serial.print(" Timed_out");
			}
			else if (client.state() == -2)
			{
				Serial.print(" INVALID_SERVER");
			}
			else if (client.state() == -3)
			{
				Serial.print(" TRUNCATED");
			}
			else if (client.state() == -4)
			{
				Serial.print(" INVALID_RESPONSE");
			}
			else
				Serial.print(" Impossible Error ... lol"); 
#pragma endregion
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

#pragma endregion


void setup() 
	{
	Serial.begin(115200);
	Serial.println();
	Serial.println("=====================================");
	Serial.println("Starting up OutputBoard Relay W5500 v1.0");
	Serial.println("=====================================");

#pragma region Mac and ip Setup

	
	Serial.print(F("MAC address: "));
	char tmpBuf[17];
	sprintf(tmpBuf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	Serial.println(tmpBuf);

	if (Enable_Dhcp == true)
	{
		Serial.println("Using Dhcp, Acquiring IP Address ...");
		while (!Ethernet.begin(mac))
		{
			Serial.println("Error trying to get dynamic ip ... retrying in 5 seconds");
			delay(5000);
		}
		Serial.print("Using Dhcp : ");

	}
	else {
		Serial.print("Using Static IP : ");
		Ethernet.begin(mac, ip);  // Use static address defined above
	}

	Serial.println(Ethernet.localIP());
	Serial.println("=====================================");


#pragma endregion

	enable_and_reset_all_outputs(); //Reset and Set all pin on OUTPUT mode

	client.setServer(broker, 1883);
	client.setCallback(callback);
	String clientString = "Starting Arduino-" + Ethernet.localIP();
	clientString.toCharArray(clientBuffer, clientString.length() + 1);
	client.publish(statusTopic, clientBuffer);
	sprintf(command_topic, "device/command/");  // For receiving messages
	reconnect();

	}
void loop() 
	{
	if (!client.connected()) {
		reconnect();
	}
	client.loop();
	}

/// Set all pint to OUTPUT and state to 0 = OFF
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
