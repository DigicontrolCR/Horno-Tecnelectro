#include <WiFi.h>

// Colocamos las credenciales de la red WIFI
const char* ssid = "DIGICONTROL"; // NOMBRE
const char* password = "7012digi19"; // CONTRASEÑA

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Se ha conectado al wifi con la IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi Conectado");
  } else {
    Serial.println("WiFi Desconectado");
  }
  delay(1000); // Esperar un segundo antes de la próxima verificación
}

