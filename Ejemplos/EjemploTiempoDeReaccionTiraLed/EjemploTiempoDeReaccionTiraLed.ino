#include <Adafruit_NeoPixel.h>

// --- Configuración de la tira LED ---
#define LED_PIN     18
#define NUM_PIXELS  3
Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- Configuración del piezo ---
const int piezoPin = 34;  // Entrada del sensor piezo
const int umbral = 400;   // Nivel de detección del golpe

// --- Variables de tiempo ---
unsigned long tInicio = 0;
unsigned long tReaccion = 0;
bool esperandoGolpe = false;

void setup() {
  Serial.begin(115200);
  pixels.begin();
  pixels.clear();
  pixels.show();

  // Rojo -> Tiempo de espera
  setColor(255, 0, 0);

  Serial.println("=== Medidor de tiempo de reacción ===");
  delay(2000);
}

void loop() {
  esperandoGolpe = true;

  // Espera aleatoria entre 1 y 6 segundos
  int espera = random(1000, 6000);
  delay(espera);

  
  setColor(0, 255, 0); // Verde → señal de inicio 
  tInicio = millis();
  

  // Espera hasta que se detecte un golpe
  while (esperandoGolpe) {
    int valor = analogRead(piezoPin);
    if (valor > umbral) {
      tReaccion = millis() - tInicio;

      // Azul -> golpe registrado
      setColor(0, 0, 255); 
      Serial.print("Golpe detectado! Tiempo de reacción: ");
      Serial.print(tReaccion); Serial.print(" ms - Umbral: "); Serial.println(valor);
      esperandoGolpe = false;
      
      // pausa antes de la siguiente ronda
      delay(2000);         
      setColor(255, 0, 0); // Rojo -> Tiempo de espera
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
