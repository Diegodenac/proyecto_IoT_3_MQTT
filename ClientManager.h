#include <WiFi.h>
#include <PubSubClient.h>
class ClientManager
{
private:
    const char* WIFI_SSID; 
    const char* WIFI_PASS; 
    const char* MQTT_BROKER; 
    int MQTT_PORT;
    const char* CLIENT_ID;
    PubSubClient client;
    WiFiClient wiFiClient;
    String servoComands[3] = {"OFF","L","0"};
    static ClientManager* instance;
public:
    ClientManager(const char* ssidWifi, const char* passWifi, const char* brokerDomain, int portBroker, const char* clientId) : client(wiFiClient){
      this->WIFI_SSID = ssidWifi;
      this->WIFI_PASS = passWifi;
      this->MQTT_BROKER = brokerDomain;
      this->MQTT_PORT = portBroker;
      this->CLIENT_ID = clientId;
      instance = this;
    };
    bool isVelocity(String message){
      int value = message.toInt();
      return message == "0" || (value > 0 && value <= 90);
    }

    bool isPowerComand(String message){
      return message == "ON" || message == "OFF";
    }

    bool isDirectionComand(String message){
      return message == "R" || message == "L";
    }
    String getMessage(byte* payload, unsigned int length){
        String message;
        for (int i = 0; i < length; i++) {
            message += (char)payload[i];
        }
        return message;
    }

    void handleCallback(char* topic, byte* payload, unsigned int length) {
        Serial.println("Message arrived on topic: " + String(topic));
        String message = getMessage(payload, length);
        
        // Lógica para modificar el array servoComands
        if (String(topic) == "home/move") {
            servoComands[0] = (isPowerComand(message) && servoComands[0] != message) ? message : servoComands[0];
            servoComands[1] = (isDirectionComand(message) && servoComands[1] != message) ? message : servoComands[1];
            servoComands[2] = isVelocity(message) ? message : servoComands[2];
        }

        Serial.println("Message is: " + message);
    }

    static void callback(char* topic, byte* payload, unsigned int length) {
        if (instance) {
            instance->handleCallback(topic, payload, length);  // Delegar a la instancia
        }
    }

    void setupWiFi() {
        delay(10);
        Serial.println();
        Serial.print("Connecting to ");
        Serial.println(WIFI_SSID);

        WiFi.begin(WIFI_SSID, WIFI_PASS);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.println();
        Serial.print("Connected to WiFi. IP address: ");
        Serial.println(WiFi.localIP());
    }

    void reconnect() {
        while (!client.connected()) {
            Serial.print("Attempting MQTT connection...");
            if (client.connect(CLIENT_ID)) {
            Serial.println("connected");
            client.subscribe("home/move");
            } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" trying again in 5 seconds");
            delay(5000);
            }
        }
    }

    bool isConeected(){
        return client.connected();
    }

    void listenMessages(){
        client.loop();
    }

    void setMqttConnection(){
        setupWiFi();
        client.setServer(MQTT_BROKER, MQTT_PORT);
        client.setCallback(ClientManager::callback);
    }
    int getValueToWrite(){
      int value;
      if (servoComands[0]=="ON"){
        if(servoComands[1]=="L"){
          value = servoComands[2].toInt()+95;
        }
        else{
          value = 90-servoComands[2].toInt();
        }
      }
      else
        value = 90;
      // Serial.println("Power received: " + servoComands[0]);
      // Serial.println("Direction received: " + servoComands[1]);
      // Serial.println("Velocity received: " + servoComands[2]);
      return value;
    }
    void sendMessage(const char* mqttTopic, char* message){
      client.publish(mqttTopic, message);
    }
};

ClientManager* ClientManager::instance = nullptr;