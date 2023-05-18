/****************************************************************************
  Code base on "MQTT Exmple for SeeedStudio Wio Terminal".
  Author: Salman Faris
  Source: https://www.hackster.io/Salmanfarisvp/mqtt-on-wio-terminal-4ea8f8
*****************************************************************************/

#include <rpcWiFi.h>
#include "WiFi.h"
#include "TFT_eSPI.h"
#include <PubSubClient.h>
#include "DHT.h"
#include <string.h>
#include <ArduinoJson.h>


#define DHTTYPE DHT11
#define DHTPIN D3
DHT dht(DHTPIN, DHTTYPE); 

#define PLANT_ID 1


const char* ssid = SSID; // WiFi Name
const char* password = PASSWORD;  // WiFi Password
const char* mqtt_server = my_IPv4;

TFT_eSPI tft;
WiFiClient wioClient;
PubSubClient client(wioClient);
long lastMsg = 0;
char moistureMsg[50];
char brightnessMsg[50];
char humidityMsg[50];
char tempMsg[50];


int moisturePin = A0;
int brightnessPin = A1;
int ledPin = D1;
int speakerPin = D2;
int humidityTempPin = D3;
int moistureValue = 0;
int brightnessValue = 0;
int ledValue = 0;
int speakerValue = 0;
float humidityValue = 0;
float tempValue = 0;

#if defined(ARDUINO_ARCH_AVR)
    #define debug  Serial

#elif defined(ARDUINO_ARCH_SAMD) ||  defined(ARDUINO_ARCH_SAM)
    #define debug  SerialUSB
#else
    #define debug  Serial
#endif


void setup_wifi() {

  delay(10);

  tft.setTextSize(2);
  tft.setCursor((320 - tft.textWidth("Connecting to Wi-Fi..")) / 2, 120);
  tft.print("Connecting to Wi-Fi..");

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); // Connecting WiFi

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

  tft.fillScreen(TFT_BLACK);
  tft.setCursor((320 - tft.textWidth("Connected!")) / 2, 120);
  tft.print("Connected!");

  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); // Display Local IP Address

}

void callback(char* topic, byte* payload, unsigned int length) {
  //tft.fillScreen(TFT_BLACK);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  char buff_p[length];
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    buff_p[i] = (char)payload[i];
  }
  Serial.println();
  buff_p[length] = '\0';
  String msg_p = String(buff_p);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor((320 - tft.textWidth("MQTT Message")) / 2, 90);
  tft.print("MQTT Message: " );
  tft.setCursor((320 - tft.textWidth(msg_p)) / 2, 120);
  tft.print(msg_p); // Print recieved payload

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "WioTerminal-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("SensorData", "hello world");
      // ... and resubscribe
      client.subscribe("NotificationData");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {

  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(3);

  Wire.begin();
  pinMode(humidityTempPin, INPUT);
  digitalWrite(humidityTempPin, 1);
  dht.begin();

  Serial.println();
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883); // Connect the MQTT Server
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    moistureValue = analogRead(moisturePin);
    brightnessValue = analogRead(brightnessPin);
    humidityValue = dht.readHumidity();
    tempValue = dht.readTemperature();
    
    snprintf (moistureMsg, 50, "%ld", moistureValue);
    snprintf (brightnessMsg, 50, "%ld", brightnessValue);
    snprintf (humidityMsg, 50, "%.2f", humidityValue);
    snprintf (tempMsg, 50, "%.2f", tempValue);

    Serial.println(moistureMsg);
    Serial.println(brightnessMsg);
    Serial.println(humidityMsg);
    Serial.println(tempMsg);

    DynamicJsonDocument doc(1024);

    doc["moisture"] = moistureMsg;
    doc["brightness"]   = brightnessMsg;
    doc["humidity"] = humidityMsg;
    doc["temperature"] = tempMsg;

    char jsonString[1024];  // Define a char array to hold the serialized JSON

    size_t jsonSize = serializeJson(doc, jsonString, sizeof(jsonString));

    char topic[20]; 

    snprintf(topic, sizeof(topic), "timeseries/%d", PLANT_ID);
    client.publish(topic, jsonString);

    tft.setTextColor(TFT_BLACK);
    // if conditions are right
    tft.fillScreen(TFT_GREENYELLOW);
    tft.setCursor(10, 30);
    tft.print("EVERYTHING IS ALRIGHT! :)");
    // if conditions are wrong
    /*
    tft.fillScreen(TFT_RED);
    tft.setCursor(10, 30);
    tft.print("NOTIFS TO BE PRINTED HERE");
    */


    tft.fillRoundRect(5, 70, 150, 60, 10, TFT_BLACK);
    tft.fillRoundRect(5, 156, 150, 60, 10, TFT_BLACK);
    tft.fillRoundRect(165, 70, 150, 60, 10, TFT_BLACK);
    tft.fillRoundRect(165, 156, 150, 60, 10, TFT_BLACK);

    tft.setTextColor(TFT_WHITE);

    tft.setCursor(15, 78);
    tft.print("Moisture");
    tft.setCursor(175, 78);
    tft.print("Temperature");
    tft.setCursor(15, 106);
    tft.print(moistureMsg);
    tft.setCursor(175, 106);
    tft.print(tempMsg);
    tft.setCursor(15, 164);
    tft.print("Brightness");
    tft.setCursor(175, 164);
    tft.print("Humidity");
    tft.setCursor(15, 192);
    tft.print(brightnessMsg);
    tft.setCursor(175, 192);
    tft.print(humidityMsg);
  }
}
