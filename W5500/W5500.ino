/*
 Name:    W5500.ino
 Created: 12/19/2018 7:14:14 AM
 Author:  mattthhdp

 Use to control my home automation, the board is split into 2 parts. Input and Output.
 Input have some DHT22 for temperature reading into each room, and some touch button (for the light)
 Output have relay

 Both do not interact. They Publish/Subscribe to the MQTT broker. 
 Every Logic is done into Home Assistant.

 Example, the light switch 1 publish to /home/masterbedroom/light/main/
 and the relay subscribe to       /home/masterbedroom/light/set/
 In Home Assistant, i can define the rule that i want (in this case, if light/main is set to on 
 then i can turn light/set/ to on, to turn on the light.
*/


//#include <DHT_U.h>
#include <DHT.h>         // For temperature / humidity sensor
#include <SPI.h>                  // For networking
#include <Ethernet.h>             // For networking
#include <PubSubClient.h>         // For MQTT

//Configuration //

//Static Mac Address
static uint8_t mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xAA };  // Set if there is no Mac_room

#define DHTTYPE DHT22
#define DHTPIN1  A0 //Chambre Samuel
#define DHTPIN2  A1 //Chambre Alexis
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
/*const char* inputPublish[] = { "chambre/samuel/lumiere/main/set/", "chambre/samuel/lumiere/closet/set/",
                 "chambre/alexis/lumiere/main/set/","chambre/alexis/lumiere/closet/set/",
                 "chambre/master/lumiere/main/set/", "chambre/master/lumiere/closet/set/" };
*/
// MQTT Settings //
const char* broker = "ubuntu.jaune.lan";        // MQTT broker
//#define mqttUser "USERNAME"         //Username for MQTT Broker
//#define mqttPassword "PASS"       //Password for MQTT Broker
/// Nothing should be modified after this. ///
DHT dht[] = { { DHTPIN1, DHTTYPE }, { DHTPIN2, DHTTYPE }, { DHTPIN3, DHTTYPE } };




// Instantiate MQTT client
//PubSubClient client(broker, 1883, callback);
EthernetClient ethclient;
PubSubClient client(ethclient);

void callback(char* topic, byte* payload, unsigned int length)
{
  byte output_number = payload[0] - '0';

  for (int i = 0; i < sizeof(subscribeRelay) / sizeof(subscribeRelay[0]); i++)
  {
    int strcomparison = strcmp(topic, subscribeRelay[i]);
    if (strcomparison == 0)
    {
      if (output_number == 1)
      {
        digitalWrite(output_pin[i], HIGH);
      }
      if (output_number == 0)
      {
        digitalWrite(output_pin[i], LOW);
      }
    }

  }
}

void reconnect() {
  while (!client.connected()) {
    char clientBuffer[50];

    String clientString = "Arduino-" + String(Ethernet.localIP()[3]);
    Serial.println(clientString);
    clientString.toCharArray(clientBuffer, clientString.length() + 1);
    if (client.connect(clientBuffer)) {

//      for (int i = 0; i < sizeof(dht) / sizeof(dht[0]); i++)
//      {
//        client.publish(dhtPublish[i],clientBuffer);
//      }

      for (int i = 0; i < sizeof(subscribeRelay) / sizeof(subscribeRelay[0]); i++)
      {
        client.subscribe(subscribeRelay[i]);
      }     
    }
  }
}

void setup() 
  {
  Serial.begin(115200);
  Serial.println("Starting up");


    while (!Ethernet.begin(mac))
    {
    }

  Serial.println(Ethernet.localIP());
  enable_and_reset_all_outputs(); 
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

  client.loop();

}


void readDHT()
{
  int dhtsize = sizeof(dht) / sizeof(dht[0]);
  for (int i = 0; i < sizeof(dht) / sizeof(dht[0]); i++)
  {

    float temperature = dht[i].readTemperature();
    float humidity = dht[i].readHumidity();
    float heatindex;
  Serial.println(temperature);

    char attributes[100];

    if (isnan(humidity) || isnan(temperature))
    {
      return;
    }

   // else
   // {
   //   heatindex = dht[i].computeHeatIndex(temperature, humidity, false);
   // }

    String payload = "{";
    payload += "\"temperature\":"; payload += String(temperature).c_str(); payload += ",";
    payload += "\"humidity\":"; payload += String(humidity).c_str(); payload += ",";
   // payload += "\"heatindex\":"; payload += String(heatindex).c_str();
    payload += "}";

    // Send payload
    payload.toCharArray(attributes, (payload.length() + 1));
    client.publish(dhtPublish[i], attributes);

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



