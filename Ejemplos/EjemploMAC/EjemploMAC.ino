#include <WiFi.h> // Librería Wi-Fi específica para ESP32

void setup() {
  Serial.begin(115200);
  Serial.println(); // Salto de línea inicial
  Serial.println("--- Buscando la Dirección MAC de mi ESP32 ---");

  // Activamos el modo Estación (cliente) para inicializar el hardware de Wi-Fi.
  // Es necesario para poder leer la MAC.
  WiFi.mode(WIFI_STA);
  delay(2000);

  // Obtenemos e imprimimos la dirección MAC
  Serial.print("Mi Dirección MAC es: ");
  Serial.println(WiFi.macAddress());
  
  Serial.println("-------------------------------------------");
}

void loop() {
  // No es necesario hacer nada en el loop,
  // la MAC solo se imprime una vez al iniciar.
  delay(10000);
}