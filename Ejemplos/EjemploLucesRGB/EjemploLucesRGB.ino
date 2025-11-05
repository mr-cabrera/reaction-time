// Pines ESP32
const int pinR = 25;
const int pinG = 26;
const int pinB = 27;

void setup() {
  pinMode(pinR, OUTPUT);
  pinMode(pinG, OUTPUT);
  pinMode(pinB, OUTPUT);
  
  // Apagar todos al inicio
  digitalWrite(pinR, HIGH);
  digitalWrite(pinG, HIGH);
  digitalWrite(pinB, HIGH);
  
  Serial.begin(115200);
  Serial.println("LED RGB Listo - Sin librerías");
}

void loop() {
  Serial.println("🔴 ROJO");
  digitalWrite(pinR, LOW);
  digitalWrite(pinG, HIGH);
  digitalWrite(pinB, HIGH);
  delay(2000);
  
  Serial.println("🟢 VERDE");
  digitalWrite(pinR, HIGH);
  digitalWrite(pinG, LOW);
  digitalWrite(pinB, HIGH);
  delay(2000);
  
  Serial.println("🔵 AZUL");
  digitalWrite(pinR, HIGH);
  digitalWrite(pinG, HIGH);
  digitalWrite(pinB, LOW);
  delay(2000);
  
  Serial.println("⚫ APAGADO");
  digitalWrite(pinR, HIGH);
  digitalWrite(pinG, HIGH);
  digitalWrite(pinB, HIGH);
  delay(1000);
}
