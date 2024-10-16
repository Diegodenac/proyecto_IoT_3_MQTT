#include <ClientManager.h>
#include <ESP32Servo.h>
#include "DHT.h"

#define DHTPIN 26     
#define DHTTYPE DHT11 
#define PIN_SERVO 32

// Network Credentials
const char* ssidWifi = "Android4444";
const char* passWifi = "jdp269gaj";

//MQTT broker details
const char* brokerDomain = "broker.hivemq.com";
int portBroker = 1883;
const char* clientId = "0c8005e166414c44b01aef66e994d751";

ClientManager client(ssidWifi, passWifi, brokerDomain, portBroker, clientId);

Servo servo_32;

DHT dht(DHTPIN, DHTTYPE);
const char* mqtt_temp_topic = "temp/grupo4/mqtt";
const char* mqtt_hume_topic = "hume/grupo4/mqtt";

void setup() {
  Serial.begin(115200);
  servo_32.attach(PIN_SERVO);
  dht.begin();
  client.setMqttConnection();
}

void loop() {
  if (!client.isConeected()) {
    client.reconnect();
  }
  client.listenMessages();
  servo_32.write(client.getValueToWrite());
  delay(500);

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  char tempStr[8];
  char humeStr[8];
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Error reading sensor DHT11");
    return;
  }
  // Serial.print(humidity);
  // Serial.print(" %\t");
  // Serial.print(temperature);
  // Serial.println(" °C");
  dtostrf(temperature, 1, 2, tempStr);
  dtostrf(humidity, 1, 2, humeStr);
  client.sendMessage(mqtt_temp_topic, tempStr);
  client.sendMessage(mqtt_hume_topic, humeStr);
}