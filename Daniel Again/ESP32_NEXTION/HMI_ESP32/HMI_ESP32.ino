#include <Nextion.h>

HardwareSerial nextionSerial(2);  // Configura UART2 para la pantalla Nextion

NexPage page0 = NexPage(0, 0, "page0");  // Página de inicio
NexPage page1 = NexPage(1, 0, "page1");  // Segunda página

NexPicture imgPage0 = NexPicture(0, 1, "p0");  // Imagen en page0 con id=1
NexPicture imgPage1 = NexPicture(1, 3, "p0");  // Imagen en page1 con id=3

NexTouch *nex_listen_list[] = {
  &imgPage0,
  &imgPage1,
  NULL
};

// Callback para tocar la imagen en page0 y cambiar a page1
void imgPage0TouchCallback(void *ptr) {
  Serial.println("Imagen en page0 tocada, cambiando a page1...");
  page1.show();  // Cambia a page1
}

// Callback para tocar la imagen en page1 y cambiar a page0
void imgPage1TouchCallback(void *ptr) {
  Serial.println("Imagen en page1 tocada, cambiando a page0...");
  page0.show();  // Cambia a page0
}

void setup() {
  Serial.begin(9600);  
  nextionSerial.begin(9600, SERIAL_8N1, 16, 17);  // UART2 en pines 16 (RX) y 17 (TX)
  delay(500);  // Espera para que la pantalla Nextion esté lista
  
  nexInit();  // Inicializa la pantalla Nextion

  // Configura los eventos táctiles para las imágenes en cada página
  imgPage0.attachPush(imgPage0TouchCallback, &imgPage0);
  imgPage1.attachPush(imgPage1TouchCallback, &imgPage1);
}

void loop() {
  nexLoop(nex_listen_list);  // Escucha eventos en la pantalla Nextion
}
