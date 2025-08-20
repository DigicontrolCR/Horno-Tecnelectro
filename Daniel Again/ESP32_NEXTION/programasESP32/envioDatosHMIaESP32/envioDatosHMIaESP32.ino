#include <Nextion.h>

// Inicializar el puerto serial para la Nextion
HardwareSerial nextionSerial(2);  // Configura UART2 en el ESP32. Pines 16 y 17

// Definir un componente de texto o número (t0 para texto, n0 para número)
NexNumber n0 = NexNumber(0, 2, "n0");  // n0 es el componente de tipo número en la Nextion

// Variable para almacenar el valor numérico recibido
uint32_t receivedValue = 0;

void setup() {
  // Iniciar la comunicación con el monitor serial y el Nextion
  Serial.begin(9600);
  nextionSerial.begin(9600, SERIAL_8N1, 16, 17);  // Configura UART2 en pines 16 (RX) y 17 (TX)
  nexInit();
 
}

void loop() {
  if (n0.getValue(&receivedValue)) {  // Usar puntero (dirección de memoria) de receivedValue
    // Si se recibe un valor desde Nextion
    Serial.print("Valor numérico recibido: ");
    Serial.println(receivedValue);  // Imprimir el valor numérico en el monitor serial
  }
}
