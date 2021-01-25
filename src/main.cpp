

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <ArduinoOTA.h>

#include <math.h>

#include "secrets.h"

#include <Wire.h>
#include <BH1750.h>

#include <PubSubClient.h>




BH1750 lightMeterA;

const char* mqtt_server = "192.168.1.110";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_WORDS	(100)
#define MSG_BUFFER_SIZE	(MSG_BUFFER_WORDS * sizeof(uint16_t))
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
 
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
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
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //client.setSocketTimeout(0xFFFF);
  client.setKeepAlive(0xFFFF);
  
  
  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  Wire.begin(D1, D2); /* join i2c bus with SDA=D1 and SCL=D2 of NodeMCU */
  lightMeterA.begin(BH1750::CONTINUOUS_LOW_RES_MODE);
}

void loop() {

  if (!client.connected()) {
    reconnect();
    value = 0;
  }
  client.loop();

  bool waited = false;
  while (!lightMeterA.measurementReady(false)) {
    waited = true;
  }
  if (!waited) {
    Serial.println("!");
  }

  // if (!lightMeterA.measurementReady(false)) {
  //   return;
  // }

  if (value == 0) {
    //Serial.println("b");
    if (!client.beginPublish("outTopic", MSG_BUFFER_SIZE, false)) {
      Serial.println("1");
      return;
    }
  }

  uint16_t val = lightMeterA.readLightLevel();
  if (client.write((uint8_t*)&val, sizeof(val))!=2) {
    Serial.println("-");
  }
  value++;

  if (value == MSG_BUFFER_WORDS) {
    //Serial.println("e");
    if (client.endPublish()==0) {
      Serial.println("2");
    }
    value = 0;
  }

}
