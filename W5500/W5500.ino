/*
 Name:		W5500.ino
 Created:	12/19/2018 7:14:14 AM
 Author:	mattthhdp

 Use to control my home automation, the board is split into 2 parts. Input and Output.
 Input have some DHT22 for temperature reading into each room, and some touch button (for the light)
 Output have relay

 Both do not interact. They Publish/Subscribe to the MQTT broker. 
 Every Logic is done into Home Assistant.

 Example, the light switch 1 publish to /home/masterbedroom/light/main/
 and the relay subscribe to				/home/masterbedroom/light/set/
 In Home Assistant, i can define the rule that i want (in this case, if light/main is set to on 
 then i can turn light/set/ to on, to turn on the light.
*/


//#include <DHT_U.h>
#include <DHT.h>				 // For temperature / humidity sensor
#include "configuration.h"
#include <SPI.h>                  // For networking
#include <Ethernet2.h>             // For networking
#include <PubSubClient.h>         // For MQTT

//Configuration //
#define Enable_Dhcp           true   // true/false
IPAddress ip(192, 168, 1, 35);           //Static Adress if Enable_Dhcp = false 

//Static Mac Address
static uint8_t mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEF };  // Set if there is no Mac_room

#define DHTTYPE DHT22
#define DHTPIN1  A0
#define DHTPIN2	 A1
#define DHTPIN3  A2
//DHT dht[] = { { DHTPIN1, DHTTYPE },{ DHTPIN2,DHTTYPE} };
DHT dht[] = { { DHTPIN1, DHTTYPE }, { DHTPIN2, DHTTYPE }, { DHTPIN3, DHTTYPE } };
const int totalDht = 3;
unsigned long lastSend = 0;
const int sendDhtInfo = 5000;    // Dht22 will report every X milliseconds.
const char* dhtPublish[] = { "/chambre/sam/climat/","/chambre/alex/climat/",
							   "/chambre/master/climat/" };

// MQTT Settings //
IPAddress broker(192, 168, 1, 116);        // MQTT broker


// Instantiate MQTT client
//PubSubClient client(broker, 1883, callback);
EthernetClient ethclient;
PubSubClient client(ethclient);

void reconnect() {
	// Loop until we're reconnected
	while (!client.connected()) {
		char clientBuffer[50];

		Serial.print("Attempting MQTT connection to : ");
		Serial.println(broker);
		// Attempt to connect
		String clientString = "Reconnecting Arduino-" + String(Ethernet.localIP());
		clientString.toCharArray(clientBuffer, clientString.length() + 1);
		if (client.connect(clientBuffer)) {
			Serial.println("connected");
			clientString.toCharArray(clientBuffer, clientString.length() + 1);

			//Publishing Sensors and Light Switch to//
			for (int i = 0; i < totalDht; i++)
			{
				client.publish(dhtPublish[i],clientBuffer);
				Serial.print("Publishing to :  ");
				Serial.println(dhtPublish[i]);
			}
			
		}
		//If not connected//
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
			Serial.println(" try again in 2 seconds");
			// Wait 5 seconds before retrying
			delay(2000);
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
			Serial.println("Error trying to get dynamic ip ... retrying in 2 seconds");
			delay(2000);
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

	client.setServer(broker, 1883);

	}
void loop()
{
	if (!client.connected())
	{
		reconnect();
	}

	if (millis() - lastSend > sendDhtInfo)
	{
		readDHT();
		lastSend = millis();
	}

	client.loop();

}


void readDHT()
{
	Serial.print("Collecting temperature data from : ");
	Serial.print(totalDht);
	Serial.println(" DHT Sensor");


	for (int i = 0; i < totalDht; i++)
	{
		//temperature[i] = dht[i].readTemperature();
		//humidity[i] = dht[i].readHumidity();
		float temperature = dht[i].readTemperature();
		float humidity = dht[i].readHumidity();


		char attributes[100];
		// Check if any reads failed
		if (!isnan(humidity) || !isnan(temperature))
		{
			Serial.print("Temperature: ");
			Serial.print(temperature);
			Serial.print(" *C\t ");

			Serial.print("Humidity: ");
			Serial.print(humidity);
			Serial.print(" %");
			Serial.println();

			// Prepare a JSON payload string
			String payload = "{";
			payload += "\"temperature\":"; payload += String(temperature).c_str(); payload += ",";
			payload += "\"humidity\":"; payload += String(humidity).c_str();
			payload += "}";

			// Send payload
			payload.toCharArray(attributes, (payload.length() + 1));
			client.publish(dhtPublish[i], attributes);
			Serial.print(dhtPublish[i]);
			Serial.println(attributes);
		}

		else
		{

			Serial.print("Failed to read from DHT sensor number : ");
			Serial.println(i);
			client.publish(dhtPublish[i], "error"); //TODO: Ajouté publish dans FAULT 1
		}
	}	
}