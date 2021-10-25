#include <OneButton.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <Ethernet.h>

//Static Mac Address
static uint8_t mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xAA };  // Chambre enfant 0x02  //AA TEST
                                                                // master cuisine 0x04
// MQTT Settings //
const char* broker = "ubuntu.jaune.lan"; // MQTT broker

#define DHTTYPE DHT22
#define DHTPIN0  A0 //al main dht22
#define DHTPIN1  A1 //al closet dht22
#define DHTPIN2  A2 //sam main dht22
#define DHTPIN3  A3 //sam closet dht22
OneButton button1(A4, false, false); //al main light switch
OneButton button2(2, false, false); //al closet light switch
OneButton button3(A5, false, false); //sam main light switch
OneButton button4(3, false, false); //sam closet light switch

const int output_pin[6] = { 4, 5, 6, 7, 8, 9 }; //Relay Pinout turn on/off light et chauffage
//4= light main al
//5= light closet al
//6= chauffage main al
//7= light main sam
//8= light closet sam
//9= chauffage main sam

//pinout A0-A5  2-9
const int sendDhtInfo = 30000;    // Dht22 will report every X milliseconds.

//topic ou seront publish les info /// TOPIC SLASH NAMEA SLASH CLIM SLASH MAIN SLASH; ///
#define NAME "Test" //nom publish pour l'adresse ip et le uptime
#define TOPIC "chambre"
#define SLASH "/"
#define NAMEA "A"
#define NAMEB "B"
#define CLIM "climat"
#define MAIN "main"
#define CLOSET "closet"

#define LIGHT "light"
#define SW "switch"
#define STATUS "status"
#define COMMAND "command"
#define HEAT "heat"
#define SIMPLE "simple"
#define DOUBLE "double"
#define LONG "long"
#define UN "1"
#define ZERO "0"
#define UPTIME "uptime"
#define IP "ip"
//Topic du Board pour adresse IP
const char BOARD_TOPIC0[] PROGMEM = TOPIC SLASH NAME SLASH IP;
const char BOARD_TOPIC1[] PROGMEM = TOPIC SLASH NAME SLASH UPTIME;
const char* const BOARD_TOPIC[] PROGMEM = { BOARD_TOPIC0, BOARD_TOPIC1 };
// En Theorie rien en dessous de ca devrait etre touche //

unsigned long lastSend = 0; 

//Uptime counter
long Day=0;
int Hour =0;
int Minute=0;
int Second=0;
int HighMillis=0;
int Rollover=0;

//input pour les DHT22 temperature et humidity
const char DHT_TOPICA1[] PROGMEM = TOPIC SLASH NAMEA SLASH CLIM SLASH MAIN SLASH;
const char DHT_TOPICA2[] PROGMEM = TOPIC SLASH NAMEA SLASH CLIM SLASH CLOSET SLASH;
const char DHT_TOPICB1[] PROGMEM = TOPIC SLASH NAMEB SLASH CLIM SLASH MAIN SLASH;
const char DHT_TOPICB2[]  PROGMEM = TOPIC SLASH NAMEB SLASH CLIM SLASH CLOSET SLASH;
const char* const DHT_TOPIC[4]  PROGMEM = { DHT_TOPICA1, DHT_TOPICA2, DHT_TOPICB1, DHT_TOPICB2 };
//Les output pour driver les relay lumieres et chauffage
const char SUBRELAY0[] = TOPIC SLASH NAMEA SLASH LIGHT SLASH MAIN SLASH STATUS SLASH;
const char SUBRELAY1[] = TOPIC SLASH NAMEA SLASH LIGHT SLASH CLOSET SLASH STATUS SLASH;
const char SUBRELAY2[] = TOPIC SLASH NAMEA SLASH HEAT SLASH MAIN SLASH STATUS SLASH;
const char SUBRELAY3[] = TOPIC SLASH NAMEB SLASH LIGHT SLASH MAIN SLASH STATUS SLASH;
const char SUBRELAY4[] = TOPIC SLASH NAMEB SLASH LIGHT SLASH CLOSET SLASH STATUS SLASH;
const char SUBRELAY5[] = TOPIC SLASH NAMEB SLASH HEAT SLASH MAIN SLASH STATUS SLASH;;
const char* const SUBRELAY[] =  { SUBRELAY0, SUBRELAY1, SUBRELAY2 ,SUBRELAY3 ,SUBRELAY4 ,SUBRELAY5 };

const char COMRELAY0[] = TOPIC SLASH NAMEA SLASH LIGHT SLASH MAIN SLASH COMMAND SLASH;
const char COMRELAY1[] = TOPIC SLASH NAMEA SLASH LIGHT SLASH CLOSET SLASH COMMAND SLASH;
const char COMRELAY2[] = TOPIC SLASH NAMEA SLASH HEAT SLASH MAIN SLASH COMMAND SLASH;
const char COMRELAY3[] = TOPIC SLASH NAMEB SLASH LIGHT SLASH MAIN SLASH COMMAND SLASH;
const char COMRELAY4[] = TOPIC SLASH NAMEB SLASH LIGHT SLASH CLOSET SLASH COMMAND SLASH;
const char COMRELAY5[] = TOPIC SLASH NAMEB SLASH HEAT SLASH MAIN SLASH COMMAND SLASH;;
const char* const COMRELAY[] =  { COMRELAY0, COMRELAY1, COMRELAY2 ,COMRELAY3 ,COMRELAY4 ,COMRELAY5 };

