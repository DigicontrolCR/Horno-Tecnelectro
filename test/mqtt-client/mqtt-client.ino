#include <WiFi.h>
#include <WebSocketsClient.h>

const char* ssid = "DIGICONTROL";
const char* password = "7012digi19";
const char* host = "horno-tecnelectro.onrender.com";
const int port = 80;

WebSocketsClient webSocket;

void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("✅ WiFi conectado");
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("❌ Desconectado");
      break;
    case WStype_CONNECTED:
      Serial.println("✅ Conectado!");
      break;
    case WStype_TEXT:
      Serial.print("📩: ");
      Serial.write(payload, length);
      Serial.println();
      break;
  }
}

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  
  // Usar path /simple para el WebSocket simple
  webSocket.begin(host, port, "/simple");
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
  
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 5000) {
    lastMsg = millis();
    
    // Enviar mensaje simple (ahora funcionará)
    String message = "ESP32_" + String(millis() / 1000);
    webSocket.sendTXT(message);
    Serial.println("📤 Enviado: " + message);
  }
  
  delay(100);
}
