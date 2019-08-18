/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
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

const int BAUD = 115200;
const int SECONDS_LOOP_DELAY = 60 * 10;
const int PUMP_PIN = 5;

// Time in milliseconds
//const unsigned long loopDelayNormalSecs = 600;
const unsigned long  loopDelayNormalSecs = 6;
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

#define MODULE_COUNT 1
Module modules[MODULE_COUNT] = {
  // .   ID .  TYPE .   RD.SEV.MH. ML  SL .SH  %  PUMP
  Module('1', "Unknown", 0, 5, 40, 70, 12, 675, 0, false),
};

void setup()
{
    setupSerial();
    setupPins();
    setupWifi();
    delay(1000);
    client.setServer(mqtt_server, mqtt_port);
    delay(1000);
    setupMqtt(); 
    Serial.println("The setup is complete");
    delay(1000);
}

void loop()
{
    readSensors();

    int loops = 0;
    while (loops < SECONDS_LOOP_DELAY){
      delay(1000);
    }
}

void setupPins(){
  
  for (int i = 0; i < (MODULE_COUNT); i++)
  {
    Serial.print("Pin ");
    Serial.print(modules[i].servoPin);
    pinMode(i, OUTPUT);
    Serial.print(" is set to OUTPUT\n");
    digitalWrite(i, HIGH);
    delay(100);
    modules[i].isPumping = false;
  }
//  pinMode(pumpPin, OUTPUT);
//  digitalWrite(pumpPin, LOW);
  }

void printSystemStats(){
  Serial.print("Chip Id ");
  Serial.println(ESP.getChipId());
  Serial.print("Flash Chip Id ");
  Serial.println(ESP.getFlashChipId());
  Serial.print("Flash Chip Size ");
  Serial.println(ESP.getFlashChipSize());
  Serial.print("Flash Chip Speed ");
  Serial.println(ESP.getFlashChipSpeed());
  Serial.print("Cycle Count ");
  Serial.println(ESP.getCycleCount() );
  Serial.print("Module Vcc ");
  Serial.println(ESP.getVcc());
  }

void setupMqtt()
{
    // Loop until we're reconnected
    while (!client.connected()){

        // Create a random client ID
        String clientId;
        clientId.reserve(30);
        clientId = "ESP8266Client-oderstr01";

        Serial.print("Attempting MQTT connection with details: ID");
        Serial.print(clientId.c_str());
        Serial.print(" and  username: ");
        Serial.print(MQTT_USER);
        Serial.print(" and password ");
        Serial.println( MQTT_PASSWORD);

        // Attempt to connect
        if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD))
        {
            Serial.println("connected");

            //Once connected, publish an announcement...
            client.publish(MQTT_SERIAL_PUBLISH_TEST, "ESP8266 client");
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

        for (int i = 0; i < MODULE_COUNT; i++)
        {
          Module currentModule = modules[i];

    strcpy(finalString, "plant_system,city=Berlin,city=Berlin,room=andrews,thing=esp8266,plant_type=");
  
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

    byte servoPinState = digitalRead(currentModule.servoPin);
    if (servoPinState == LOW)
    {
        strcat(finalString, ",servo=1");
    }
    else
    {
        strcat(finalString, ",servo=0");
    }

    byte pumpPinState = digitalRead(PUMP_PIN);

    if (pumpPinState == LOW)
    {
        strcat(finalString, ",pump=0");
    }
    else
    {
        strcat(finalString, ",pump=1");
    }
    delay(100);
    Serial.println(finalString);
    client.publish(MQTT_SERIAL_PUBLISH_PLANTS, finalString);

    // End of loop of modules
    }

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



void setupSerial()
{
    Serial.begin(115200);
    Serial.print("Connecting to ");
}
