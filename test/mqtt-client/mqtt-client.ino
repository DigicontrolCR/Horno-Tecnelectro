#include <WiFi.h>
#include <PubSubClient.h>

// Credenciales WiFi
const char* ssid = "DIGICONTROL";
const char* password = "7012digi19";

// Dirección del broker (IP de la computadora)
const char* mqtt_server = "192.168.100.137"; // la IP de la PC
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// Callback cuando llega un mensaje
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Mensaje recibido en tópico [");
  Serial.print(topic);
  Serial.print("]: ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conectar al broker MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado!");
      client.subscribe("esp32/out");
      client.subscribe("node/out");
    } else {
      Serial.print("Fallo, rc=");
      Serial.print(client.state());
      Serial.println(" reintentando en 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Publicar un mensaje cada 5 segundos en esp32/out
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 5000) {
    lastMsg = millis();
    String msg = "Hola desde ESP32!";
    client.publish("esp32/out", msg.c_str());
    Serial.println("Publicado en esp32/out: " + msg);
  }
}
