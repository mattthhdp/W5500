#include <OneButton.h>

#include <DHT.h>
#include <PubSubClient.h>
#include <Ethernet.h>

//Configuration //
//#define Enable_Dhcp  true                 // true/false
//IPAddress ip(192, 168, 1, 105);           //Static Adress if Enable_Dhcp = false

//Static Mac Address
static uint8_t mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xA1 };  // masterbedroom
#define DHTTYPE DHT22
#define DHTPIN0  A0 //masterbedroom main dht22
#define DHTPIN1  A1 //masterbedroom closet dht22
#define DHTPIN2  A2 //passage main
#define DHTPIN3  A3 //passage proche garage
OneButton button1(A4, false); //masterbedroom main light switch
OneButton button2(2, false); //masterbedroom closet light switch
OneButton button3(A5, false); //passage main light switch
OneButton button4(3, false); //passage closet light switch

//pinout A0-A5  2-9
const int sendDhtInfo = 30000;    // Dht22 will report every X milliseconds.
unsigned long lastSend = 0;
const char* dhtPublish[] = { "c/mas/climat/main/", "c/mas/climat/closet/", "c/pas/climat/main/", "c/pas/climat/closet/" };

const int output_pin[6] = { 4, 5, 6, 7, 8, 9 }; //Relay Pinout turn on/off light et chauffage
//4=l main a
//5=l garde-robe a
//6=chauffage a
//7=l main s
//8=l garde-robe s
//9=chauffage s
const char* subscribeRelay[] = { "c/mapapas/l/main/status/", "c/mapapapas/l/closet/status/",
                                 "c/a/heat/main/status/", "c/papapas/l/closet/status/",
                                 "c/papapas/l/main/status/", "c/papapas/l/closet/status/"
                               };

const char* inputPublish[] = { "c/mapapas/l/main/simple/", "c/mapapas/l/main/double/", "c/mapapas/l/main/long/",
                               "c/mapapas/l/closet/simple/","c/mapapas/l/closet/double/","c/mapapas/l/closet/long/",
                               "c/papas/l/main/simple/", "c/papas/l/main/double/", "c/papas/l/main/long/",
                               "c/papas/l/closet/simple/","c/papas/l/closet/double/","c/papas/l/closet/long/" 
                               };

// MQTT Settings //
const char* broker = "ubuntu.jaune.lan"; // MQTT broker
//#define mqttUser "USERNAME"         //Username for MQTT Broker
//#define mqttPassword "PASS"       //Password for MQTT Broker

DHT dht[] = { { DHTPIN0, DHTTYPE }, { DHTPIN1, DHTTYPE }, { DHTPIN2, DHTTYPE }, { DHTPIN3, DHTTYPE } }; // { DHTPIN4, DHTTYPE }, { DHTPIN5, DHTTYPE } };

EthernetClient ethclient;
PubSubClient client(ethclient);

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.println("Callback");
  Serial.println(topic);
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

    String clientString = "ip : " + String(Ethernet.localIP()[2]) + "." + String(Ethernet.localIP()[3]);
    clientString.toCharArray(clientBuffer, clientString.length() + 1);
    if (client.connect(clientBuffer)) 
    {

      client.publish("arduino/c/ip/", clientBuffer);

      // for (int i = 0; i < sizeof(dhtPublish) / sizeof(dhtPublish[0]); i++)
      // {
      //   client.publish(dhtPublish[i], clientBuffer);
      // }

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
  // if (Enable_Dhcp == true)
  // {
    while (!Ethernet.begin(mac))
    {

    }
  //}
  // else {
  //   Ethernet.begin(mac, ip);  // Use static address defined above
  // }

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

   button3.attachClick(click3);
   button3.attachDoubleClick(doubleclick3);
   button3.attachLongPressStart(longPressStart3);

   button4.attachClick(click4);
   button4.attachDoubleClick(doubleclick4);
   button4.attachLongPressStart(longPressStart4);

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
  button2.tick();
  button3.tick();
  button4.tick();

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


//Boutton1
void click1() 
{
  client.publish("c/mapapas/l/main/simple/", "1");
} 
void doubleclick1() 
{
  client.publish("c/mapapas/l/main/double/", "1");
} 
void longPressStart1() 
{
  client.publish("c/mapapas/l/main/long/", "1");
} 


//Boutton2
void click2() 
{
  client.publish("c/mapapas/l/closet/simple/", "1");
} 
void doubleclick2() 
{
  client.publish("c/mapapas/l/closet/double/", "1");
} 
void longPressStart2() 
{
  client.publish("c/mapapas/l/closet/long/", "1");
} 

//Buton3
void click3() 
{
  client.publish("c/mapapas/l/closet/simple/", "1");
} 
void doubleclick3() 
{
  client.publish("c/mapapas/l/closet/double/", "1");
} 
void longPressStart3() 
{
  client.publish("c/mapapas/l/closet/long/", "1");
} 

//buton4
void click4() 
{
  client.publish("c/mapapas/l/closet/simple/", "1");
} 
void doubleclick4() 
{
  client.publish("c/mapapas/l/closet/double/", "1");
} 
void longPressStart4() 
{
  client.publish("c/mapapas/l/closet/long/", "1");
} 