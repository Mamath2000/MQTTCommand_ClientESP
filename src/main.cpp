#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h> //Included SoftwareSerial Library

#define DEBUG ;

const char *mqtt_server = "192.168.1.79";

// unsigned long lastMsg = 0;
// #define MSG_BUFFER_SIZE (50)
// char msg[MSG_BUFFER_SIZE];
// int value = 0;

#ifndef STASSID
#define STASSID "Livebox-19E2"
#define STAPSK "hcTzdCTZAqgVLxmKbM"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

// unsigned long lastTime = 0;
// unsigned long timerDelay = 100; // send readings timer

int pinSoftRX = D6; // D6
int pinSoftTX = D5; // D5

void blink(int nb, int wait);
void reconnect();

// Use WiFiClient class to create TCP connections
WiFiClient espClient;
PubSubClient mqttClient(espClient);
SoftwareSerial arvSerial(pinSoftRX, pinSoftTX);

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

void setup_serial()
{
    pinMode(LED_BUILTIN, OUTPUT); // Initialize the BUILTIN_LED pin as an output
    digitalWrite(LED_BUILTIN, LOW);

    blink(5, 50);

    arvSerial.begin(9600);

#ifdef DEBUG
    //Serial for debug
    Serial.begin(115200);
    Serial.println("Démarrage de l'ESP");
#endif
}

void setup_mqtt()
{
    mqttClient.setServer(mqtt_server, 1883);

    if (!mqttClient.connected())
        reconnect();

    // mqttClient.loop();
}

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
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(3000);
        }
    }
}

void setup()
{

    setup_serial();

    setup_wifi();

    setup_mqtt();

    digitalWrite(LED_BUILTIN, HIGH);
}

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

String mainTopic = "proto1";
String cmndTopic = "cmnd";
String topic = cmndTopic + "/" + mainTopic + "/move";
String data;

void loop()
{
    //     // if ((millis() - lastTime) > timerDelay)
    //     // {
    //     // Read value of command
    String data = readSerial();

    if (data != "")
    {

        mqttClient.publish("cmnd/remote/value", data.c_str());
        Serial.println(data);
        digitalWrite(LED_BUILTIN, LOW);
    }

    //   lastTime = millis();
    // }
}

// /------------------------------------------------------------------------------/
// /---- SUPPORT METHODES --------------------------------------------------------/
// /------------------------------------------------------------------------------/
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
