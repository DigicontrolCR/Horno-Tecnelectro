//Se incluyen las librerias
#include <Nextion.h>
#include <SPI.h>
#include "Adafruit_MAX31855.h"
#include <PID_v1.h>

//Declaracion y definicion de variables
#define MAXDO 19
#define MAXCS 5
#define MAXCLK 18
#define PIN_OUTPUT 25
#define pilotoStart 27
#define pilotoFalla 12
#define turbina 21
uint32_t receivedValue = 0;  // Variable para almacenar el valor numérico recibido
double temperatura = 0;      //temperatura original
double temperatura2 = 0;
double temperatura3 = 0;  // para utilizarlo en el PID
float temperatura4 = 0;   // para visualizacion en la nube
double Setpoint, Input, Output;
double Kp = 2, Ki = 5, Kd = 1;  // parametros de ajustes iniciales
char tempHMI[6];
char corrienteHMI[6];
unsigned long previousMillis = 0;  // Variable para almacenar el tiempo pasado
unsigned long currentMillis = 0;
const long tiempoTemp = 2000;       // Intervalo de tiempo para actualizar el dato de la temperatura en HMI(en milisegundos)
unsigned long previousMillis1 = 0;  // Variable para almacenar el tiempo pasado
unsigned long currentMillis1 = 0;
const long tiempoTemp1 = 500;  // Intervalo de tiempo para actualizar el dato de la temperatura en HMI(en milisegundos)
int EMO = 0;
int estadoEMO = 0;
int estadolsw = 0;
int estadotempFalla = 0;
int lsw = 0;
int start = 0;
int tempFalla = 0;
int sensorCorriente = 0;
float corrienteActual = 0;
int val = 0;
const int pausePin = 36;        // Pin digital para pausar el conteo
unsigned long startTime = 0;    // Tiempo cuando se reanuda
unsigned long elapsedTime = 0;  // Tiempo acumulado total sin incluir pausas
unsigned long tiempoAnterior = 0;

HardwareSerial nextionSerial(2);  // Configura UART2 en el ESP32. Pines 16 y 17

//Especificacion de enlaces
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
  Serial.begin(9600);                             //Se inicia comunicacion del ESP32
  analogReadResolution(12);                       // Configurar resolución del ADC a 12 bits
  nextionSerial.begin(9600, SERIAL_8N1, 16, 17);  // Se inicia configuracion de comunicacion con HMI
  nexInit();                                      // Se inicia la HMI

  Input = temperatura3;
  myPID.SetMode(AUTOMATIC);

  while (!Serial) delay(1);  // Espera la comunicacion del modulo de temperatura
  Serial.println("MAX31855 test");
  mensajes("MAX31855 test");
  delay(5000);

  // Espera que el MAX se estabilice
 // Serial.print("Inicializando sensor...");
  mensajes("Inicializando sensor...");
  delay(5000);

  //Si el modulo al iniciar esta en fallo imprime el error, de lo contrario imprime "Done"
  if (!thermocouple.begin()) {
    Serial.println("ERROR Modulo Temperatura");
    mensajes("ERROR! Modulo Temperatura");
    while (1) delay(10);
  }
  Serial.println("DONE! Modulo Temperatura.");
  mensajes("DONE! Modulo Temperatura");
  delay(5000);
  mensajes("Sin ERRORES");

  //Configura el llamado de las acciones de los diferentes atributos de la HMI
  n0.attachPop(setTemperatura, &n0);  //accion al presionar el atributo para setear temperatura


  pinMode(pilotoStart, OUTPUT);
  pinMode(pilotoFalla, OUTPUT);
  pinMode(turbina, OUTPUT);
  pinMode(13, INPUT);
  pinMode(14, INPUT);
  pinMode(37, INPUT);
  pinMode(38, INPUT);
  digitalWrite(pilotoFalla, HIGH);
  pinMode(pausePin, INPUT);  // Configurar el pin como entrada
}

void loop() {
  actualTemperatura();
  Corriente();
  acciones();
  erroresTemperatura();
  lecturaEntradas();
  graficas();
  inicio();
}

//Para desplegar mensajes en HMI
void mensajes(String mensaje) {
  char mensajesText[30];                  // Crea un arreglo de caracteres para almacenar el mensaje
  mensaje.toCharArray(mensajesText, 30);  // Convierte el String a un arreglo de caracteres
  g0.setText(mensajesText);               // Muestra el mensaje en el HMI
}

//Ejecuta las acciones de los componentes en HMI
void acciones() {
  nexLoop(nex_listen_list);
}

