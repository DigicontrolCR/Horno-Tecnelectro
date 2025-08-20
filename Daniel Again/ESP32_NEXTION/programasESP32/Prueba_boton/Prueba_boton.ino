#include <Nextion.h>

HardwareSerial nextionSerial(2);  // Configura UART2 en el ESP32

NexPage p0 = NexPage(0, 0, "page0");             // Página 0 en la pantalla Nextion
NexPicture lamPicture = NexPicture(0, 1, "p0");  // Imagen en la página 0 con ID 1
NexButton lamButton = NexButton(0, 6, "b0");     // Botón en la página 0 con ID 2

bool lamButtonStatus = true;

NexTouch *nex_listen_list[] = {
  &lamButton,
  NULL
};

// Callback para el evento de toque en el botón
void lamButtonPopCallback(void *ptr) {
  Serial.println("Presionado");
  lamButtonStatus = !lamButtonStatus;

  if (lamButtonStatus == true) {
    lamButton.setText("Apagar");
    lamPicture.setPic(1);  // Cambia la imagen
    Serial.println("presionado2");
  } else {
    lamButton.setText("Encender");
    lamPicture.setPic(0);  // Cambia la imagen
  }
}

void setup() {
  Serial.begin(9600);
  nextionSerial.begin(9600, SERIAL_8N1, 16, 17);  // Configura UART2 en pines 16 (RX) y 17 (TX)
  delay(500);

  nexInit();                                              // Inicializa la pantalla Nextion
  lamButton.attachPop(lamButtonPopCallback, &lamButton);  // Configura el callback para el botón
}

void loop() {
  nexLoop(nex_listen_list);
}
