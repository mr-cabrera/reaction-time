#include <Adafruit_NeoPixel.h>

// --- Configuración de la tira LED ---
const int pinR = 25;
const int pinG = 26;
const int pinB = 27;
#define LED_PIN     18
#define NUM_PIXELS  3
Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- Configuración del piezo ---
const int piezoPin = 34;  // Entrada del sensor piezo
int umbral;   // Nivel de detección del golpe

// --- Variables de tiempo ---
unsigned long tInicio = 0;
unsigned long tReaccion = 0;
bool esperandoGolpe = false;
int sumaLecturas;

void setup() {
  Serial.begin(115200);
  pinMode(pinR, OUTPUT);
  pinMode(pinG, OUTPUT);
  pinMode(pinB, OUTPUT);
  
  // Apagar todos al inicio
  digitalWrite(pinR, HIGH);
  digitalWrite(pinG, HIGH);
  digitalWrite(pinB, HIGH);
  delay(1000);
  digitalWrite(pinR, LOW);
  digitalWrite(pinG, HIGH);
  digitalWrite(pinB, HIGH);
  

  for (int i = 0; i < 100; i++) {
    int lectura = analogRead(piezoPin); // Lee el valor del pin analógico (0 a 1023)
    sumaLecturas = sumaLecturas + lectura; // Suma la lectura al acumulador
    delay(1); // Pequeña pausa opcional para asegurar la separación de muestras
  }
  Serial.println(sumaLecturas/100);
  umbral = (sumaLecturas/100)-700;
  delay(2000);

}

void loop() {
  esperandoGolpe = true;

  // Espera aleatoria entre 1 y 6 segundos
  int espera = random(1000, 6000);
  delay(espera);

  digitalWrite(pinR, HIGH); digitalWrite(pinG, LOW); digitalWrite(pinB, HIGH);

  tInicio = millis();
  

  // Espera hasta que se detecte un golpe
  while (esperandoGolpe) {
    int valor = analogRead(piezoPin);
    if (valor < umbral) {
      tReaccion = millis() - tInicio;

      // Azul -> golpe registrado
      digitalWrite(pinR, HIGH); digitalWrite(pinG, HIGH); digitalWrite(pinB, LOW);
      Serial.print("Golpe detectado! Tiempo de reacción: ");
      Serial.print(tReaccion); Serial.print(" ms - Umbral: "); Serial.println(valor);
      esperandoGolpe = false;
      
      // pausa antes de la siguiente ronda
      delay(2000);         
      digitalWrite(pinR, LOW);digitalWrite(pinG, HIGH);digitalWrite(pinB, HIGH);
      delay(5000);  
    }
  }
}

void setColor(int r, int g, int b) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}
