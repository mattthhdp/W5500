#include <DHT.h>                  // For temperature / humidity sensor
#include <SPI.h>                  // For networking
#include <Ethernet.h>             // For networking
#include <PubSubClient.h>         // For MQTT

//Configuration //
#define Enable_Dhcp  true                 // true/false
IPAddress ip(192, 168, 1, 105);           //Static Adress if Enable_Dhcp = false 

//Static Mac Address
static uint8_t mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xBB };  // Set if there is no Mac_room

#define DHTTYPE DHT22
#define DHTPIN0  A0 //Chambre Samuel
#define DHTPIN1  A1 //Chambre Alexis
#define DHTPIN2  A2 //Chambre Maitre
#define DHTPIN3  A3 //Salon
#define DHTPIN4  A4 //Corridor
#define DHTPIN5  A5 //Cuisine

const int sendDhtInfo = 2000;    // Dht22 will report every X milliseconds.
unsigned long lastSend = 0;
const char* dhtPublish[] = { "chambre/samuel/climat/","chambre/alexis/climat/",
                 "chambre/master/climat/", "salon/haut/climat/",
                "corridor/climat/","cuisine/climat/" };

const int output_pin[6] = { 8,9,10,11,12,13 }; //Relay Pinout turn on/off light
const char* subscribeRelay[] = { "chambre/samuel/lumiere/main/status/", "chambre/samuel/lumiere/closet/status/",
                 "chambre/alexis/lumiere/main/status/","chambre/alexis/lumiere/closet/status/",
                 "chambre/master/lumiere/main/status/", "chambre/master/lumiere/closet/status/" };

const int intput_pin[6] = { 2,3,4,5,6,7 }; //Input Pinout light button
const char* inputPublish[] = { "chambre/samuel/lumiere/main/set/", "chambre/samuel/lumiere/closet/set/",
                 "chambre/alexis/lumiere/main/set/","chambre/alexis/lumiere/closet/set/",
                 "chambre/master/lumiere/main/set/", "chambre/master/lumiere/closet/set/" };

// MQTT Settings //
const char* broker = "192.168.1.240";        // MQTT broker
//#define mqttUser "USERNAME"         //Username for MQTT Broker
//#define mqttPassword "PASS"       //Password for MQTT Broker

DHT dht[] = { { DHTPIN0, DHTTYPE }, { DHTPIN1, DHTTYPE }, { DHTPIN2, DHTTYPE }, { DHTPIN3, DHTTYPE }, { DHTPIN4, DHTTYPE }, { DHTPIN5, DHTTYPE } };

EthernetClient ethclient;
PubSubClient client(ethclient);

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.println("Callback ");
  byte output_number = payload[0] - '0';

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
    char clientBuffer[40];

    String clientString = "Arduino-" + String(Ethernet.localIP()[2]) + "." + String(Ethernet.localIP()[3]);
    clientString.toCharArray(clientBuffer, clientString.length() + 1);
    if (client.connect(clientBuffer)) {

      for (int i = 0; i < sizeof(dhtPublish) / sizeof(dhtPublish[0]); i++)
      {
        client.publish(dhtPublish[i],clientBuffer);
      }

      for (int i = 0; i < sizeof(subscribeRelay) / sizeof(subscribeRelay[0]); i++)
      {
        client.subscribe(subscribeRelay[i]);
      }   
    }
    else {
      
      Serial.println("failed, rc=");
      Serial.println(client.state());
  }
}
}
void setup() 
  {
  Serial.begin(115200);
  Serial.println("Starting up OutputBoard Relay W5500 v1.0");
  Serial.println("====");


  if (Enable_Dhcp == true)
  {
    while (!Ethernet.begin(mac))
    {

    }
  }
  else {
    Ethernet.begin(mac, ip);  // Use static address defined above
  }

  Serial.println(Ethernet.localIP());
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
  //scan_buttons()
  client.loop();
}

void readDHT()
{
  
  for (int i = 0; i < sizeof(dhtPublish) / sizeof(dhtPublish[0]); i++)
  {

    float temperature = dht[i].readTemperature();
    float humidity = dht[i].readHumidity();
    float heatindex;

    char attributes[100];
Serial.print(i);
Serial.print("-Temp sensor: ");
Serial.print(temperature);
Serial.println("");


    
    heatindex = dht[i].computeHeatIndex(temperature, humidity, false);
    

    String payload = "{";
    payload += "\"temperature\":"; payload += String(temperature).c_str(); payload += ",";
    payload += "\"humidity\":"; payload += String(humidity).c_str(); payload += ",";
    payload += "\"heatindex\":"; payload += String(heatindex).c_str();
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

