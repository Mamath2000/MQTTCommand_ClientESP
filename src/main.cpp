#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h> //Included SoftwareSerial Library

//Debug Option
//#define DEBUG

const char *mqtt_server = "192.168.1.79";

// Wifi Setting
#ifndef STASSID
#define STASSID "Livebox-19E2"
#define STAPSK "hcTzdCTZAqgVLxmKbM"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

// Serial with Arduino
int pinSoftRX = D6; // D6
int pinSoftTX = D5; // D5

//Function
void blink(int nb, int wait);
void reconnect();
void setup_serial();
void setup_wifi();
void setup_mqtt();
String readSerial();

// Use WiFiClient class to create TCP connections
WiFiClient espClient;
PubSubClient mqttClient(espClient);
SoftwareSerial arvSerial(pinSoftRX, pinSoftTX);

// MQTT Topics
String mainTopic = "proto1";
String cmndTopic = "cmnd";
String topic = cmndTopic + "/" + mainTopic + "/move";

// /------------------------------------------------------------------------------/
void setup()
{

    setup_serial();

    setup_wifi();

    setup_mqtt();

    digitalWrite(LED_BUILTIN, HIGH);
}


// /------------------------------------------------------------------------------/
void loop()
{
    //     // if ((millis() - lastTime) > timerDelay)
    //     // {
    //     // Read value of command
    String data = readSerial();

    if (data != "")
    {

        mqttClient.publish(topic.c_str(), data.c_str());
#ifdef DEBUG
        Serial.println(data);
#endif
        digitalWrite(LED_BUILTIN, LOW);
    }

    //   lastTime = millis();
    // }
}

// /------------------------------------------------------------------------------/
// /---- SUPPORT METHODES --------------------------------------------------------/
// /------------------------------------------------------------------------------/

// /Read serial communication with Arduino ---------------------------------------/
String readSerial()
{
    char chr;
    String data = "";

    do
    {
        if (arvSerial.available())
        {
            digitalWrite(LED_BUILTIN, HIGH);

            chr = arvSerial.read();
            if (chr != '#')
                data.concat(chr);
        }
    } while (chr != '#');

    arvSerial.flush();
    return data;
}

// /Wifi Setup -------------------------------------------------------------------/
void setup_wifi()
{
    blink(1, 200);

    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    // We start by connecting to a WiFi network
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.println();
    Serial.println();
    Serial.print("Wait for WiFi... ");

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        blink(1, 200);
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

// /Serial Setup -------------------------------------------------------------------/
void setup_serial()
{
    pinMode(LED_BUILTIN, OUTPUT); // Initialize the BUILTIN_LED pin as an output
    digitalWrite(LED_BUILTIN, LOW);

    blink(5, 50);

    arvSerial.begin(9600);

    //Serial for debug
    Serial.begin(115200);
    Serial.println("Démarrage de l'ESP");
}

// /MQTT Setup -------------------------------------------------------------------/
void setup_mqtt()
{
    mqttClient.setServer(mqtt_server, 1883);

    if (!mqttClient.connected())
        reconnect();

    // mqttClient.loop();
}

// /MQTT reconnect -------------------------------------------------------------------/
void reconnect()
{
    // Loop until we're reconnected
    while (!mqttClient.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (mqttClient.connect(clientId.c_str()))
        {
            Serial.println("connected");
            // Once connected, publish an announcement...
            mqttClient.publish("stat/remote/status", "connected");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 3 seconds");
            // Wait 5 seconds before retrying
            delay(3000);
        }
    }
}

void blink(int nb, int wait)
{
    for (byte i = 0; i < nb; i++)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(wait);
        digitalWrite(LED_BUILTIN, LOW);
        delay(wait);
    }
}
