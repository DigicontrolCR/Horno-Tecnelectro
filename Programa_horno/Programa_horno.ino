#include "arduino_secrets.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include "Adafruit_MAX31855.h"
#include <Nextion.h>
#include <PID_v1.h>
#include "thingProperties.h"

// Definición de pines (ajusta según tu conexión)
#define MAXDO   19
#define MAXCLK  18
#define MAXCS   5

#define PIN_OUTPUT 25
#define pilotoStart 27
#define pilotoFalla 12
#define turbina 21

#define PIN_EMO 14
#define PIN_LSW 37
#define PIN_TEMPFALLA 38
#define PIN_START 13

uint32_t receivedValue = 0;  // Variable para almacenar el valor numérico recibido
double temperatura = 0;      //temperatura original
double temperatura2 = 0;
double temperatura3 = 0;  // para utilizarlo en el PID
double Setpoint, Input, Output;
double Kp = 2, Ki = 5, Kd = 1;  // parametros de ajustes iniciales


char tempHMI[6];


int estado_EMO = HIGH;
int estado_lsw = HIGH;
int estado_tempFalla = HIGH;
int estado_start = HIGH;


char corrienteHMI[6];
//unsigned long previousMillis = 0;  // Variable para almacenar el tiempo pasado
//unsigned long currentMillis = 0;
//const long tiempoTemp = 2000;       // Intervalo de tiempo para actualizar el dato de la temperatura en HMI(en milisegundos)
//unsigned long previousMillis1 = 0;  // Variable para almacenar el tiempo pasado
//unsigned long currentMillis1 = 0;
//const long tiempoTemp1 = 500;  // Intervalo de tiempo para actualizar el dato de la temperatura en HMI(en milisegundos)
int EMO = 0;
int estadoEMO = 0;
int estadolsw = 0;
int estadotempFalla = 0;
int bandera = 0;
int lsw = 0;
//int start = 0;
int tempFalla = 0;
int sensorCorriente = 0;
float corrienteActual = 0;
int val = 0;
const int pausePin = 36;        // Pin digital para pausar el conteo
unsigned long startTime = 0;    // Tiempo cuando se reanuda
unsigned long elapsedTime = 0;  // Tiempo acumulado total sin incluir pausas
unsigned long tiempoAnterior = 0;
//unsigned long interval = 60000;

String token = "Bearer EAAQUPPSZBwk0BOZCumOlSbitS9IyyuzEuIKCgD3iHMmeAsDBdmuz8oA6M3e5ZCD14MxtjlS6MKRqMTDROC0B7sCrUXhZBfGi1FmYPCn4wN0ztk91JDsWOkygL32FL40QQOU7v9yFIKq891EYBKIv0hEs6ZBXCeYZA9EIVYZCSgqZCCZAbtfZCViYOSKClJXUSLcoUYEbZBnfhGkk7ZCPC34fpfmM3rwJnlZBxfaQZD";
String servidor = "https://graph.facebook.com/v21.0/295426096985808/messages";
String payload = "{\"messaging_product\":\"whatsapp\",\"to\":\"50689836964\",\"type\":\"text\",\"text\":{\"body\": \"ALARMA!! Sobre temperatura\"}}";


// Configuración HMI Nextion
HardwareSerial nextionSerial(2);  // Usar UART2 en ESP32 (pines 16-TX, 17-RX)

// Intervalo entre lecturas (ms)
const unsigned long lecturaInterval = 1000;
const unsigned long lecturaEntradasInterval = 100;

// Variables para la HMI
//char tempHMI[10];  // Buffer para el texto de temperatura
//NexText NexTemperatura = NexText(0, 3, "t0");  // Asumiendo página 0, componente t0
//NexScrolltext g0 = NexScrolltext(0, 5, "g0");  // Para mensajes

// Crear objeto termopar
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

//Configuracion de cada atributo de la HMI
NexPage p0 = NexPage(0, 0, "page0");  //primero se indica la pagina, id, nombre del atributo
NexPage p1 = NexPage(1, 0, "page1");


NexText NexTemperatura = NexText(0, 3, "t0");


NexNumber n0 = NexNumber(0, 1, "n0");  // n0 es el componente donde se obtiene el set de temperatura
NexNumber n1 = NexNumber(0, 12, "n1");
NexNumber n2 = NexNumber(0, 13, "n2");


NexScrolltext g0 = NexScrolltext(0, 5, "g0");


NexText t4 = NexText(0, 7, "t4");
NexWaveform s0 = NexWaveform(1, 1, "s0");

