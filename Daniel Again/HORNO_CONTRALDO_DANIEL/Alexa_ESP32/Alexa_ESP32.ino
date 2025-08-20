#include <Arduino.h>
#ifdef ESP32
    #include <WiFi.h>
#else
    #include <ESP8266WiFi.h>
#endif
#include "fauxmoESP.h"

// Colocamos las credenciales de la red WIFI
const char* ssid = "DIGICONTROL"; // NOMBRE
const char* password = "7012digi19"; // CONTRASEÑA

fauxmoESP fauxmo;

// Definir el LED y el dispositivo
#define LED_PIN             2
#define DEVICE_NAME         "led rojo"

void setup() {
    // Inicializar el puerto serie
    Serial.begin(9600);
    Serial.println();

    // Configurar el pin del LED como salida
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Configurar WiFi
    WiFi.mode(WIFI_STA);
    Serial.printf("[WIFI] Connecting to %s ", ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println();
    Serial.printf("[WIFI] Connected, IP address: %s\n", WiFi.localIP().toString().c_str());

    // Configurar fauxmoESP
    fauxmo.createServer(true);
    fauxmo.setPort(80);
    fauxmo.enable(true);

    // Añadir el dispositivo virtual
    fauxmo.addDevice(DEVICE_NAME);

    // Configurar el callback para el cambio de estado del dispositivo
    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
        if (strcmp(device_name, DEVICE_NAME) == 0) {
            digitalWrite(LED_PIN, state ? HIGH : LOW);
        }
    });
}

void loop() {
    fauxmo.handle();

    // Mostrar la memoria libre cada 5 segundos (opcional)
    static unsigned long last = millis();
    if (millis() - last > 5000) {
        last = millis();
        Serial.printf("[MAIN] Free heap: %d bytes\n", ESP.getFreeHeap());
    }

    // Verificar el estado de la conexión WiFi
    static wl_status_t lastStatus = WL_IDLE_STATUS;
    wl_status_t currentStatus = WiFi.status();
    if (currentStatus != lastStatus) {
        lastStatus = currentStatus;
        if (currentStatus == WL_CONNECTED) {
            Serial.println("[WIFI] Connected");
        } else if (currentStatus == WL_DISCONNECTED) {
            Serial.println("[WIFI] Disconnected, attempting reconnection...");
            WiFi.begin(ssid, password);
        } else {
            Serial.println("[WIFI] Connecting...");
        }
    }
}

