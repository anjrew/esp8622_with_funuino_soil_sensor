/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK  "your-password"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

const char* host = "djxmmx.net";
const uint16_t port = 17;

const int BAUD = 115200;
const int LOOP_DELAY = 2000;
const int PUMP_PIN = 15;

// Time in milliseconds
const unsigned long  loopDelayNormalSecs = 600;
//const unsigned long  loopDelayNormalSecs = 6;
const int loopDelayPumpmilli = 100;


class Module
{
public:
    Module( char, char*, int, int, int, int, int, int, int, bool);
    char id;
    char* plantType;
    int moistureSettingLow;
    int moistureSettingHigh;
    int sensorLowerValue;
    int sensorUpperValue;
    int readPin;
    int currentPercentage;
    int servoPin;
    bool isPumping;

};

/// Constructor for each module
Module::Module(char a, char* b, int c, int d, int e, int f, int g, int h, int i, bool j )
{
    id = a;
    plantType = b;
    readPin = c;
    servoPin = d;
    moistureSettingLow = e;
    moistureSettingHigh = f;
    sensorLowerValue = g;
    sensorUpperValue = h;
    currentPercentage = i;
    isPumping = j;
}

Module plantOne = Module('1', "Unknown", 0, 5, 40, 70, 12, 449, 0, false);


//#define MODULE_COUNT 7
//Module modules[MODULE_COUNT] = {
//      Module('1', "Unknown", 0, 0, 40, 70, 12, 595, 0, false)
//
////    Module(A1, 0, '2', 70, 3, 640, 323, 40, false, "scindapsus"),    /// Checked sensor values 2/6/2019 Plant two - Hanging plant
////    Module(A2, 0, '3', 70, 4, 622, 312, 40, false, "scindapsus"),
////    Module(A3, 0, '4', 90, 5, 664, 339, 60, false, "tray"),    // Big plant bed
////    Module(A4, 0, '5', 70, 6, 672, 308, 40, false, "bonsai"),     // Checked sensor values 8/5/2019 bonsai
////    Module(A5, 0, '6', 60, 7, 700, 372, 30, false, "cactus"),     // Checked sensor values 2/6/2019 Cactus
////    Module(A6, 0, '7', 70, 8, 597, 287, 40, false, "peace_lily"), // Checked sensor values 8/5/2019 Peace Lily
//};


void setup(){
  setupSerial();
  setupPins();
  }

void loop(){
    readSensors();
    delay(LOOP_DELAY);
  }

void readSensors(){

    bool needsPump = false;

//    for (int i = 0; i < MODULE_COUNT; i++)
//    {
//      Module currentModule = modules[i];
        Module currentModule = plantOne;
        // Project /
//        Serial.print("<plant_system");
//        //Tags /
//        Serial.print(",city=Berlin");
//        Serial.print(",location=oderstrasse");
//        Serial.print(",room=andrews");
//        Serial.print(",plant_type=");
//        Serial.print(currentModule.plantType);
//        Serial.print(",plant_id=");
//        Serial.print(currentModule.id);
//        
//        Serial.print(" ");
//
//        //Fields
//        Serial.print("servo_pin=");
//        Serial.print(currentModule.servoPin);
//
//        Serial.print(",read_pin=");
//        Serial.print(currentModule.readPin);
//
        Serial.print(",sensor_reading=");
        Serial.print(analogRead(currentModule.readPin));
//
//        Serial.print(",moi_setting_high=");
//        Serial.print(currentModule.moistureSettingHigh);
//
//        Serial.print(",moi_setting_low=");
//        Serial.print(currentModule.moistureSettingLow);

        Serial.print(",sensor_low_value=");
        Serial.print(currentModule.sensorLowerValue);

        Serial.print(",sensor_high_value=");
        Serial.print(currentModule.sensorUpperValue);

        
        currentModule.currentPercentage = convertToPercent(analogRead(currentModule.readPin),currentModule);
//        currentModule.currentPercentage = 

        Serial.print(",moisture_level=");
        Serial.println(currentModule.currentPercentage);

//        Serial.println(currentModule.currentPercentage);
        
//        if (currentModule.currentPercentage < currentModule.moistureSettingLow)
//        {
//            currentModule.isPumping = true;
//            needsPump = true;
//            digitalWrite(currentModule.servoPin, LOW);
//
//            //Opening servo
//            Serial.print(",dead_zone=0");
//        }
//        if (currentModule.currentPercentage >= currentModule.moistureSettingLow && currentModule.currentPercentage <= currentModule.moistureSettingHigh)
//        {
//            if (!currentModule.isPumping)
//            {
//                digitalWrite(currentModule.servoPin, HIGH);
//            }
//
//            //the deadzone
//            Serial.print(",dead_zone=1");
//        }
//        if (currentModule.currentPercentage > currentModule.moistureSettingHigh)
//        {
//            currentModule.isPumping = false;
//            digitalWrite(currentModule.servoPin, HIGH);
//
//            //Opening servo
//            Serial.print(",dead_zone=0");
//        }
//
//        byte servoPinState = digitalRead(currentModule.servoPin);
//        if (servoPinState == LOW)
//        {
//            Serial.print(",servo=1");
//        }
//        else
//        {
//            Serial.print(",servo=0");
//        } 
//
//        byte pumpPinState = digitalRead(PUMP_PIN);
//        
//        if (pumpPinState == LOW)
//        {
//            Serial.println(",pump=0>");
//        }
//        else
//        {
//            Serial.println(",pump=1>");
//        }
//        delay(100);
//    }
//
//    if (needsPump)
//    {
//        digitalWrite(PUMP_PIN, HIGH);
//        delay(loopDelayPumpmilli);
//    }
//    else
//    {
//        digitalWrite(PUMP_PIN, LOW);
//        int p = 0;
//        while (p < loopDelayNormalSecs) {
//        delay(1000);
//        p++;
//      }
//
//    }
}


int convertToPercent(int sensorValue, Module module)
{
    int percentValue = map(sensorValue, module.sensorLowerValue, module.sensorUpperValue, 0, 100);
    return percentValue;
}
  
void setupPins(){
  pinMode(plantOne.readPin, INPUT);
  pinMode(plantOne.servoPin, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
}

void setupSerial() {
  Serial.begin(115200);
  Serial.print("Connecting to ");
}

void setupWifi(){
   
  Serial.println(ssid);
  /* Explicitly set the ESP8266 to be a WiFi-client */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  }

  
void wifiloop() {
  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    delay(5000);
    return;
  }

  // This will send a string to the server
  Serial.println("sending data to server");
  if (client.connected()) {
    client.println("hello from ESP8266");
  }

  // wait for data to be available
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      delay(60000);
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  Serial.println("receiving from remote server");
  // not testing 'client.connected()' since we do not need to send data here
  while (client.available()) {
    char ch = static_cast<char>(client.read());
    Serial.print(ch);
  }

  // Close the connection
  Serial.println();
  Serial.println("closing connection");
  client.stop();

  delay(300000); // execute once every 5 minutes, don't flood remote service
}