// Input pour les boutton de lumiere (single, double et long)
const char INPUTPUBLISH0[] PROGMEM = TOPIC SLASH NAMEA SLASH SW SLASH MAIN SLASH SIMPLE SLASH;
const char INPUTPUBLISH1[] PROGMEM = TOPIC SLASH NAMEA SLASH SW SLASH MAIN SLASH DOUBLE SLASH;
const char INPUTPUBLISH2[] PROGMEM = TOPIC SLASH NAMEA SLASH SW SLASH MAIN SLASH LONG SLASH;
const char INPUTPUBLISH3[] PROGMEM = TOPIC SLASH NAMEA SLASH SW SLASH CLOSET SLASH SIMPLE SLASH;
const char INPUTPUBLISH4[] PROGMEM = TOPIC SLASH NAMEA SLASH SW SLASH CLOSET SLASH DOUBLE SLASH;
const char INPUTPUBLISH5[] PROGMEM = TOPIC SLASH NAMEA SLASH SW SLASH CLOSET SLASH LONG SLASH;
const char INPUTPUBLISH6[] PROGMEM = TOPIC SLASH NAMEB SLASH SW SLASH MAIN SLASH SIMPLE SLASH;
const char INPUTPUBLISH7[] PROGMEM = TOPIC SLASH NAMEB SLASH SW SLASH MAIN SLASH DOUBLE SLASH;
const char INPUTPUBLISH8[] PROGMEM = TOPIC SLASH NAMEB SLASH SW SLASH MAIN SLASH LONG SLASH;
const char INPUTPUBLISH9[] PROGMEM = TOPIC SLASH NAMEB SLASH SW SLASH CLOSET SLASH SIMPLE SLASH;
const char INPUTPUBLISH10[] PROGMEM = TOPIC SLASH NAMEB SLASH SW SLASH CLOSET SLASH DOUBLE SLASH;
const char INPUTPUBLISH11[] PROGMEM = TOPIC SLASH NAMEB SLASH SW SLASH CLOSET SLASH LONG SLASH;
const char* const INPUTPUBLISH[] PROGMEM =  {INPUTPUBLISH0, INPUTPUBLISH1, INPUTPUBLISH2, INPUTPUBLISH3, INPUTPUBLISH4, INPUTPUBLISH5,
                                             INPUTPUBLISH6, INPUTPUBLISH7, INPUTPUBLISH8, INPUTPUBLISH9, INPUTPUBLISH10, INPUTPUBLISH11 };

DHT dht[] = { { DHTPIN0, DHTTYPE }, { DHTPIN1, DHTTYPE }, { DHTPIN2, DHTTYPE }, { DHTPIN3, DHTTYPE } }; // { DHTPIN4, DHTTYPE }, { DHTPIN5, DHTTYPE } };

EthernetClient ethclient;
PubSubClient client(ethclient);

void callback(char* topic, byte* payload, unsigned int length)
{
  //Serial.print(topic);
  byte output_number = payload[0] - '0';

  for (int i = 0; i < sizeof(SUBRELAY) / sizeof(SUBRELAY[0]); i++)
  {
    int strcomparison = strcmp(topic, COMRELAY[i]);
    if (strcomparison == 0)
    {
      if (output_number == 1)// || ((char)payload[0] == '1'))
      {
        digitalWrite(output_pin[i], HIGH);
       // ConvertAndSend (&SUBRELAY[i], UN);
      client.publish(SUBRELAY[i],"ON");
      }
      if (output_number == 0)
      {
        digitalWrite(output_pin[i], LOW);
        //ConvertAndSend (&SUBRELAY [i], ZERO);
        client.publish(SUBRELAY[i],"OFF");

      }
    }
  }
}

void reconnect() 
{
  while (!client.connected()) 
  {
    char clientBuffer[60];
    String clientString = String(Ethernet.localIP()[0]) + "." + String(Ethernet.localIP()[1]) + "." + String(Ethernet.localIP()[2]) + "." + String(Ethernet.localIP()[3]);
    clientString.toCharArray(clientBuffer, clientString.length() + 1);
    if (client.connect(clientBuffer)) 
    {
      ConvertAndSend (&BOARD_TOPIC[0], clientBuffer);
      //Serial.print(clientBuffer);
      for (int i = 0; i < sizeof(SUBRELAY) / sizeof(SUBRELAY[0]); i++)
      {
        client.subscribe(COMRELAY[i]);
        //Serial.print(F("Relay Subscribe : "));
        //Serial.println(SUBRELAY[i]);
      }
    }
  }
}

