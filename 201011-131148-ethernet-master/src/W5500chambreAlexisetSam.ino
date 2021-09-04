#include <OneButton.h>

#include <PubSubClient.h>
#include <Ethernet.h>

//Static Mac Address
static uint8_t mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0x04 };  // Chambre enfant 0x02 /0x04 = test
                                                                // Master cuisine 0x01
// MQTT Settings //
const char* broker = "ubuntu.jaune.lan"; // MQTT broker

OneButton button1(A4, false, false); //master main light switch
OneButton button2(2, false, false); //master closet light switch
OneButton button3(A5, false, false); //cuisine main light switch
OneButton button4(3, false, false); //cuisine closet light switch

const int output_pin[6] = { 4, 5, 6, 7, 8, 9 }; //Relay Pinout turn on/off light et chauffage
//4= light main master
//5= light closet master
//6= chauffage main master
//7= light main cuisine
//8= light closet cuisine
//9= chauffage main cuisine

//pinout A0-A5  2-9
const int sendDhtInfo = 30000;    // Dht22 will report every X milliseconds.

//topic ou seront publish les info /// TOPIC SLASH NAMEA SLASH CLIM SLASH MAIN SLASH; ///
#define NAME "arduino-03" //nom publish pour l'adresse ip et le uptime
#define TOPIC "chambre"
#define SLASH "/"
#define NAMEA "test1"
#define NAMEB "test2"
#define CLIM "climat"
#define MAIN "main"
#define CLOSET "closet"

#define LIGHT "light"
#define STATUS "status"
#define HEAT "heat"
#define SIMPLE "simple"
#define DOUBLE "double"
#define LONG "long"
#define UN "1"
#define ZERO "0"
#define UPTIME "uptime"
#define IP "ip"
//Topic du Board pour adresse IP
const char BOARD_TOPIC0[] PROGMEM = TOPIC SLASH NAME IP;
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

//Les output pour driver les relay lumieres et chauffage
const char SUBSCRIBERELAY0[] = TOPIC SLASH NAMEA SLASH LIGHT SLASH MAIN SLASH STATUS SLASH;
const char SUBSCRIBERELAY1[] = TOPIC SLASH NAMEA SLASH LIGHT SLASH CLOSET SLASH STATUS SLASH;
const char SUBSCRIBERELAY2[] = TOPIC SLASH NAMEA SLASH HEAT SLASH MAIN SLASH STATUS SLASH;
const char SUBSCRIBERELAY3[] = TOPIC SLASH NAMEB SLASH LIGHT SLASH MAIN SLASH STATUS SLASH;
const char SUBSCRIBERELAY4[] = TOPIC SLASH NAMEB SLASH LIGHT SLASH CLOSET SLASH STATUS SLASH;
const char SUBSCRIBERELAY5[] = TOPIC SLASH NAMEB SLASH HEAT SLASH MAIN SLASH STATUS SLASH;;
const char* const SUBSCRIBERELAY[] =  { SUBSCRIBERELAY0, SUBSCRIBERELAY1, SUBSCRIBERELAY2 ,SUBSCRIBERELAY3 ,SUBSCRIBERELAY4 ,SUBSCRIBERELAY5 };

// Input pour les boutton de lumiere (single, double et long)
const char INPUTPUBLISH0[] PROGMEM = TOPIC SLASH NAMEA SLASH LIGHT SLASH MAIN SLASH SIMPLE SLASH;
const char INPUTPUBLISH1[] PROGMEM = TOPIC SLASH NAMEA SLASH LIGHT SLASH MAIN SLASH DOUBLE SLASH;
const char INPUTPUBLISH2[] PROGMEM = TOPIC SLASH NAMEA SLASH LIGHT SLASH MAIN SLASH LONG SLASH;
const char INPUTPUBLISH3[] PROGMEM = TOPIC SLASH NAMEA SLASH LIGHT SLASH CLOSET SLASH SIMPLE SLASH;
const char INPUTPUBLISH4[] PROGMEM = TOPIC SLASH NAMEA SLASH LIGHT SLASH CLOSET SLASH DOUBLE SLASH;
const char INPUTPUBLISH5[] PROGMEM = TOPIC SLASH NAMEA SLASH LIGHT SLASH CLOSET SLASH LONG SLASH;
const char INPUTPUBLISH6[] PROGMEM = TOPIC SLASH NAMEB SLASH LIGHT SLASH MAIN SLASH SIMPLE SLASH;
const char INPUTPUBLISH7[] PROGMEM = TOPIC SLASH NAMEB SLASH LIGHT SLASH MAIN SLASH DOUBLE SLASH;
const char INPUTPUBLISH8[] PROGMEM = TOPIC SLASH NAMEB SLASH LIGHT SLASH MAIN SLASH LONG SLASH;
const char INPUTPUBLISH9[] PROGMEM = TOPIC SLASH NAMEB SLASH LIGHT SLASH CLOSET SLASH SIMPLE SLASH;
const char INPUTPUBLISH10[] PROGMEM = TOPIC SLASH NAMEB SLASH LIGHT SLASH CLOSET SLASH DOUBLE SLASH;
const char INPUTPUBLISH11[] PROGMEM = TOPIC SLASH NAMEB SLASH LIGHT SLASH CLOSET SLASH LONG SLASH;
const char* const INPUTPUBLISH[] PROGMEM =  {INPUTPUBLISH0, INPUTPUBLISH1, INPUTPUBLISH2, INPUTPUBLISH3, INPUTPUBLISH4, INPUTPUBLISH5,
                                             INPUTPUBLISH6, INPUTPUBLISH7, INPUTPUBLISH8, INPUTPUBLISH9, INPUTPUBLISH10, INPUTPUBLISH11 };

