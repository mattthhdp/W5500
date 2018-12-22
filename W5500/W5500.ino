/*
 Name:		W5500.ino
 Created:	12/19/2018 7:14:14 AM
 Author:	mattthhdp
*/


#include "configuration.h"
#include <SPI.h>                  // For networking
#include <Ethernet2.h>             // For networking
#include <PubSubClient.h>         // For MQTT
#include "Wire.h"                 // For MAC address //not working on KeyStudio W5500


//Configuration //
#define Enable_Dhcp               true   // true/false
IPAddress ip(192, 168, 1, 35);                //Static Adress if Enable_Dhcp = false

#define Enable_Mac_Address_Rom    false   // true/false
static uint8_t mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE };  // Set if there is no Mac_room
#define MAC_I2C_ADDRESS             0x50   // Microchip 24AA125E48 I2C ROM address


// MQTT Settings //
IPAddress broker(192, 168, 1, 111);        // MQTT broker
const char* statusTopic = "events";    // MQTT topic to publish status reports
char messageBuffer[100];
char topicBuffer[100];
char clientBuffer[50];

//Relay Pinout //
int output_pin[20] = { A0, A1, A2, A3, A4, A5, 0, 1, 2, 3,
                        4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
int output_state[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                         0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
const int output_number_pin = 20;

#pragma region Setup

/**
 * MQTT callback
 */
void callback(char* topic, byte* payload, unsigned int length)
{
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
	}
	Serial.println();
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
			// ... and resubscribe
			//client.subscribe("inTopic");
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

	if (Enable_Mac_Address_Rom == true)
	{
		Serial.print(F("Getting MAC address from ROM: "));
		mac[0] = readRegister(0xFA);
		mac[1] = readRegister(0xFB);
		mac[2] = readRegister(0xFC);
		mac[3] = readRegister(0xFD);
		mac[4] = readRegister(0xFE);
		mac[5] = readRegister(0xFF);
	}
	else {
		Serial.print(F("Using static MAC address: "));
	}
	// Print MAC address
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
	reconnect();

	}
void loop() 
	{
	if (!client.connected()) {
		reconnect();
	}
	client.loop();
	//Test MQTT Relay
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

byte readRegister(byte r)
{
	unsigned char v;
	Wire.beginTransmission(MAC_I2C_ADDRESS);
	Wire.write(r);  // Register to read
	Wire.endTransmission();

	Wire.requestFrom(MAC_I2C_ADDRESS, 1); // Read a byte
	while (!Wire.available())
	{
		// Wait
	}
	v = Wire.read();
	return v;
}