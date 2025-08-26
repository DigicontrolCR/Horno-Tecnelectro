#include <WiFi.h>
#include <PubSubClient.h>
#include <WebSocketsClient.h>

// ===== Configuración Wi-Fi =====
const char* ssid = "DIGICONTROL";
const char* password = "7012digi19";

// ===== Configuración MQTT WebSocket =====
const char* MQTT_SERVER = "horno-tecnelectro.onrender.com";
const int MQTT_PORT = 80;
const char* MQTT_CLIENT_ID = "ESP32_Client";

// Objetos globales
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
WebSocketsClient webSocket;

// ===== Variables de estado =====
bool mqttConnected = false;
unsigned long lastReconnectAttempt = 0;
unsigned long lastPublishTime = 0;

// ===== Funciones =====
void connectToWiFi() {
  Serial.println("Conectando a Wi-Fi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n✅ Wi-Fi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// Función para reconectar MQTT
bool reconnectMQTT() {
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("✅ Conectado al broker MQTT WebSocket!");
    
    // Suscribirse a tópicos
    mqttClient.subscribe("node/out");
    mqttClient.subscribe("esp32/out");
    Serial.println("📡 Suscrito a tópicos: node/out, esp32/out");
    
    return true;
  } else {
    Serial.print("❌ Falló conexión MQTT, rc=");
    Serial.print(mqttClient.state());
    Serial.println(" intentando de nuevo en 5 segundos");
    return false;
  }
}

// Callback para mensajes MQTT
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("📩 Mensaje recibido en [");
  Serial.print(topic);
  Serial.print("]: ");
  
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Función para publicar mensaje
void publishMessage() {
  String message = "Hola desde ESP32! Tiempo: " + String(millis() / 1000) + "s";
  
  if (mqttClient.publish("esp32/out", message.c_str())) {
    Serial.print("📤 Publicado: ");
    Serial.println(message);
  } else {
    Serial.println("❌ Error al publicar mensaje");
  }
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("🚀 Iniciando ESP32 MQTT WebSocket Client");

  // Conectar a WiFi
  connectToWiFi();

  // Configurar cliente MQTT
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(1024); // Buffer más grande para WebSocket

  // Configurar timeout de conexión
  mqttClient.setSocketTimeout(30);
}

// ===== Loop =====
void loop() {
  // Mantener conexión MQTT
  if (!mqttClient.connected()) {
    mqttConnected = false;
    unsigned long now = millis();
    
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      if (reconnectMQTT()) {
        mqttConnected = true;
        lastReconnectAttempt = 0;
      }
    }
  } else {
    mqttConnected = true;
    mqttClient.loop();
  }

  // Publicar mensaje cada 5 segundos si está conectado
  if (mqttConnected && millis() - lastPublishTime > 5000) {
    lastPublishTime = millis();
    publishMessage();
  }

  delay(100);
}
