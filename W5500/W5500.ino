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
#include <SPI.h>                  // For networking
#include <Ethernet2.h>             // For networking
#include <PubSubClient.h>         // For MQTT

//Configuration //
#define Enable_Dhcp           true   // true/false
IPAddress ip(192, 168, 1, 30);           //Static Adress if Enable_Dhcp = false 

//Static Mac Address
static uint8_t mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEF };  // Set if there is no Mac_room

#define DHTTYPE DHT22
#define DHTPIN1  A0 //Chambre Samuel
#define DHTPIN2	 A1	//Chambre Alexis
#define DHTPIN3  A2 //Chambre Maitre
#define DHTPIN3  A3 //Salon
#define DHTPIN3  A4 //Corridor
#define DHTPIN3  A5 //Cuisine

const int sendDhtInfo = 10000;    // Dht22 will report every X milliseconds.
unsigned long lastSend = 0;
const char* dhtPublish[] = { "chambre/samuel/climat/","chambre/alexis/climat/",
							   "chambre/master/climat/", "salon/haut/climat/"
								"corridor/climat/","/cuisine/climat/" };

const int output_pin[6] = { 8,9,10,11,12,13 }; //Relay Pinout turn on/off light
const char* subscribeRelay[] = { "chambre/samuel/lumiere/main/status/", "chambre/samuel/lumiere/closet/status/",
								 "chambre/alexis/lumiere/main/status/","chambre/alexis/lumiere/closet/status/",
								 "chambre/master/lumiere/main/status/", "chambre/master/lumiere/closet/status/" };

const int intput_pin[6] = { 2,3,4,5,6,7 }; //Input Pinout light button
const char* inputPublish[] = { "chambre/samuel/lumiere/main/set/", "chambre/samuel/lumiere/closet/set/",
							   "chambre/alexis/lumiere/main/set/","chambre/alexis/lumiere/closet/set/",
							   "chambre/master/lumiere/main/set/", "chambre/master/lumiere/closet/set/" };

// MQTT Settings //
IPAddress broker(192, 168, 1, 30);        // MQTT broker
//#define mqttUser "USERNAME"					//Username for MQTT Broker
//#define mqttPassword "PASS"				//Password for MQTT Broker
/// Nothing should be modified after this. ///
DHT dht[] = { { DHTPIN1, DHTTYPE }, { DHTPIN2, DHTTYPE }, { DHTPIN3, DHTTYPE } };




// Instantiate MQTT client
//PubSubClient client(broker, 1883, callback);
EthernetClient ethclient;
PubSubClient client(ethclient);

void callback(char* topic, byte* payload, unsigned int length)
{
	/*Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) 
		{
		Serial.print((char)payload[i]);
		}
	*/
	byte output_number = payload[0] - '0';

	//Serial.println();

	for (int i = 0; i < sizeof(subscribeRelay) / sizeof(subscribeRelay[0]); i++)
	{
		int strcomparison = strcmp(topic, subscribeRelay[i]);
		if (strcomparison == 0)
		{

			Serial.print("Matched Topic # ");
			Serial.println(i);
			if (output_number == 1)// || ((char)payload[0] == '1'))
			{
				digitalWrite(output_pin[i], HIGH);
				//Serial.print("Output: ");
				//Serial.print(output_pin[i]);
			}
			if (output_number == 0)
			{
				digitalWrite(output_pin[i], LOW);
				//Serial.print("Output: ");
				//Serial.print(output_pin[i]);
			}
		}

	}
}

void reconnect() {
	// Loop until we're reconnected
	while (!client.connected()) {
		char clientBuffer[50];

		//Serial.print("Attempting MQTT connection to : ");
		//Serial.println(broker);
		// Attempt to connect
		String clientString = "Arduino-" + String(Ethernet.localIP());
		clientString.toCharArray(clientBuffer, clientString.length() + 1);
		if (client.connect(clientBuffer)) {
			//client.connect(clientBuffer, mqttUser, mqttPassword);
			//Serial.println("connected");
			//clientString.toCharArray(clientBuffer, clientString.length() + 1);

			//Publishing Sensors and Light Switch to//
			for (int i = 0; i < sizeof(dht) / sizeof(dht[0]); i++)
			{
				client.publish(dhtPublish[i],clientBuffer);
				//Serial.print("Publishing to :  ");
				//Serial.println(dhtPublish[i]);
			}

			for (int i = 0; i < sizeof(subscribeRelay) / sizeof(subscribeRelay[0]); i++)
			{
				client.subscribe(subscribeRelay[i]);
				//Serial.println("Subscribing to :");
				//.println(subscribeRelay[i]);
			}
			
		}
		//If not connected//
		//else {
			/*
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
			//Wait 2 seconds before retrying
			*/
			//delay(2000);
		//}
	}
}