EthernetClient ethclient;
PubSubClient client(ethclient);

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.println(topic);
  byte output_number = payload[0] - '0';

  for (int i = 0; i < sizeof(SUBSCRIBERELAY) / sizeof(SUBSCRIBERELAY[0]); i++)
  {
    int strcomparison = strcmp(topic, SUBSCRIBERELAY[i]);
    if (strcomparison == 0)
    {
      if (output_number == 1)// || ((char)payload[0] == '1'))
      
      {
        digitalWrite(output_pin[i], HIGH);
        ConvertAndSend (&SUBSCRIBERELAY [i], UN);
      }
      if (output_number == 0)
      {
        digitalWrite(output_pin[i], LOW);
        ConvertAndSend (&SUBSCRIBERELAY [i], ZERO);

      }
    }

  }
}

void reconnect() 
{
  while (!client.connected()) 
  {
    Serial.println();
    char clientBuffer[60];
    String clientString = String(Ethernet.localIP()[0]) + "." + String(Ethernet.localIP()[1]) + "." + String(Ethernet.localIP()[2]) + "." + String(Ethernet.localIP()[3]);
    clientString.toCharArray(clientBuffer, clientString.length() + 1);
    if (client.connect(clientBuffer)) 
    {

      //Serial.println(clientBuffer);
      ConvertAndSend (&BOARD_TOPIC [0], clientBuffer);

      for (int i = 0; i < sizeof(SUBSCRIBERELAY) / sizeof(SUBSCRIBERELAY[0]); i++)
      {
        client.subscribe(SUBSCRIBERELAY[i]);
        Serial.print(F("Relay Subscribe : "));
        Serial.println(SUBSCRIBERELAY[i]);
      }
    }
  }
}
void setup()
{
  Serial.begin(115200);
  Serial.println(F("Getting IP"));
  while (!Ethernet.begin(mac))  { }
  Serial.println(F("Ip Acquired"));

  enable_and_reset_all_outputs(); //Reset and Set all pin on OUTPUT mode

  client.setServer(broker, 1883);
  client.setCallback(callback);

  button1.attachClick(click1);
  button1.attachDoubleClick(doubleclick1);
  button1.attachLongPressStart(longPressStart1);
  //button1.attachLongPressStop(longPressStop1);
  //button1.attachDuringLongPress(longPress1);

  button2.attachClick(click2);
  button2.attachDoubleClick(doubleclick2);
  //button2.attachLongPressStart(longPressStart2);

   button3.attachClick(click3);
   button3.attachDoubleClick(doubleclick3);
   //button3.attachLongPressStart(longPressStart3);

   button4.attachClick(click4);
   button4.attachDoubleClick(doubleclick4);
   //button4.attachLongPressStart(longPressStart4);
  
  reconnect();

}

void loop()
{
  if (!client.connected())
  {
   Serial.println(F("reconnect"));
   reconnect();
  }

  if (millis() - lastSend > sendDhtInfo)
  {
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

void ConvertAndSend (const char * const * topic, String payload)
  {
  const char *ptr = reinterpret_cast<const char *>(pgm_read_ptr (topic));  // pointer to message

  String temp = reinterpret_cast<const __FlashStringHelper *>(ptr);
  const char *test = temp.c_str();
  client.publish(test ,  payload.c_str());
   Serial.print(test);
  Serial.print(payload);
  Serial.println();

  } 

void enable_and_reset_all_outputs()
{
  for (int i = 0; i < sizeof(SUBSCRIBERELAY) / sizeof(SUBSCRIBERELAY[0]); i++)
  {
    pinMode(output_pin[i], INPUT_PULLUP);
    pinMode(output_pin[i], OUTPUT);
    digitalWrite(output_pin[i], LOW);
  }
}


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
// void longPressStart2() 
// {
//ConvertAndSend (&INPUTPUBLISH[5], UN);
// } 

//Buton3
void click3() 
  {
  ConvertAndSend (&INPUTPUBLISH[6], UN);
  } 
void doubleclick3() 
  {
  ConvertAndSend (&INPUTPUBLISH[7], UN);
  } 
// void longPressStart3() 
// {
//ConvertAndSend (&INPUTPUBLISH[8], UN);
// } 

//buton4
void click4() 
  {
  ConvertAndSend (&INPUTPUBLISH[9], UN);
  } 
void doubleclick4() 
  {
  ConvertAndSend (&INPUTPUBLISH[10], UN);
  } 
// void longPressStart4() 
// {
//ConvertAndSend (&INPUTPUBLISH[11], UN);
// } 




//Conteur pour le UPTIME (temps depuis qu'il est en marche)
void uptime()
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

//imprime et publish au format JSON
void print_Uptime(){

    String payload = "{";
    payload += "\"uptime\":"; payload += String(Day); payload += ",";
    payload += "\"Hour\":";   payload += (Hour < 10 ? "0" : "")   + String(Hour); payload += ",";
    payload += "\"Minute\":"; payload += (Minute < 10 ? "0" : "") + String(Minute); payload += ",";
    payload += "\"Second\":"; payload += (Second < 10 ? "0" : "") + String(Second);
    payload += "}";

    //client.publish("chambre/test/uptime/", payload.c_str());
    Serial.print(payload);
    Serial.println();

    ConvertAndSend (&BOARD_TOPIC[1], payload);
};