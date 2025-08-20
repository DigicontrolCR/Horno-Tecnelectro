

/***************************************************
  This is an example for the Adafruit Thermocouple Sensor w/MAX31855K

  Designed specifically to work with the Adafruit Thermocouple Sensor
  ----> https://www.adafruit.com/products/269

  These displays use SPI to communicate, 3 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/
#include <Nextion.h>
#include <SPI.h>
#include "Adafruit_MAX31855.h"


#define MAXDO 19
#define MAXCS 5
#define MAXCLK 18

HardwareSerial nextionSerial(2);  // Configura UART2 en el ESP32. Pines 16 y 17
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

//Configuracion de cada atributo de la HMI
NexPage p0 = NexPage(0, 0, "page0");  //primero se indica la pagina, id, nombre del atributo
NexText NexTemperatura = NexText(0, 1, "t0");

void setup() {
  Serial.begin(9600);
  nextionSerial.begin(9600, SERIAL_8N1, 16, 17);  // Configura UART2 en pines 16 (RX) y 17 (TX)
  nexInit();

  while (!Serial) delay(1);  // wait for Serial on Leonardo/Zero, etc
  Serial.println("MAX31855 test");
  // wait for MAX chip to stabilize
  delay(500);
  Serial.print("Initializing sensor...");

  //Si el modulo al iniciar esta en fallo imprime el error, de lo contrario imprime "Done"
  if (!thermocouple.begin()) {
    Serial.println("ERROR.");
    while (1) delay(10);
  }
  Serial.println("DONE.");
}

void loop() {
  double temperatura = thermocouple.readCelsius();  //valor de la temperatura

  //hay que convertir la temperatura en un arreglo. Esto porque en el atrubuto de numero solo se puede mostrar enteros, entonces para flotantes se utiliza el atributo de texto
  char tempHMI[6];
  String TempString = String(temperatura);
  TempString.toCharArray(tempHMI, 6);
  NexTemperatura.setText(tempHMI);

  //detalle e impresion de errores del modulo sensor de temperatuta
  if (isnan(temperatura)) {
    Serial.println("Thermocouple fault(s) detected!");
    uint8_t e = thermocouple.readError();
    if (e & MAX31855_FAULT_OPEN) Serial.println("FAULT: Thermocouple is open - no connections.");
    if (e & MAX31855_FAULT_SHORT_GND) Serial.println("FAULT: Thermocouple is short-circuited to GND.");
    if (e & MAX31855_FAULT_SHORT_VCC) Serial.println("FAULT: Thermocouple is short-circuited to VCC.");
  } else {
    Serial.println(temperatura);
  }

  delay(1000);
}