void setup()
{
  //Serial.begin(115200);
  //Serial.println(F("Getting IP"));
  while (!Ethernet.begin(mac))  { }
  //Serial.println(F("Ip Acquired"));

  enable_and_reset_all_outputs(); //Reset and Set all pin on OUTPUT mode

  client.setServer(broker, 1883);
  client.setCallback(callback);

  for (int i = 0; i < sizeof(DHT_TOPIC) / sizeof(DHT_TOPIC[0]); i++)
    {
      dht[i].begin();
    }

  #pragma region Initialize the Buton
  button1.attachClick(click1);
  button1.attachDoubleClick(doubleclick1);
  button1.attachLongPressStart(longPressStart1);

  button2.attachClick(click2);
  button2.attachDoubleClick(doubleclick2);
  button2.attachLongPressStart(longPressStart2);

  button3.attachClick(click3);
  button3.attachDoubleClick(doubleclick3);
  button3.attachLongPressStart(longPressStart3);

  button4.attachClick(click4);
  button4.attachDoubleClick(doubleclick4);
  button4.attachLongPressStart(longPressStart4);
  #pragma endregion Toute l'initialisation des boutons click,doubleclick, longclick
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
    print_Uptime();
  }
  uptime();

  button1.tick();
  button2.tick();
  button3.tick();
  button4.tick();

  client.loop();
}
void readDHT() //Lecture des Senseur Temperature, humidite et calcul du heat index (Temperature ressentie)

{
  for (int i = 0; i < sizeof(DHT_TOPIC) / sizeof(DHT_TOPIC[0]); i++)
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

    ConvertAndSend (&DHT_TOPIC [i], payload);
  }
}

void ConvertAndSend (const char * const * topic, String payload) //Voodoo magic publish
  {
  const char *ptr = reinterpret_cast<const char *>(pgm_read_ptr (topic));  // pointer to message

  String temp = reinterpret_cast<const __FlashStringHelper *>(ptr);
  const char *test = temp.c_str();
  client.publish(test ,  payload.c_str());
  //Serial.print(test);
  //Serial.print(payload);
  //Serial.println();
  } 

void enable_and_reset_all_outputs() //Setup des boutons
{
  for (int i = 0; i < sizeof(SUBRELAY) / sizeof(SUBRELAY[0]); i++)
  {
    pinMode(output_pin[i], INPUT_PULLUP);
    pinMode(output_pin[i], OUTPUT);
    digitalWrite(output_pin[i], LOW);
  }
}

#pragma region Button Gestion du simple double et long press

//Boutton1
void click1() 
  {
    ConvertAndSend (&INPUTPUBLISH[0], UN);
  } 
void doubleclick1() 
  {
  ConvertAndSend (&INPUTPUBLISH[1], UN);
  } 
void longPressStart1() 
  {
  ConvertAndSend (&INPUTPUBLISH[2], UN);
  } 
//Boutton2
void click2() 
  {
  ConvertAndSend (&INPUTPUBLISH[3], UN);
  } 
void doubleclick2() 
  {
  ConvertAndSend (&INPUTPUBLISH[4], UN);
  } 
void longPressStart2() 
{
ConvertAndSend (&INPUTPUBLISH[5], UN);
} 
//Buton3
void click3() 
  {
  ConvertAndSend (&INPUTPUBLISH[6], UN);
  } 
void doubleclick3() 
  {
  ConvertAndSend (&INPUTPUBLISH[7], UN);
  } 
void longPressStart3() 
{
ConvertAndSend (&INPUTPUBLISH[8], UN);
} 
//buton4
void click4() 
  {
  ConvertAndSend (&INPUTPUBLISH[9], UN);
  } 
void doubleclick4() 
  {
  ConvertAndSend (&INPUTPUBLISH[10], UN);
  } 
void longPressStart4() 
{
ConvertAndSend (&INPUTPUBLISH[11], UN);
} 
#pragma endregion Button

void uptime() //Conteur pour le UPTIME (temps depuis qu'il est en marche)
{
  //** Making Note of an expected rollover *****//   
  if(millis()>=3000000000)
  { 
    HighMillis=1;
  }
  //** Making note of actual rollover **//
  if(millis()<=100000 && HighMillis==1)
  {
    Rollover++;
    HighMillis=0;
  } 

long secsUp = millis()/1000;

Second = secsUp%60;

Minute = (secsUp/60)%60;

Hour = (secsUp/(60*60))%24;

Day = (Rollover*50)+(secsUp/(60*60*24));  //First portion takes care of a rollover [around 50 days]
                       
};

void print_Uptime(){ //imprime et publish au format JSON

    String payload = "{";
    payload += "\"uptime\":"; payload += String(Day); payload += ",";
    payload += "\"Hour\":";   payload += (Hour < 10 ? "0" : "")   + String(Hour); payload += ",";
    payload += "\"Minute\":"; payload += (Minute < 10 ? "0" : "") + String(Minute); payload += ",";
    payload += "\"Second\":"; payload += (Second < 10 ? "0" : "") + String(Second);
    payload += "}";

    ConvertAndSend (&BOARD_TOPIC[1], payload);
};