//Se incluyen las librerias
#include <Nextion.h>
#include <SPI.h>
#include "Adafruit_MAX31855.h"

//Declaracion y definicion de variables
#define MAXDO 19
#define MAXCS 5
#define MAXCLK 18


HardwareSerial nextionSerial(2);  // Configura UART2 en el ESP32. Pines 16 y 17

NexPage p0 = NexPage(0, 0, "page0");  //primero se indica la pagina, id, nombre del atributo
NexScrolltext g0 = NexScrolltext(0, 6, "g0");

void setup() {
  Serial.begin(9600);                             //Se inicia comunicacion del ESP32
  nextionSerial.begin(9600, SERIAL_8N1, 16, 17);  // Se inicia configuracion de comunicacion con HMI
  nexInit();
}

void loop() {
  char mensajes[30]="MENSAJE";
  g0.setText(mensajes);
}