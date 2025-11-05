/*CONEXION PIEZO ELECTRICO*/
/*Cable negro GND*/
/*Cable Rojo GPIO34*/
/*Entre GND y GPIO34 una resistencia de 1 Mega Ohm */

const int piezoPin = 34;  // ADC para los dos piezos en paralelo
const int umbral = 1000;   // Ajustar con pruebas
unsigned long debounce_ms = 10;
unsigned long lastHit = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  int valor = analogRead(piezoPin);

  if (valor < umbral && millis() - lastHit > debounce_ms) {
    lastHit = millis();
    Serial.print("Golpe detectado! Valor: ");
    Serial.println(valor);
  }
}
