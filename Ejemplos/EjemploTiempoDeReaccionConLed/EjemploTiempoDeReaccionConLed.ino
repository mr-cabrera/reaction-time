// Definiciones de pines
const int pulsadorPin = 25; // Pin de entrada para el pulsador
const int ledPin = 26;      // LED indicador

// Variables globales
unsigned long tInicio = 0;
bool esperandoGolpe = false; // Renombrada para más claridad

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  
  // --- CAMBIO PRINCIPAL ---
  // Configura el pin del pulsador con la resistencia PULL-UP interna.
  // - Cuando el pulsador NO está presionado, el pin lee HIGH (alto).
  // - Cuando el pulsador SÍ está presionado (conecta a GND), el pin lee LOW (bajo).
  pinMode(pulsadorPin, INPUT_PULLUP); 
  
  Serial.println("=== Medidor de tiempo de reacción (con Pulsador) ===");
  digitalWrite(ledPin, LOW); // Asegurarse que el LED empieza apagado
  delay(2000);
}

void loop() {
  esperandoGolpe = true;
  // 1. Espera aleatoria entre 1 y 6 segundos
  int espera = random(1000, 6000);
  delay(espera);

  // 2. Enciende el LED → señal de inicio
  digitalWrite(ledPin, HIGH);
  tInicio = millis();
  

  // 3. Espera hasta que se detecte un golpe
  while (esperandoGolpe) {
    int valor = digitalRead(pulsadorPin);
    if (valor == 0) {
      unsigned long tReaccion = millis() - tInicio;
      Serial.print("Golpe detectado! Tiempo de reacción: ");
      Serial.print(tReaccion); Serial.println(" ms");

      // Apago LED y preparo próxima medición
      digitalWrite(ledPin, LOW);
      esperandoGolpe = false;
      delay(1000);  // pausa antes de la siguiente ronda
    }
  }
}
