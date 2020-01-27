#include <OneButton.h>

#include <DHT.h>
#include <PubSubClient.h>
#include <Ethernet.h>

//Configuration //
#define Enable_Dhcp  true                 // true/false
IPAddress ip(192, 168, 1, 105);           //Static Adress if Enable_Dhcp = false

//Static Mac Address
static uint8_t mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xB1 };  // Master = BB, Cuisine =B1
#define DHTTYPE DHT22
#define DHTPIN0  A0 //Cuisine main
#define DHTPIN1  A1 //cuisine Lavabo
OneButton button1(A4, false); //Cuisine main
OneButton button2(A5, false); // Cuisine Lavabo
//#define DHTPIN2  A2 //Closet
//#define DHTPIN3  A3 //Salon
//#define DHTPIN4  A4 //Corridor
//#define DHTPIN5  A5 //Cuisine
//pinout A0-A5  2-9
const int sendDhtInfo = 30000;    // Dht22 will report every X milliseconds.
unsigned long lastSend = 0;
const char* dhtPublish[] = { "chambre/cuisine/climat/main/", "chambre/cuisine/climat/lavabo/" };

const int output_pin[3] = { 2, 3, 4 }; //Relay Pinout turn on/off light et chauffage
//2=lumiere main
//3=lumiere garde-robe
//4=chauffage
const char* subscribeRelay[] = { "chambre/cuisine/lumiere/main/status/", "chambre/cuisine/lumiere/lavabo/status/",
                                 "chambre/cuisine/heat/main/status/"}; //, "chambre/alexis/lumiere/closet/status/",
                                // "chambre/master/lumiere/main/status/", "chambre/master/lumiere/closet/status/"
                               //};

//const int intput_pin[2] = { 5, 6 }; //Input Pinout light button
const char* inputPublish[] = { "chambre/cuisine/lumiere/main/simple/", "chambre/cuisine/lumiere/main/double/", "chambre/cuisine/lumiere/main/long/",
                               "chambre/cuisine/lumiere/lavabo/simple/","chambre/cuisine/lumiere/lavabo/double/","chambre/cuisine/lumiere/lavabo/long/" };

// MQTT Settings //
//const char* broker = "192.168.1.240";        // MQTT broker
const char* broker = "ubuntu.jaune.lan";        // MQTT broker
//#define mqttUser "USERNAME"         //Username for MQTT Broker
//#define mqttPassword "PASS"       //Password for MQTT Broker

DHT dht[] = { { DHTPIN0, DHTTYPE }, { DHTPIN1, DHTTYPE }, }; //{ DHTPIN2, DHTTYPE }, { DHTPIN3, DHTTYPE }, { DHTPIN4, DHTTYPE }, { DHTPIN5, DHTTYPE } };

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

    String clientString = "ip" + String(Ethernet.localIP()[2]) + "." + String(Ethernet.localIP()[3]);
    clientString.toCharArray(clientBuffer, clientString.length() + 1);
    if (client.connect(clientBuffer)) {

      for (int i = 0; i < sizeof(dhtPublish) / sizeof(dhtPublish[0]); i++)
      {
        client.publish(dhtPublish[i], clientBuffer);
      }

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
  if (Enable_Dhcp == true)
  {
    while (!Ethernet.begin(mac))
    {

    }
  }
  else {
    Ethernet.begin(mac, ip);  // Use static address defined above
  }

  enable_and_reset_all_outputs(); //Reset and Set all pin on OUTPUT mode

  client.setServer(broker, 1883);
  client.setCallback(callback);
  for (int i = 0; i < sizeof(dhtPublish) / sizeof(dhtPublish[0]); i++)
  {
  dht[i].begin();
  }

  button1.attachClick(click1);
  button1.attachDoubleClick(doubleclick1);
  button1.attachLongPressStart(longPressStart1);
  //button1.attachLongPressStop(longPressStop1);
  //button1.attachDuringLongPress(longPress1);

  button2.attachClick(click2);
  button2.attachDoubleClick(doubleclick2);
  button2.attachLongPressStart(longPressStart2);
  //button1.attachLongPressStop(longPressStop2);
  //button1.attachDuringLongPress(longPress2);

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
  button1.tick();
  client.loop();
}

void readDHT()
{

  for (int i = 0; i < sizeof(dhtPublish) / sizeof(dhtPublish[0]); i++)
  {

    float temperature = dht[i].readTemperature();
    float humidity = dht[i].readHumidity();
    float heatindex;

    if(isnan(humidity) || isnan(temperature))
    {
      temperature = 100.0f;
      humidity = 100.0f;
      heatindex = 100.0f;
    }
    else
    {
      heatindex = dht[i].computeHeatIndex(temperature,humidity,false);
    }

    String payload = "{";
    payload += "\"temperature\":"; payload += String(temperature).c_str(); payload += ",";
    payload += "\"humidity\":"; payload += String(humidity).c_str(); payload += ",";
    payload += "\"heatindex\":"; payload += String(heatindex).c_str();
    payload += "}";

    // Send payload
    char attributes[100];
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


////Boutton1
void click1() 
{
  client.publish("chambre/cuisine/lumiere/main/simple/", "1");
} 

void doubleclick1() 
{
  client.publish("chambre/cuisine/lumiere/main/double/", "1");
} 

void longPressStart1() 
{
  client.publish("chambre/cuisine/lumiere/main/long/", "1");
} 


////Boutton1
void click2() 
{
  client.publish("chambre/cuisine/lumiere/lavabo/simple/", "1");
} 

void doubleclick2() 
{
  client.publish("chambre/cuisine/lumiere/lavabo/double/", "1");
} 

void longPressStart2() 
{
  client.publish("chambre/cuisine/lumiere/lavabo/long/", "1");
} 
