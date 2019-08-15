/*
    FOR EACH SKETCK SETUP THE PLANTS AND THE DETAILS FOR THE MQTT
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// For Temparature
#include "DHTesp.h"
uint8_t DHTPin = 4;
DHTesp dht;
float Temperature;
float Humidity;

uint8_t LED_ONE = 2;

// WIFI
#include <ESP8266WiFi.h>
const char* ssid = "(Don't mention the war)";
const char* password = "56939862460419967485";
WiFiClient espClient;


//MQTT
PubSubClient client(espClient);
const char *mqtt_server = "postman.cloudmqtt.com";
#define mqtt_port 11968
#define MQTT_USER "xxtdtmwf"
#define MQTT_PASSWORD "c-0_VSx4qaOv"
#define MQTT_SERIAL_PUBLISH_TEST "test"
#define MQTT_SERIAL_PUBLISH_PLANTS "plants/berlin/oderstrasse/andrew"
#define MQTT_SERIAL_PUBLISH_CPU "things"
#define MQTT_SERIAL_PUBLISH_PLACE "places/berlin/oderstrasse/andrew"
#define CLIENT_ID "ESP8266Client-oderstr02"

const int BAUD = 115200;
const int LOOP_DELAY = 30000;
const int PUMP_PIN = 5;

// Time in milliseconds
const unsigned long loopDelayNormalSecs = 600;
//const unsigned long  loopDelayNormalSecs = 6;
const int loopDelayPumpmilli = 100;

int initialised = 0;

class Module
{
public:
    Module(char, char *, int, int, int, int, int, int, int, bool);
    char id;
    char *plantType;
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
Module::Module(char a, char *b, int c, int d, int e, int f, int g, int h, int i, bool j)
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

Module plantOne = Module('1', "Unknown", 0, 5, 40, 70, 12, 560.75, 0, false);

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

void setup()
{
    setupSerial();
    setupPins();
    setupWifi();
    client.setServer(mqtt_server, mqtt_port);
    setupMqtt(); 
    logChipDetails();
}

void logChipDetails() {
 
  Serial.begin(115200);
  Serial.println();
 
  Serial.print("Chip ID: ");
  Serial.println(ESP.getFlashChipId());
 
  Serial.print("Chip Real Size: ");
  Serial.println(ESP.getFlashChipRealSize());
 
  Serial.print("Chip Size: ");
  Serial.println(ESP.getFlashChipSize());
 
  Serial.print("Chip Speed: ");
  Serial.println(ESP.getFlashChipSpeed());
 
  Serial.print("Chip Mode: ");
  Serial.println(ESP.getFlashChipMode());
}

void loop()
{
    readSensors();
    delay(LOOP_DELAY);
}

void setupMqtt()
{
    Serial.print("Keep alive time is: ");
    Serial.println(MQTT_KEEPALIVE);
    // Loop until we're reconnected
    while (!client.connected()){

        Serial.print("Attempting MQTT connection with details: ID");
        Serial.print(CLIENT_ID);
        Serial.print(" and  username: ");
        Serial.print(MQTT_USER);
        Serial.print(" and password ");
        Serial.println( MQTT_PASSWORD);

        // Attempt to connect
        if (client.connect(CLIENT_ID, MQTT_USER, MQTT_PASSWORD))
        {
            Serial.println("connected");

            //Once connected, publish an announcement...
            client.publish(MQTT_SERIAL_PUBLISH_TEST, CLIENT_ID);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
        }
    }
 }


void setupWifi()
{

    Serial.println(ssid);
    /* Explicitly set the ESP8266 to be a WiFi-client */
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void readSensors()
{

    char finalString[400];

    bool needsPump = false;

    //    for (int i = 0; i < MODULE_COUNT; i++)
    //    {
    //      Module currentModule = modules[i];
    Module currentModule = plantOne;

    strcat(finalString, ",city=berlin,location=oderstrasse,room=andrews cpu_temp_c=");


    strcpy(finalString, "plant_system,city=Berlin,city=Berlin,room=andrews,thing=");

    strcat(finalString, CLIENT_ID);

    strcat(finalString, ",plant_type=");

    strcat(finalString, currentModule.plantType);

    strcat(finalString, ",plant_id=");

    char z[20];
    snprintf(z, 20, "%c", currentModule.id);
    strcat(finalString, z);

    strcat(finalString, " ");

    //Fields
    strcat(finalString, "servo_pin=");
    char a[20];
    snprintf(a, 20, "%ld", currentModule.servoPin);
    strcat(finalString, a);

    strcat(finalString, ",read_pin=");
    char b[20];
    snprintf(b, 20, "%ld", currentModule.readPin);
    strcat(finalString, b);

    strcat(finalString, ",sensor_reading=");
    char c[20];
    snprintf(c, 20, "%ld", analogRead(currentModule.readPin));
    strcat(finalString, c);

    strcat(finalString, ",moi_setting_high=");
    char d[20];
    snprintf(d, 20, "%ld", currentModule.moistureSettingHigh);
    strcat(finalString, d);

    strcat(finalString, ",moi_setting_low=");
    char e[20];
    snprintf(e, 20, "%ld", currentModule.moistureSettingLow);
    strcat(finalString, e);

    strcat(finalString, ",sensor_low_value=");
    char f[20];
    snprintf(f, 20, "%ld", currentModule.sensorLowerValue);
    strcat(finalString, f);

    strcat(finalString, ",sensor_high_value=");
    char g[20];
    snprintf(g, 20, "%ld", currentModule.sensorUpperValue);
    strcat(finalString, g);

    currentModule.currentPercentage = convertToPercent(analogRead(currentModule.readPin), currentModule);

    strcat(finalString, ",moisture_level=");
    char h[20];
    snprintf(h, 20, "%ld", currentModule.currentPercentage);
    strcat(finalString, h);

    if (currentModule.currentPercentage < currentModule.moistureSettingLow)
    {
        currentModule.isPumping = true;
        needsPump = true;
        digitalWrite(currentModule.servoPin, LOW);

        //Opening servo
        strcat(finalString, ",dead_zone=0");
    }
    if (currentModule.currentPercentage >= currentModule.moistureSettingLow && currentModule.currentPercentage <= currentModule.moistureSettingHigh)
    {
        if (!currentModule.isPumping)
        {
            digitalWrite(currentModule.servoPin, HIGH);
        }

        //the deadzone
        strcat(finalString, ",dead_zone=1");
    }
    if (currentModule.currentPercentage > currentModule.moistureSettingHigh)
    {
        currentModule.isPumping = false;
        digitalWrite(currentModule.servoPin, HIGH);

        //Opening servo
        strcat(finalString, ",dead_zone=0");
    }

//    byte servoPinState = digitalRead(currentModule.servoPin);
//    if (servoPinState == LOW)
//    {
//        strcat(finalString, ",servo=1");
//    }
//    else
//    {
//        strcat(finalString, ",servo=0");
//    }
//
//    byte pumpPinState = digitalRead(PUMP_PIN);
//
//    if (pumpPinState == LOW)
//    {
//        strcat(finalString, ",pump=0");
//    }
//    else
//    {
//        strcat(finalString, ",pump=1");
//    }
    delay(100);
    Serial.println(finalString);
    client.publish(MQTT_SERIAL_PUBLISH_PLANTS, finalString);

    // EnMd of loop
    //    }

//    if (initialised >= 5)
//    {
//
//        if (needsPump)
//        {
//            digitalWrite(PUMP_PIN, HIGH);
//            delay(loopDelayPumpmilli);
//        }
//        else
//        {
//            digitalWrite(PUMP_PIN, LOW);
//            int p = 0;
//            while (p < loopDelayNormalSecs)
//            {
//                delay(1000);
//                p++;
//            }
//        }
//    }
//    else
//    {
//        initialised++;
//    }
}

int convertToPercent(int sensorValue, Module module)
{
    int percentValue = map(sensorValue, module.sensorLowerValue, module.sensorUpperValue, 0, 100);
    return percentValue;
}

void setupPins()
{
    pinMode(plantOne.readPin, INPUT);
    pinMode(plantOne.servoPin, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);
}

void setupSerial()
{
    Serial.begin(115200);
    Serial.print("Connecting to ");
}