NexTouch *nex_listen_list[] = {
  &n0,
  NULL
};


void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando configuración...");

  // 1. Hardware crítico
  analogReadResolution(12);
  nextionSerial.begin(9600, SERIAL_8N1, 16, 17);
  nexInit();
  
  // 2. Termopar
  if (!thermocouple.begin()) {
    Serial.println("ERROR Modulo Temperatura");
    mensajesHMI("ERROR! Modulo Temperatura");
  } else {
    Serial.println("Termopar OK");
  }

  // 3. Configuración de pines
  pinMode(pilotoStart, OUTPUT);
  pinMode(pilotoFalla, OUTPUT);
  pinMode(turbina, OUTPUT);
  pinMode(13, INPUT);
  pinMode(14, INPUT);
  pinMode(37, INPUT);
  pinMode(38, INPUT);
  
  pinMode(pausePin, INPUT);

  digitalWrite(pilotoFalla, HIGH);
  digitalWrite(pilotoStart, LOW);
  digitalWrite(turbina, LOW);

  // 4. PID
  Input = temperatura3;
  myPID.SetMode(AUTOMATIC);

  // 5. Conexión WiFi (con timeout)
  Serial.println("Conectando WiFi...");
  unsigned long wifiTimeout = 30000; // 30 segundos
  unsigned long startTime = millis();

  WiFi.begin(SECRET_SSID, SECRET_OPTIONAL_PASS);
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < wifiTimeout) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Fallo WiFi - Modo local");
  } else {
    Serial.println("WiFi conectado");
  }

  // 6. IoT Cloud (solo si WiFi está conectado)
  if (WiFi.status() == WL_CONNECTED) {
    if (!ArduinoCloud.begin(ArduinoIoTPreferredConnection)) {
      Serial.println("Fallo IoT Cloud");
    }
  }

  delay(500);
  Serial.println("Configuración completada");
}

void loop() {
  //ArduinoCloud.update();
  actualTemperatura();
  //Corriente();
  //acciones();
  //erroresTemperatura();
  lecturaEntradas();
  graficas();
  //onIndicationStarChange();
  encender();
  //notificaciones();
}

void actualTemperatura(){
  static unsigned long lastReadTime = 0;
  unsigned long currentTime = millis();
  
  // Leer cada segundo
  if (currentTime - lastReadTime >= lecturaInterval) {
    lastReadTime = currentTime;
    
    // Leer temperatura en Celsius
    double tempC = thermocouple.readCelsius();
    
    // Verificar si la lectura es válida
    if (isnan(tempC)) {
      Serial.println("Error en la lectura del termopar");
      mensajesHMI("Error termopar");
      verificarErrores();
    } else {
      // Mostrar temperatura en consola
      Serial.print("Temperatura: ");
      Serial.print(tempC);
      Serial.println(" °C");
      
      // Mostrar temperatura en HMI
      //mensajesHMI("Termopar OK");
      mostrarTemperaturaHMI(tempC);
    }
  }
}

// Función para mostrar temperatura en HMI
void mostrarTemperaturaHMI(double temperatura) {
  // Formatear el valor con 1 decimal
  String tempStr = String(temperatura, 1);
  tempStr.toCharArray(tempHMI, 10);
  
  // Enviar a componente de texto en HMI
  NexTemperatura.setText(tempHMI);
}

//Setear temperatura
void setTemperatura(void *ptr) {
  delay(6000);
  if (n0.getValue(&receivedValue)) {  // Usar puntero (dirección de memoria) de receivedValue
    //Setpoint = receivedValue;
    Setpoint = 100;
    mensajesHMI("Temperatura Seteada");
    Serial.print("Temperatura set: ");
    Serial.println(receivedValue);  // Imprimir el valor numérico en el monitor serial
  }
}

// Función para mostrar mensajes en HMI
void mensajesHMI(String mensaje) {
  char bufferMensaje[50];
  mensaje.toCharArray(bufferMensaje, 50);
  g0.setText(bufferMensaje);
}

// Función para verificar y mostrar errores específicos
void verificarErrores() {
  uint8_t error = thermocouple.readError();
  
  if (error) {
    Serial.print("Error en termopar (código: ");
    Serial.print(error);
    Serial.print("): ");
    
    if (error & MAX31855_FAULT_OPEN) {
      Serial.println("Termocupla abierta - Verifica la conexión");
      mensajesHMI("Error: Termocupla abierta");
    }
    if (error & MAX31855_FAULT_SHORT_GND) {
      Serial.println("Corto circuito a GND");
      mensajesHMI("Error: Corto a GND");
    }
    if (error & MAX31855_FAULT_SHORT_VCC) {
      Serial.println("Corto circuito a VCC");
      mensajesHMI("Error: Corto a VCC");
    }
  } else {
    Serial.println("Error desconocido");
    mensajesHMI("Error desconocido");
  }
}

