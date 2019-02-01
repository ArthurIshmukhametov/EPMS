
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

String getToken() {

  String token = "";

  Serial.print("[HTTP] begin...\n");

  //http.begin("http://192.168.0.113:9880/auth/");

  http.begin("http://server-api.cara.bi/authorize/register/");

  Serial.print("[HTTP] POST auth...\n");

  int httpCode = http.POST(proj);

  // httpCode will be negative on error
  if (httpCode > 0) {
    if (debug) {
      Serial.printf("[HTTP] POST auth... code: %d\n", httpCode);
    }

    if (httpCode == HTTP_CODE_OK) {

      StaticJsonBuffer<512> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(http.getString());

      if (!root.success()) {
        Serial.println(F("Parsing failed!"));
        http.end();
        return token;
      }

      token = root["token"].as<char*>();

      // Extract values
      Serial.println(F("Response token:"));
      Serial.println(token);
    }
  } else {
    Serial.printf("[HTTP] POST auth... failed, error: %s\n", http.errorToString(httpCode).c_str());
    http.end();
    return token;
  }

  http.end();
  return token;
}

void sendStatus(String token, String params) {

  String payload = "{\"token\": \""
                   + token
                   + "\",\"queryName\":\"SNSZ_IMPORT_SENSOR_DATA\",\"count\":20,\"params\":{\"IN_CLOB\":\""
                   + params
                   + "\"}}";

  //http.begin("http://192.168.0.113:9880/query/");

  http.begin("http://server-api.cara.bi/query/run/");
  Serial.print("[HTTP] POST query...\n");

  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    if (debug) {
      Serial.printf("[HTTP] POST query... code: %d\n", httpCode);
    }

    if (httpCode == HTTP_CODE_OK) {
      Serial.println(httpCode);

      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] POST query... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

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

long getDistance() {
  long duration, distance;
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration / 2) / 29.1; //??
  return distance;
}

bool checkLevel(long distance) {
  return ((distance >= 0) && (distance <= 20)) ? false : true;
}

//Функция чтения RFID
String readTag() {

  String msg = "";
  int data;

  if (rfid.available() > 0) {
    delay(100);               // Необходима чтобы дать данным пройти через аналоговый буфер
    data = rfid.read();
    if (data == 2) { // Начало передачи
      for (int z = 0 ; z < 12 ; z++) {    // Чтение метки
        data = rfid.read();
        msg += char(data);
      }
    }
    data = rfid.read();
    rfid.flush();
    if (data == 3) {     // Конец передачи
      return msg;
    }
  }                       // Выдача метки
  return emptyTag;           //Выдача, если метка не найдена
}

String checkTag(String tagID, double Irms) {

  String tag = emptyTag;

  if (tagID != emptyTag) {
    tag = tagID;
  }
  if (Irms < 1.1) {
    tag = emptyTag;
  }
  return tag;
}

// Функция определения состояния
String checkState(double value) {

if ((value >= 0) && (value < low)) {
 return stateOff;
 }

if ((value >= low) && (value < hi)) {
    return stateOn;
  }

 return overCurrent;
}
