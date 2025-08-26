#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// ===== CONFIGURACIÓN =====
const char* ssid = "DIGICONTROL";
const char* password = "7012digi19";
const char* serverURL = "https://horno-tecnelectro.onrender.com/api/message";

unsigned long lastSendTime = 0;
int messageCount = 0;

// ===== FUNCIONES =====
void connectToWiFi() {
  Serial.print("Conectando a WiFi");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi conectado");
    Serial.print("📶 RSSI: ");
    Serial.println(WiFi.RSSI());
    Serial.print("🌐 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Error: No se pudo conectar a WiFi");
  }
}

void sendMessageToBroker() {
  Serial.println("\n📤 Enviando mensaje al broker...");
  
  WiFiClientSecure client;
  HTTPClient http;
  
  // Configuración importante para SSL en ESP32
  client.setInsecure(); // Aceptar todos los certificados (necesario para Render)
  client.setTimeout(10000); // Timeout de 10 segundos
  
  // Iniciar conexión HTTPS
  if (http.begin(client, serverURL)) {
    http.addHeader("Content-Type", "application/json");
    http.addHeader("User-Agent", "ESP32-Client");
    
    // Crear mensaje JSON
    messageCount++;
    String messageText = "Mensaje #" + String(messageCount) + " desde ESP32 - " + String(millis() / 1000) + "s";
    String jsonPayload = "{\"topic\":\"esp32/out\",\"message\":\"" + messageText + "\"}";
    
    Serial.print("📦 JSON: ");
    Serial.println(jsonPayload);
    
    // Enviar solicitud POST
    int httpCode = http.POST(jsonPayload);
    
    Serial.print("📡 Respuesta HTTP: ");
    Serial.println(httpCode);
    
    if (httpCode > 0) {
      // Éxito - mostrar respuesta
      if (httpCode == 200) {
        String response = http.getString();
        Serial.print("✅ Éxito: ");
        Serial.println(response);
      } else {
        String response = http.getString();
        Serial.print("⚠️  Respuesta: ");
        Serial.println(response);
      }
    } else {
      // Error
      Serial.print("❌ Error HTTP: ");
      Serial.println(http.errorToString(httpCode));
    }
    
    // Cerrar conexión
    http.end();
    
  } else {
    Serial.println("❌ No se pudo conectar al servidor");
  }
  
  Serial.println("--------------------------------");
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(3000);
  
  Serial.println("\n🚀 ESP32 Client para Horno Tecnelectro");
  Serial.println("======================================");
  Serial.print("🔗 Broker: ");
  Serial.println(serverURL);
  
  connectToWiFi();
}

// ===== LOOP =====
void loop() {
  // Verificar conexión WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️  WiFi desconectado, reconectando...");
    connectToWiFi();
    delay(5000);
    return;
  }
  
  // Enviar mensaje cada 10 segundos
  if (millis() - lastSendTime > 10000) {
    lastSendTime = millis();
    sendMessageToBroker();
  }
  
  // Mostrar estado cada 5 segundos
  static unsigned long lastStatusTime = 0;
  if (millis() - lastStatusTime > 5000) {
    lastStatusTime = millis();
    
    Serial.print("💾 Memoria libre: ");
    Serial.print(ESP.getFreeHeap());
    Serial.print(" bytes | 📶 RSSI: ");
    Serial.println(WiFi.RSSI());
  }
  
  delay(1000);
}
