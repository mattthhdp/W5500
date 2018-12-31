/*
 Name:		W5500.ino
 Created:	12/19/2018 7:14:14 AM
 Author:	mattthhdp
*/


//#include <DHT_U.h>
#include <DHT.h>				 // For temperature / humidity sensor
#include "configuration.h"
#include <SPI.h>                  // For networking
#include <Ethernet2.h>             // For networking
#include <PubSubClient.h>         // For MQTT

//Configuration //
#define Enable_Dhcp               true   // true/false
IPAddress ip(192, 168, 1, 35);           //Static Adress if Enable_Dhcp = false 

//Static Mac Address
static uint8_t mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEF };  // Set if there is no Mac_room

#define DHTTYPE DHT22
#define DHTPIN  A0
DHT dht(DHTPIN, DHTTYPE);
unsigned long lastSend = 0;
int sendDhtInfo = 5000;    // Dht22 will report every X milliseconds.


// MQTT Settings //
IPAddress broker(192, 168, 1, 116);        // MQTT broker
const char* subscribeTo[] = { "/chambre/sam/lumiere/", "/chambre/sam/lumiere1/",
							   "/chambre/alex/lumiere/","/chambre/alex/lumiere1/",
							   "/chambre/master/lumiere/", "/chambre/master/lumiere1/" };
//SPI = 10,11,12,13		//0,1 rx,tx for usb
int output_pin[6] = { 2,A1,A2,A3,A4,A5 }; //Relay Pinout
int output_state[6] = { 0, 0, 0, 0, 0, 0 };
const int output_number_pin = 6;

const char* statusTopic[5] = { "/home/test1/set/", "/home/test2/set/","/home/test3/set/","/home/test4/set/","/home/test5/set/" };    // MQTT topic to publish status reports
int input_pin[7] = {3, 4, 5, 6, 7, 8, 9 }; //SPI = 10,11,12,13		//0,1 rx,tx for usb
int input_state[7] = { 0, 0, 0, 0, 0, 0, 0 };
const int input_number_pin = 7;
byte lastButtonPressed = 0;
#define DEBOUNCE_DELAY 50

// Instantiate MQTT client
//PubSubClient client(broker, 1883, callback);
EthernetClient ethclient;
PubSubClient client(ethclient);
char messageBuffer[100];
char topicBuffer[100];
char clientBuffer[50];
char command_topic[50];


#pragma region Setup

void callback(char* topic, byte* payload, unsigned int length)
{
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
	}
	byte output_number = payload[0] - '0';

	Serial.println();

	for ( int i = 0; i < output_number_pin; i++)
	{
		int strcomparison = strcmp(topic, subscribeTo[i]);
		if (strcomparison == 0)
		{
			Serial.print("Matched Topic # ");
			Serial.println(i);
			if (output_number == 1)// || ((char)payload[0] == '1'))
			{
				digitalWrite(output_pin[i], HIGH);
				output_state[i] = 1;
				Serial.print("Output: ");
				Serial.print(output_pin[i]);
				Serial.print(" State: ");
				Serial.println(output_state[i]);
			}
			if (output_number == 0)
			{
				digitalWrite(output_pin[i], LOW);
				output_state[i] = 0;
				Serial.print("Output: ");
				Serial.print(output_pin[i]);
				Serial.print(" State: ");
				Serial.println(output_state[i]);
			}
		}

	}
}


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
			clientString.toCharArray(clientBuffer, clientString.length() + 1);

			//Publishing Sensors and Light Switch to//
			for (int i = 0; i < 5; i++)
			{
				client.publish(statusTopic[i],clientBuffer);
				Serial.println("Publishing to :");
				Serial.println(statusTopic[i]);
			}

			//Subscribe for the relay//
			for (int i = 0; i < 5; i++)
			{
				client.subscribe(subscribeTo[i]);
				Serial.println("Subscribing to :");
				Serial.println(subscribeTo[i]);
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
/*
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
*/
#pragma endregion

	enable_and_reset_all_outputs(); //Reset and Set all pin on OUTPUT mode
	enable_and_reset_all_inputs();  //Reset and Set all pin on INPUT mode

	client.setServer(broker, 1883);
	client.setCallback(callback);

	//reconnect();

	}
void loop()
{
	//if (!client.connected())
	//{
	//	reconnect();
	//}
	if (millis() - lastSend > sendDhtInfo) { // Update and send only after 1 seconds
		readDHT();
		lastSend = millis();
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

void enable_and_reset_all_inputs()
{
	for (int i = 0; i < input_number_pin; i++)
	{
		pinMode(input_pin[i], INPUT);
		digitalWrite(input_pin[i], input_state[i]);
		input_state[i] = 0;
	}
}

void readDHT()
{
	Serial.println("Collecting temperature data.");

	// Reading temperature or humidity takes about 250 milliseconds!
	float h = dht.readHumidity();
	// Read temperature as Celsius (the default)
	float t = dht.readTemperature();

	// Check if any reads failed and exit early (to try again).
	if (isnan(h) || isnan(t)) {
		Serial.println("Failed to read from DHT sensor!");
		return;
	}

	Serial.print("Temperature: ");
	Serial.print(t);
	Serial.print(" *C\t ");

	Serial.print("Humidity: ");
	Serial.print(h);
	Serial.print(" %");
	Serial.println();

	String temperature = String(t);
	String humidity = String(h);

	// Prepare a JSON payload string
	String payload = "{";
	payload += "\"temperature\":"; payload += temperature; payload += ",";
	payload += "\"humidity\":"; payload += humidity;
	payload += "}";

	// Send payload
	char attributes[100];
	payload.toCharArray(attributes, 100);
	client.publish(statusTopic[0], attributes);
	Serial.print(statusTopic[0]);
	Serial.println(attributes);
	
}

