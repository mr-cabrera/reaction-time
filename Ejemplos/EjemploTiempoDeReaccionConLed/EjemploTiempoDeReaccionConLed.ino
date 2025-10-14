const int piezoPin = 34;  // Entrada del sensor piezo
const int ledPin = 2;     // LED indicador
const int umbral = 100;   // Nivel de detección del golpe
unsigned long tInicio = 0;
bool esperandoGolpe = false;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  Serial.println("=== Medidor de tiempo de reacción ===");
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
    int valor = analogRead(piezoPin);
    if (valor > umbral) {
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
