#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

// ===== Configuración Wi-Fi =====
const char* ssid = "DIGICONTROL";
const char* password = "7012digi19";

// ===== Configuración WebSocket =====
const char* MQTT_WS_HOST = "horno-tecnelectro.onrender.com";
const uint16_t MQTT_WS_PORT = 80; // Puerto WS de Render
const char* MQTT_TOPIC_SUB1 = "node/out";
const char* MQTT_TOPIC_SUB2 = "esp32/out";
const char* MQTT_TOPIC_PUB = "esp32/out";

WebSocketsClient webSocket;
unsigned long lastPublish = 0;

// ===== Funciones =====
void connectToWiFi() {
  Serial.println("Conectando a Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Wi-Fi conectado");
}

// Callback al recibir mensajes del WebSocket
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("⚠️ WebSocket desconectado");
      break;
    case WStype_CONNECTED:
      Serial.println("✅ Conectado al broker WebSocket");
      // Suscribirse a tópicos
      webSocket.sendTXT("{\"type\":\"subscribe\",\"topic\":\"" + String(MQTT_TOPIC_SUB1) + "\"}");
      webSocket.sendTXT("{\"type\":\"subscribe\",\"topic\":\"" + String(MQTT_TOPIC_SUB2) + "\"}");
      break;
    case WStype_TEXT:
      Serial.print("📩 Mensaje recibido: ");
      Serial.write(payload, length);
      Serial.println();
      break;
    default:
      break;
  }
}

// Publicar mensaje en formato JSON
void publishMessage(const char* topic, const char* msg) {
  String json = "{\"type\":\"publish\",\"topic\":\"" + String(topic) + "\",\"payload\":\"" + String(msg) + "\"}";
  webSocket.sendTXT(json);
  Serial.print("📤 Publicado en ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(msg);
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  delay(1000);

  connectToWiFi();

  webSocket.begin(MQTT_WS_HOST, MQTT_WS_PORT, "/"); // Path por defecto "/"
  webSocket.onEvent(webSocketEvent);
}

// ===== Loop =====
void loop() {
  webSocket.loop();

  // Publicar cada 5 segundos
  if (millis() - lastPublish > 5000) {
    lastPublish = millis();
    publishMessage(MQTT_TOPIC_PUB, "Hola desde ESP32 WebSocket!");
  }
}
