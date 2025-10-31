#include <WiFi.h>
#include <esp_now.h>

typedef struct {
  int tipo; // 0 = espera, 1 = tiempo reacción
  int valor;
} mensajeRT;

mensajeRT msg;
uint8_t receptorMAC[] = {0xF4, 0x65, 0x0B, 0x46, 0xE5, 0xE0}; // MAC del esclavo

esp_now_peer_info_t peer;

bool recibido = false;  // 🔹 Bandera para saber si ya se recibió la respuesta
unsigned long ultimaOrden = 0;

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  mensajeRT incoming;
  memcpy(&incoming, data, sizeof(incoming));
  
  if (incoming.tipo == 1) {
    Serial.print("Tiempo de reacción recibido: ");
    Serial.print(incoming.valor);
    Serial.println(" ms");

    recibido = true; // 🔹 Marca que el esclavo ya respondió
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  delay(2000); // Tiempo de configuración wifi
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb(OnDataRecv);

  memcpy(peer.peer_addr, receptorMAC, 6);
  peer.channel = 0;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  Serial.println("Maestro listo");
}

void loop() {
  static bool esperandoRespuesta = false;

  if (!esperandoRespuesta) {
    int espera = random(1000, 6000);

    msg.tipo = 0;  
    msg.valor = espera;

    Serial.print("Orden enviada: esperar ");
    Serial.print(espera);
    Serial.println(" ms");
    
    esp_now_send(receptorMAC, (uint8_t*)&msg, sizeof(msg));

    esperandoRespuesta = true;   // 🔹 Esperar la respuesta del esclavo
    recibido = false;            // 🔹 Reset de bandera
  }

  // 🔹 Cuando se recibe la respuesta, preparar el siguiente envío
  if (esperandoRespuesta && recibido) {
    esperandoRespuesta = false;
    Serial.println("Listo para nueva prueba...\n");
    delay(2000); // Pequeña pausa entre pruebas
  }
}


