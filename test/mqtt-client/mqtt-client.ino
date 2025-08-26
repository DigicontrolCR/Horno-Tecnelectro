#include <WiFi.h>
#include <WebSocketsClient.h>

// ===== Configuración =====
const char* ssid = "DIGICONTROL";
const char* password = "7012digi19";
const char* WS_SERVER = "horno-tecnelectro.onrender.com";
const uint16_t WS_PORT = 80;

WebSocketsClient webSocket;
unsigned long lastSendTime = 0;

void connectToWiFi() {
  Serial.println("Conectando a WiFi...");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi conectado");
  }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("❌ Desconectado");
      break;
    case WStype_CONNECTED:
      Serial.println("✅ Conectado al broker!");
      break;
    case WStype_TEXT:
      Serial.print("📩 Recibido: ");
      Serial.write(payload, length);
      Serial.println();
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("Iniciando cliente MQTT WebSocket...");
  connectToWiFi();
  
  webSocket.begin(WS_SERVER, WS_PORT, "/");
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
  
  if (millis() - lastSendTime > 5000) {
    lastSendTime = millis();
    
    if (webSocket.isConnected()) {
      String msg = "ESP32 - " + String(millis());
      webSocket.sendTXT(msg);
      Serial.println("📤 Enviado: " + msg);
    } else {
      Serial.println("⚠️  No conectado, intentando reconectar...");
      webSocket.begin(WS_SERVER, WS_PORT, "/");
    }
  }
  
  delay(100);
}
