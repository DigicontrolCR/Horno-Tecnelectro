#include <WiFi.h>
#include <WebSocketsClient.h>

const char* ssid = "DIGICONTROL";
const char* password = "7012digi19";
const char* host = "horno-tecnelectro.onrender.com";
const int port = 80;

WebSocketsClient webSocket;

void connectToWiFi() {
  Serial.print("Conectando a WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi conectado");
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("❌ Desconectado del broker");
      break;
    case WStype_CONNECTED:
      Serial.println("✅ Conectado al broker WebSocket!");
      break;
    case WStype_TEXT:
      Serial.print("📩 Respuesta del broker: ");
      for(size_t i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
      }
      Serial.println();
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("🚀 Iniciando ESP32 WebSocket Client");
  connectToWiFi();

  // Usar el path /simple para el WebSocket simple
  webSocket.begin(host, port, "/simple");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(3000);
}

void loop() {
  webSocket.loop();
  
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 5000) {
    lastMsg = millis();
    
    if (webSocket.isConnected()) {
      // Enviar mensaje simple (ahora funcionará)
      String message = "ESP32_" + String(millis() / 1000);
      webSocket.sendTXT(message);
      Serial.println("📤 Enviado: " + message);
    } else {
      Serial.println("⚠️  Esperando conexión...");
    }
  }
  
  delay(100);
}