void setup() 
	{
	Serial.begin(115200);
	
	Serial.println();
	//Serial.println("=====================================");
	Serial.println("Starting up OutputBoard Relay W5500 v1.0");
	//Serial.println("=====================================");

	//Serial.print(F("MAC address: "));
	//char tmpBuf[17];
	//sprintf(tmpBuf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	//Serial.println(tmpBuf);


	if (Enable_Dhcp == true)
	{
		//Serial.println("Using Dhcp, Acquiring IP Address ...");
		while (!Ethernet.begin(mac))
		{
			//Serial.println("Error trying to get dynamic ip ... retrying in 2 seconds");
			//delay(2000);
		}
		//Serial.print("Using Dhcp : ");

	}
	else {
		//Serial.print("Using Static IP : ");
		Ethernet.begin(mac, ip);  // Use static address defined above
	}

	Serial.println(Ethernet.localIP());
	//Serial.println("=====================================");


	enable_and_reset_all_outputs(); //Reset and Set all pin on OUTPUT mode


	client.setServer(broker, 1883);
	client.setCallback(callback);

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

	scan_buttons()
	client.loop();

}

void scan_buttons()
{
	if (millis() - last_activity_time > debounce_timeout)
	{
		byte this_button_state = BUTTONUP;
		for (int i = 0; i < 8; i++)
		{
			this_button_state = digitalRead(button_pin[i]);
			//Serial.print(this_button_state);
			if (this_button_state == BUTTONDOWN && button_state[i] == BUTTONUP)
			{
				// We've detected a keypress
				last_activity_time = millis();
				screen_line_4 = "Button press";
				refresh_screen();
				char button[2];
				sprintf(button, "%d", i);
				button_state[i] = BUTTONDOWN;
				byte output_number = i + 1;
				if (output_state[i] == 0)
				{
					turn_output_on(output_number);
				}
				else {
					turn_output_off(output_number);
				}
			}

			button_state[i] = this_button_state;
		}
	}
}

void readDHT()
{
	
	int dhtsize = sizeof(dht) / sizeof(dht[0]);
	Serial.print("Collecting temperature data from : ");
	Serial.print(dhtsize);
	Serial.println(" DHT Sensor");
	

	for (int i = 0; i < sizeof(dht) / sizeof(dht[0]); i++)
	{

		float temperature = dht[i].readTemperature();
		float humidity = dht[i].readHumidity();
		float heatindex;

		char attributes[100];
		// Check if any reads failed
		if (isnan(humidity) || isnan(temperature)) // set value to -1 so we publish an error
		{
			temperature = -1;
			humidity = -1;
			heatindex = -1;
		}

		else
		{
			heatindex = dht[i].computeHeatIndex(temperature, humidity, false);
		}
		
		Serial.print("Temperature: ");
		Serial.print(temperature);
		Serial.print(" *C\t ");

		Serial.print("Humidity: ");
		Serial.print(humidity);
		Serial.print(" %");
		Serial.println();

		Serial.print("HeatIndex: ");
		Serial.print(heatindex);
		Serial.print(" *C");
		Serial.println();
		
		// Prepare a JSON payload string
		String payload = "{";
		payload += "\"temperature\":"; payload += String(temperature).c_str(); payload += ",";
		payload += "\"humidity\":"; payload += String(humidity).c_str(); payload += ",";
		payload += "\"heatindex\":"; payload += String(heatindex).c_str();
		payload += "}";

		// Send payload
		payload.toCharArray(attributes, (payload.length() + 1));
		client.publish(dhtPublish[i], attributes);
		//Serial.print(dhtPublish[i]);
		//Serial.println(attributes);

		///////////
		///TO BE DELETED
		if (temperature > 19)
		{
			digitalWrite(output_pin[i], LOW);
			Serial.print("Relay ");
			Serial.println(" Low");
		}
		else if (temperature < 18.0 && temperature > 5)
		{
			digitalWrite(output_pin[i], HIGH);
			Serial.print("Relay ");
			Serial.println(" High");
		}
		else
		{
			digitalWrite(output_pin[i], LOW);
			Serial.print("Relay ");
			Serial.println(" Low");
			Serial.println("!!! Sensor Defect !!!");
			//remove me

		}
	}
}

void enable_and_reset_all_outputs()
{
	for (int i = 0; i < sizeof(subscribeRelay) / sizeof(subscribeRelay[0]); i++)
	{
		pinMode(output_pin[i], OUTPUT);
		digitalWrite(output_pin[i], LOW);
	}
}
