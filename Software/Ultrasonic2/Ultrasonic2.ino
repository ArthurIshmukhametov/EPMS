//-------- Libraris and defines --------
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <EmonLib.h>
#include "Ultrasonic.h"
#include <Wire.h>
#define TRIGGER_PIN  5         // Синий кабель. Оптимально 5 (1 пин)
#define ECHO_PIN     4         // Серый кабель. Оптимально 4 (2 пин)

//-------- Turning on devices --------
Ultrasonic ultrasonic(5, 4);
EnergyMonitor emon;
//ESP8266WebServer server(80);
SoftwareSerial rfid = SoftwareSerial(D3, D4);

WiFiClient client;
HTTPClient http;


//-------- Variabels --------
const char* ssid = "Diablos";
const char* password = "vivadiablosnetwork";

bool debug = true;        //для проверки установить true

const String stateOff    = "off";
const String stateOn     = "on";
const String overCurrent = "hight";
const String emptyTag    = "000000000000";


const double low = 1.1;
const double hi  = 16;

bool   oldLevel    = false;              // Предыдущее состояние глубины. False - мало воды, True - намальна
String oldState    = stateOff;           // Предыдущее состояние тока
String oldTag      = emptyTag;              // Предыдущий тэг

unsigned long newtime, oldtime;

String proj = "{\"project\":\"snsz\"}";


//=======================================================================
//                              Setup
//=======================================================================
void setup()
{
  Serial.begin(9600);

  emon.current(0, 30);

  pinMode(TRIGGER_PIN, OUTPUT);   // Назначение
  pinMode(ECHO_PIN, INPUT);       // пинов для
  pinMode(BUILTIN_LED, OUTPUT);   // у/з локатора

  // подключение к wi-fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  { delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Server started: ");
  Serial.println(WiFi.localIP());

  // включение
  rfid.begin(9600);
  Serial.println("RFID Ready to listen");

}

//=======================================================================
//                               Loop
//=======================================================================
void loop() {

  newtime = millis();
  if (newtime - oldtime > 1000) {

    double Irms         = emon.calcIrms(1480);
    String tagID        = readTag();
    String currentTag   = checkTag(tagID, Irms);
    String currentState = checkState(Irms);
    long distance       = getDistance();
    bool currentLevel   = checkLevel(distance);

    //Проверка пяти переменных и состояний
    if (debug) {
      Serial.println("Distance: " + String(distance));
      Serial.println("Amperage: " + String(Irms));
      Serial.println("tagID: " + tagID);
      Serial.println("currentTag: " + currentTag);
      Serial.println("previosTag: " + oldTag);
    }

    // Формирование строки для отправки данных
    if ((currentState != oldState) || (currentLevel != oldLevel) || (currentTag != oldTag)) {

      String params = "1157628&1157630&="
                      + String(distance)
                      + "&1157629&="
                      + Irms
                      + "&1157631&="
                      + currentTag;

      oldTag   = currentTag;
      oldState = currentState;
      oldLevel = currentLevel;

      String token = getToken();

      if (token != "") {
        sendStatus(token, params);
        Serial.println(ESP.getFreeHeap());
      }
    }
    oldtime = newtime;
  }
  yield();
}