//Toda la parte de sensado de la corriente
void Corriente() {
  //static unsigned long lastReadTime = 0;
  unsigned long currentTime = millis();

  //previousMillis1 = millis();

  val = 0;
  //while (millis() - previousMillis1 < 100) {
  while (millis() - currentTime < 100) {
    sensorCorriente = analogRead(26);
    // Actualizar el valor si es mayor
    if (sensorCorriente > val) {
      val = sensorCorriente;
    }
  }
  if (val != 0) {
    val = val + 800;  //factor multiplicador por consumo, 0.7v que consume el diodo del transistor
  } else {
    val = 0;
  }
  corrienteActual = val / (4095 / 8.3333);  //entre 8.333 porque tiene 6 vueltas el sensor por lo que la corriente max hay que dividirla entre 6, (50/6)
                                            //hay que convertir la corrienteActual en un arreglo. Esto porque en el atrubuto de numero solo se puede mostrar enteros, entonces para flotantes se utiliza el atributo de texto
  String corrienteActual1 = String(corrienteActual);
  corrienteActual1.toCharArray(corrienteHMI, 6);
  t4.setText(corrienteHMI);
  Serial.println(corrienteActual);
  Serial.println(val);
}

//Ejecuta las acciones de los componentes en HMI
void acciones() {
  nexLoop(nex_listen_list);
}

/*
void erroresTemperatura() {
  //detalle e impresion de errores del modulo sensor de temperatuta
  if (isnan(temperatura)) {
    Serial.println("Fallas detectadas en modulo temperatura!");
    uint8_t e = thermocouple.readError();
    if (e & MAX31855_FAULT_OPEN) Serial.println("FALLA: Termocupla abierta o no coenctada.");
    if (e & MAX31855_FAULT_SHORT_GND) Serial.println("FALLA: Termocupla en cocrto circuito a  GND.");
    if (e & MAX31855_FAULT_SHORT_VCC) Serial.println("FALLA: Termocupla en cocrto circuito a VCC.");
  }
}
*/

//Solamente lee el estado de las entrdas
//Solamente lee el estado de las entrdas
void lecturaEntradas() {
  static unsigned long lastReadTime = 0;
  unsigned long currentTime = millis();

  char mensaje[100];

  // Leer cada segundo
  if (currentTime - lastReadTime >= lecturaEntradasInterval) {
    lastReadTime = currentTime;

    // Leer entradas
    estado_EMO       = digitalRead(PIN_EMO);
    estado_lsw       = digitalRead(PIN_LSW);
    estado_tempFalla = digitalRead(PIN_TEMPFALLA);
    estado_start     = digitalRead(PIN_START);
    
    mensaje[0] = '\0';

    if (estado_EMO == HIGH || estado_lsw == HIGH || estado_tempFalla == HIGH) {
        //estado_start = LOW;

        if(estado_EMO == LOW){
          snprintf(mensaje, sizeof(mensaje),
             "(EMO) Paro de emergencia activado");
        }

        if(estado_lsw == LOW){
          snprintf(mensaje, sizeof(mensaje),
             "(LSW) Puerta abierta");
        }

        if(estado_tempFalla == LOW){
          snprintf(mensaje, sizeof(mensaje),
             "Fallo de temperatura");
        }

        // Mostrar en HMI
        mensajesHMI(mensaje);
    }
    
  }
}

//se grafican las variables en HMI
void graficas() {
  static unsigned long lastReadTime = 0;
  unsigned long currentTime = millis();
  
  // Leer cada segundo
  if (currentTime - lastReadTime >= lecturaInterval) {
    lastReadTime = currentTime;
    
    // Leer temperatura en Celsius
    double tempC = thermocouple.readCelsius();
    
    // Verificar si la lectura es válida
    if (isnan(tempC)) {
      Serial.println("Error en la lectura del termopar");
      //mensajesHMI("Error termopar");
      verificarErrores();
    } else {
      s0.addValue(0, tempC);  //el primer dato indica el canal, el componente permite 4 canales, pero en la programacion el canal 1 sería el 0 en el ESP32
  
    }

    //falta settear el valor de la variable corrienteActual
    s0.addValue(1, corrienteActual);

  }
}

