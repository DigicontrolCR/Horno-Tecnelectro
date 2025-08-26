#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>

// ===== Configuración Wi-Fi =====
const char* ssid = "DIGICONTROL";
const char* password = "7012digi19";

// ===== Configuración MQTT =====
#define MQTT_HOST "horno-tecnelectro.onrender.com"
#define MQTT_PORT 80   // Render expone WebSockets en puerto 80

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;
Ticker publishTimer;  // Para enviar mensajes periódicos

// ===== Funciones =====
void connectToWifi() {
  Serial.println("Conectando a Wi-Fi...");
  WiFi.begin(ssid, password);
}

void connectToMqtt() {
  if (!mqttClient.connected()) {
    Serial.println("Intentando conectar al broker MQTT...");
    mqttClient.connect();
  }
}

// Callback para eventos Wi-Fi
void WiFiEvent(WiFiEvent_t event) {
  switch(event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.println("✅ Wi-Fi conectado");
      connectToMqtt();
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("⚠️ Wi-Fi desconectado, reconectando...");
      wifiReconnectTimer.once(2, connectToWifi);
      break;
  }
}

// Callback al conectar MQTT
void onMqttConnect(bool sessionPresent) {
  Serial.println("✅ Conectado al broker MQTT (WebSocket)!");
  
  // Suscribirse a ambos tópicos
  mqttClient.subscribe("node/out", 1);
  mqttClient.subscribe("esp32/out", 1);

  // Iniciar publicación periódica
  publishTimer.attach(5, []() {
    mqttClient.publish("esp32/out", 1, false, "Hola desde ESP32!");
    Serial.println("📤 Mensaje publicado en esp32/out");
  });
}

// Callback al recibir mensaje MQTT
void onMqttMessage(char* topic, char* payload,
                   AsyncMqttClientMessageProperties properties,
                   size_t len, size_t index, size_t total) {
  Serial.print("📩 Mensaje en [");
  Serial.print(topic);
  Serial.print("]: ");
  for (size_t i = 0; i < len; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.onEvent(WiFiEvent);              // Registrar eventos Wi-Fi
  mqttClient.onConnect(onMqttConnect);  // Registrar eventos MQTT
  mqttClient.onMessage(onMqttMessage);

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setClientId("ESP32Client");
  mqttClient.setWill("esp32/status", 1, true, "offline");  // Mensaje de último recurso
  mqttClient.setKeepAlive(15);

  connectToWifi();
}

// ===== Loop vacío =====
void loop() {
  // Nada que hacer aquí: AsyncMqttClient maneja todo con callbacks y timers
}