//Errores en módulo de temperatura
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


//Muestra la temperatura actual
void actualTemperatura() {
  currentMillis = millis();
  temperatura = thermocouple.readCelsius();  //valor de la temperatura
  if (currentMillis - previousMillis >= tiempoTemp) {
    previousMillis = currentMillis;  // Guardar el tiempo actual
    //hay que convertir la temperatura en un arreglo. Esto porque en el atrubuto de numero solo se puede mostrar enteros, entonces para flotantes se utiliza el atributo de texto
    String TempString = String(temperatura);
    temperatura4 = temperatura;
    TempString.toCharArray(tempHMI, 6);
    NexTemperatura.setText(tempHMI);
    Serial.print("Temperatura actual: ");
    Serial.println(temperatura);
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

//Setear temperatura
void setTemperatura(void *ptr) {
  delay(6000);
  if (n0.getValue(&receivedValue)) {  // Usar puntero (dirección de memoria) de receivedValue
    //Setpoint = receivedValue;
    Setpoint = 100;
    mensajes("Temperatura Seteada");
    Serial.print("Temperatura set: ");
    Serial.println(receivedValue);  // Imprimir el valor numérico en el monitor serial
  }
}

//Se procesan las entradas
void inicio() {
  if (EMO == LOW) {
    EMO1();
  } else {
    estadoEMO = 0;
    mensajes("");
  }

  if (lsw == LOW) {
    lsw1();
  } else {
    estadolsw = 0;
    mensajes("");
  }

  if (tempFalla == LOW) {
    tempFalla1();
  } else {
    estadotempFalla = 0;
    mensajes("");
  }

  if (start == LOW && estadoEMO == 0 && estadolsw == 0 && estadotempFalla == 0) {
    digitalWrite(pilotoStart, LOW);
    PID();
    tiempo();
    digitalWrite(turbina, LOW);
  } else {
    digitalWrite(pilotoStart, HIGH);
    digitalWrite(turbina, HIGH);
     analogWrite(PIN_OUTPUT, 0);
  }
}


//Solamente lee el estado de las entrdas
void lecturaEntradas() {
  start = digitalRead(13);
  EMO = digitalRead(14);
  lsw = digitalRead(37);
  tempFalla = digitalRead(38);
}

//Configuracion del EMO
void EMO1() {

  estadoEMO = 1;
  mensajes("EMO presionado, REINICIAR SISTEMA");
  //PONER CONDICION DE APAGAR RESISTENCIA
  digitalWrite(turbina, HIGH);
  digitalWrite(pilotoStart, HIGH);
  digitalWrite(pilotoFalla, LOW);
  delay(500);
  digitalWrite(pilotoFalla, HIGH);
  delay(500);
}

//Lógica del limit switch de la puerta
void lsw1() {
  estadolsw = 1;
  mensajes("Puerta abierta, CERRAR");
  //PONER CONDICION DE APAGAR RESISTENCIA
  digitalWrite(turbina, HIGH);
  digitalWrite(pilotoStart, HIGH);
  digitalWrite(pilotoFalla, LOW);
  delay(500);
  digitalWrite(pilotoFalla, HIGH);
  delay(500);
}

//Lógica del fallo de temperatura del controlador independiente
void tempFalla1() {
  estadotempFalla = 1;
  mensajes("Temperatura Máxima.");
  //PONER CONDICION DE APAGAR RESISTENCIA
  digitalWrite(turbina, HIGH);
  digitalWrite(pilotoStart, HIGH);
  digitalWrite(pilotoFalla, LOW);
  delay(500);
  digitalWrite(pilotoFalla, HIGH);
  delay(500);
}

//Toda la parte de sensado de la corriente
void Corriente() {
  previousMillis1 = millis();
  val = 0;
  while (millis() - previousMillis1 < 100) {
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

//se grafican las variables en HMI
void graficas() {
  s0.addValue(0, temperatura);  //el primer dato indica el canal, el componente permite 4 canales, pero en la programacion el canal 1 sería el 0 en el ESP32
  s0.addValue(1, corrienteActual);
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
    // Mostrar el tiempo acumulado en formato hh:mm:ss
    //Serial.print("Tiempo acumulado: ");
    if (hours < 10) Serial.print("0");
   // Serial.print(hours);
    //Serial.print(":");
    if (minutes < 10) Serial.print("0");
    //Serial.print(minutes);
   // Serial.print(":");
    if (seconds < 10) Serial.print("0");
   // Serial.println(seconds);
  }
}