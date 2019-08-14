#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// For Temparature
#include "DHTesp.h"
//#define DHTTYPE DHT22
uint8_t DHTPin = 4;
DHTesp dht;
float Temperature;
float Humidity;

// For RTC
#include <Wire.h> // for I2C with RTC module
#include "RTClib.h"
RTC_DS3231 rtc;
int currentHour = 12;
uint8_t LED_ONE = 2;
uint8_t LED_TWO = 16;


// Update these with values suitable for your network.
// Monkey Park
//const char* ssid = "MonkeyPark";
//const char* password = "MonkeyPark";

// Dads House
//const char* ssid = "BrightBox-9sbpsc";
//const char* password = "tag-dread-tame";

const int baudRate = 115200;

#ifdef __cplusplus
extern "C"
{
#endif

    uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

const char *mqtt_server = "postman.cloudmqtt.com";
#define mqtt_port 11968
#define MQTT_USER "xxtdtmwf"
#define MQTT_PASSWORD "c-0_VSx4qaOv"
#define MQTT_SERIAL_PUBLISH_TEST "test"
#define MQTT_SERIAL_PUBLISH_PLANTS "plants/berlin/oderstrasse/andrew"
#define MQTT_SERIAL_PUBLISH_CPU "things/esp32"
#define MQTT_SERIAL_PUBLISH_PLACE "places/berlin/oderstrasse/andrew"

// Wifi
WiFiClient wifiClient;
char ssid []= "(Don't mention the war)";
char password []= "56939862460419967485";

// HTTP
char *timeApiEndP = "http://worldtimeapi.org/api/ip";

//MQTT
PubSubClient client(wifiClient);

int loops = 0;
const int checkLoops = 2000000;
//const int checkLoops = 80000;
const byte numChars = 255;
char receivedChars[numChars];
boolean newData = false;

void setup()
{
    setupSerialPorts();
    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    reconnect();
    setupRtc();
    setupEnviromentreading();
}

void loop()
{
    client.loop();
    if (loops > checkLoops)
    {
        loops = 0;
        readCpuTemp();
//        readEnviroment();
        // recordOutsideWeather();
        ledFlash();
    }
    loops++;
    readSoftwareSerial2();
}

void setupLeds(){
    pinMode(LED_TWO, OUTPUT);     // Initialize the LED_BUILTIN pin as an outputß
    pinMode(LED_ONE, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
}

void setupEnviromentreading()
{
    dht.setup(DHTPin, DHTesp::DHT22);
}

void setupSerialPorts()
{
    Serial.begin(baudRate);
    Serial1.begin(baudRate);
    Serial1.setRxBufferSize(1024);
    delay(100);
    Serial1.println("Hello, world?");
    delay(100);
    Serial.setTimeout(1000);
}

void setupRtc()
{

    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC");
        while (1)
            ;
    }
    if (rtc.lostPower())
    {
        Serial.println("RTC lost power, lets set the time!");
        // following line sets the RTC to the date &amp; time this sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date &amp; time, for example to set
        // January 21, 2014 at 3am you would call:
        //     rtc.adjust(DateTime(2019, 7, 21, 1, 30, 0));
    }
    rtc.adjust(DateTime(__DATE__, __TIME__));
}

void setup_wifi()
{
    // We start by connecting to a WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void ledFlash(){
    digitalWrite(LED_ONE, LOW);
    delay(500);
    digitalWrite(LED_ONE, HIGH);
    digitalWrite(LED_TWO, LOW);
    delay(500);
    digitalWrite(LED_TWO, HIGH);
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {

        Serial.print("Attempting MQTT connection...");

        // Create a random client ID
        String clientId;
        clientId.reserve(30);
        clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);

        // Attempt to connect
        if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD))
        {
            Serial.println("connected");

            //Once connected, publish an announcement...
            client.publish(MQTT_SERIAL_PUBLISH_TEST, "ESP32 client");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
        }
    }
}

void readSoftwareSerial2()
{

    if (Serial1.available())
    {
        noInterrupts();
        recvWithStartEndMarkers();
        showNewData();
        interrupts();
    }
}

void recvWithStartEndMarkers()
{
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial1.available() > 0 && newData == false)
    {
        rc = Serial1.read();

        if (recvInProgress == true)
        {
            if (rc != endMarker)
            {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars)
                {
                    ndx = numChars - 1;
                }
            }
            else
            {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker)
        {
            recvInProgress = true;
        }
    }
}

void showNewData()
{
    if (newData == true)
    {
        Serial.println(receivedChars);
        client.publish(MQTT_SERIAL_PUBLISH_PLANTS, receivedChars);
        newData = false;
    }
}


//void readEnviroment()
//{
//    TempAndHumidity lastValues = dht.getTempAndHumidity();
//    float temperature = lastValues.temperature;
//    float humidity = lastValues.humidity;
//    char finalString[140];
//    char influxString[120] = "places,thing-id=andrews-esp32-nodemcu,city=berlin,location=oderstrasse,room=andrews,sensor=dht22 temp_c=";
//    char tempString[10];
//    snprintf(tempString, 10, "%f", temperature);
//    char humidString[10];
//    snprintf(humidString, 10, "%f", humidity);
//    strcpy(finalString, influxString);
//    strcat(finalString, tempString);
//    strcat(finalString, ",humidity=");
//    strcat(finalString, humidString);
//    Serial.println(finalString);
//    client.publish(MQTT_SERIAL_PUBLISH_PLACE, finalString);
//    delay(10);
//}

void readCpuTemp()
{
    double tempInC = (temprature_sens_read() - 32) / 1.8;

    char finalString[110];
    char influxString[100] = "things,thing-id=andrews-esp32-nodemcu,city=berlin,location=oderstrasse,room=andrews cpu_temp_c=";
    char tempString[10];
    snprintf(tempString, 10, "%f", tempInC);

    strcpy(finalString, influxString);
    strcat(finalString, tempString);
    Serial.println(finalString);
    client.publish(MQTT_SERIAL_PUBLISH_CPU, finalString);
    delay(10);
}

void resetHeap()
{
//    heap_caps_dump_all();
}




String extractDateTime(String string)
{
    int startIndex = string.lastIndexOf("\"datetime\":\"");
    int endIndex = string.indexOf(",\"client_ip\"");
    return string.substring(startIndex + 12, endIndex - 1);
}
