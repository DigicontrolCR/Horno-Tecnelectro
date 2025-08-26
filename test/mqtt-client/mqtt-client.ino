#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>

const char* ssid = "DIGICONTROL";
const char* password = "7012digi19";

#define MQTT_HOST "horno-tecnelectro.onrender.com"
#define MQTT_PORT 80   // Render expone WebSockets en puerto 80

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;

void connectToWifi() {
  Serial.println("Conectando a Wi-Fi...");
  WiFi.begin(ssid, password);
}

void connectToMqtt() {
  Serial.println("Conectando al broker MQTT por WebSockets...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
  switch(event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.println("WiFi conectado");
      connectToMqtt();
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("WiFi desconectado, reconectando...");
      wifiReconnectTimer.once(2, connectToWifi);
      break;
  }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("✅ Conectado al broker MQTT (WebSocket)!");
  mqttClient.subscribe("node/out", 1);
  mqttClient.publish("esp32/out", 1, false, "Hola desde ESP32!");
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties,
                   size_t len, size_t index, size_t total) {
  Serial.print("📩 Mensaje en [");
  Serial.print(topic);
  Serial.print("]: ");
  for (size_t i = 0; i < len; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Intentando conectar a Wi-Fi...");
}

void loop() {
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("Wi-Fi conectado!");
  } else {
    Serial.println("Conectando...");
  }
  delay(2000);
}
