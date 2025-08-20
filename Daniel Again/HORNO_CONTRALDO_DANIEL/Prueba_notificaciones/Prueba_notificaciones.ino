#include <WiFi.h>
#include <HTTPClient.h>

// Colocamos las credenciales de la red WIFI
const char* ssid = "DIGICONTROL"; // NOMBRE
const char* password = "7012digi19"; // CONTRASEÑA

// Colocar el token de meta
String token = "Bearer EAAF2Vuj8ao4BO0tdIcKx3fDKRy68vUtDFG47aoy9H2fEmNCMZAhoxFYqaLhJo4CF5neRzixMGc9PsNqlNibSZAArUCfA0lAVFZB40ubpYuMl9gIYZB1hwUoOEX88ssZCJNghxi5R2Ko9zsWyXJOIceloWEdCychfIHB0yCaKDHTuMJzEZAVkUQImIT1MAxHL0XILyUN6mdGgZBdvJN5eZBySQTP7WSWsFSwjRya9ZCQcZD";
String servidor = "https://graph.facebook.com/v18.0/295426096985808/messages";
String payload = "{\"messaging_product\":\"whatsapp\",\"to\":\"50689836964\",\"type\":\"text\",\"text\":{\"body\": \"ALARMA!! Fallo por variador de frecuencia\"}}";


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
    HTTPClient http;
    http.begin(servidor.c_str());
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", token);
    int httpPostCode = http.POST(payload);
    if (httpPostCode > 0) {
      int httpResponseCode = http.GET();
      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
    }
    http.end();
    delay(500); // Espera un momento después de enviar la notificación
  }
  else {
    Serial.println("WiFi Desconectado");
  }
  delay(100); // Agregar el punto y coma aquí
}