//Se procesan las entradas
void onIndicationStarChange() {
  estadotempFalla = 0;
  mensajesHMI("");

  if (EMO == LOW) {
    estadotempFalla = 3;
    onIndicationFaultChange();
    return;
  }

  if (lsw == LOW) {
    estadotempFalla = 2;
    onIndicationFaultChange();
    return;
  }

  if (tempFalla == LOW) {
    estadotempFalla = 1;
    onIndicationFaultChange();
    return;
  }

  if (bandera == 1 && estadoEMO == 0 && estadolsw == 0 && estadotempFalla == 0) {
    indication_fault = false;
    PID();
    tiempo();
  } else if (stop == HIGH) {
    //  onStopChange();
  }
}

//Lógica del fallo de temperatura del controlador independiente
// Maneja las fallas
void onIndicationFaultChange() {
  /*
  indication_fault = true;
  indication_star = false;
  bandera = 0;
  
  
  switch (estadotempFalla) {
    case 1:  // Temperatura máxima
      mensajesHMI("Temperatura Máxima.");
      break;
    case 2:  // Puerta abierta
      mensajesHMI("Puerta abierta, CERRAR");
      break;
    case 3:  // EMO presionado
      mensajesHMI("EMO presionado, REINICIAR SISTEMA");
      break;
    default:
      mensajesHMI("");
      return;  // Salir si no hay falla reconocida
  }
  

  // Lógica común para todas las fallas
  digitalWrite(turbina, HIGH);
  digitalWrite(pilotoStart, HIGH);
  digitalWrite(pilotoFalla, LOW);
  delay(500);
  digitalWrite(pilotoFalla, HIGH);
  delay(500);
  */
}


void encender() {
  estado_start = digitalRead(PIN_START);
  if (estado_start == LOW) {
    Serial.println("START PRECIONADO");
  //if (start == LOW) {
    indication_star = true;
    digitalWrite(pilotoStart, LOW);
    digitalWrite(turbina, LOW);
    bandera = 1;
  } else if (estado_start == HIGH) {
    digitalWrite(pilotoStart, HIGH);
    digitalWrite(turbina, HIGH);
  }
   else if (stop == HIGH) {
    //  onStopChange();
  }
}

void notificaciones() {
double  temperatura4 = thermocouple.readCelsius();
  if (temperatura4 > 30) {
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
  delay(1000);
}

//lleva el control del tiempo en funcionamiento y logica de pausa si se va la luz.
void tiempo() {
  // Leer el estado del botón de pausa
  if (digitalRead(pausePin) == LOW) {
    delay(50);  // Debounce
    if (digitalRead(pausePin) == LOW) {
      startTime = elapsedTime;  // Guardar el tiempo actual al pausar
    }
    while (digitalRead(pausePin) == LOW) {
      delay(10);  // Esperar hasta que se suelte el botón
    }
    elapsedTime = startTime;
  }

  // Si no está en pausa, actualizar el tiempo acumulado
  if (millis() - tiempoAnterior > 1000) {
    elapsedTime = elapsedTime + 1;  // Incrementar en 1 segundo
    tiempoAnterior = millis();

    // Convertir el tiempo acumulado a horas, minutos y segundos
    unsigned long totalSeconds = elapsedTime;
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    //Mostrar numeros en HMI
    n1.setValue(hours);
    n2.setValue(minutes);
    //Mostrar el tiempo acumulado en formato hh : mm : ss
    Serial.print("Tiempo acumulado: ");
    if (hours < 10) Serial.print("0");
    // Serial.print(hours);
    //Serial.print(":");
    if (minutes < 10) Serial.print("0");
    Serial.print(minutes);
    Serial.print(":");
    if (seconds < 10) Serial.print("0");
    Serial.println(seconds);
  }
}

//PID del sistema de temperatura
void PID() {
  Setpoint = 100;
  temperatura2 = int(Setpoint);
  double temperatura3 = temperatura2;
  Serial.print("Temperatura para PID: ");
  Serial.println(temperatura2);
  Input = temperatura;
  myPID.Compute();
  analogWrite(PIN_OUTPUT, Output);
}

/*
void onStartChange() {
  indication_star = true;
  digitalWrite(pilotoStart, LOW);
  digitalWrite(turbina, LOW);
  bandera = 1;
}

void onStopChange() {
  digitalWrite(pilotoStart, HIGH);
  digitalWrite(turbina, HIGH);
  analogWrite(PIN_OUTPUT, 0);
  indication_star = false;
  bandera = 0;
}
*/
