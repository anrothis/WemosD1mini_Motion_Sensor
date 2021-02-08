#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <credentials.h>

// #define DEBUG       // Debug Node
#ifdef DEBUG
 #define DEBUG_PRINT(x) Serial.print (x)
 #define DEBUG_PRINTDEC(x)  Serial.print (x, DEC)
 #define DEBUG_PRINTLN(x) Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTLN(x)
#endif


//*----------------------------------------------------------------------------------------------*//
//*-----------------------------    Init Classes & Variables   ----------------------------------*//
//*----------------------------------------------------------------------------------------------*//
// const char* ssid = "xxxxxxxxxxxx";
// const char* password = "xxxxxxxxxxxxxx";

const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};

WiFiClient espClient;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "192.168.2.128", utcOffsetInSeconds);

void setup_wifi() {

  delay(10);
  DEBUG_PRINTLN();
  DEBUG_PRINT("Connecting to ");
  DEBUG_PRINTLN(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG_PRINT(".");
  }

  randomSeed(micros());

  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("WiFi connected");
  DEBUG_PRINTLN("IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());
}

void setup() {
  pinMode(14, INPUT);
  pinMode(5, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  #ifdef DEBUG
    Serial.begin(115200);
    DEBUG_PRINTLN(F("\nBuildtime"));
    DEBUG_PRINTLN(__DATE__);
    DEBUG_PRINTLN(__TIME__);
  #endif

  //*----------------------------------------------------------------------------------------------*//
  //*---------------------------    Setup WiFi & MQTT    ------------------------------------------*//
  //*----------------------------------------------------------------------------------------------*//
  setup_wifi();
  timeClient.begin();

  ArduinoOTA.setHostname("IoT_Movment_Light");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
  long unsigned int time = millis();
  int setTimer = 30;
  int duration = setTimer * 1000;
  
  timeClient.update();
  int hour = timeClient.getHours();
  
  if(digitalRead(14)== HIGH && (hour >= 17 || hour < 8))
  {
    while((duration > millis() - time) && (millis() - time >= 0))
    {
      digitalWrite(5,HIGH);
      if(digitalRead(14)== HIGH)
      {
        time = millis();
        digitalWrite(LED_BUILTIN, LOW);
        DEBUG_PRINT(F("Movement detected at "));
        DEBUG_PRINTLN(time);
        DEBUG_PRINTLN(timeClient.getFormattedTime());
        DEBUG_PRINTLN(daysOfTheWeek[timeClient.getDay()]);
        delay(500);
        digitalWrite(LED_BUILTIN, HIGH);
      }
      yield();

    }
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(5, LOW);
    delay(100);
  }

}