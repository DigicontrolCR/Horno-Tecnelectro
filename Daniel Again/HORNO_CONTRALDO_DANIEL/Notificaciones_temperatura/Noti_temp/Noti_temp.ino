#include <WiFi.h>
#include <HTTPClient.h>
#include "Adafruit_MAX31855.h"
#include <SPI.h>
// Configuración de red WiFi
const char* ssid = "DIGICONTROL";
const char* password = "7012digi19";
String token = "Bearer EAAF2Vuj8ao4BO0tdIcKx3fDKRy68vUtDFG47aoy9H2fEmNCMZAhoxFYqaLhJo4CF5neRzixMGc9PsNqlNibSZAArUCfA0lAVFZB40ubpYuMl9gIYZB1hwUoOEX88ssZCJNghxi5R2Ko9zsWyXJOIceloWEdCychfIHB0yCaKDHTuMJzEZAVkUQImIT1MAxHL0XILyUN6mdGgZBdvJN5eZBySQTP7WSWsFSwjRya9ZCQcZD";
String servidor = "https://graph.facebook.com/v18.0/295426096985808/messages";
String payload = "{\"messaging_product\":\"whatsapp\",\"to\":\"50689836964\",\"type\":\"text\",\"text\":{\"body\": \"TEMPERATURA ALTA\"}}";

// Pines utilizados

#define MAXDO 19
#define MAXCS 5
#define MAXCLK 18
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);


void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado al WiFi. Dirección IP: ");
  Serial.println(WiFi.localIP());

  if (!thermocouple.begin()) {
    Serial.println("Error al inicializar.");
    while (1)
      ;
  }
}

void loop() {
  double c = thermocouple.readCelsius();
  Serial.println(c);
  if (c > 40) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(servidor.c_str());
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Authorization", token);
      int httpPostCode = http.POST(payload);
      if (httpPostCode > 0) {
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
          String payload = http.getString();
        } else {
          Serial.print("Código de error HTTP: ");
          Serial.println(httpResponseCode);
        }
      }
      http.end();
    } else {
      Serial.println("WiFi desconectado.");
    }
  }

  delay(500);
}
