#include <WiFi.h>
#include <esp_now.h>

const int pulsadorPin = 25;
const int ledPin = 26;  
const int umbral = 400;

typedef struct {
  int tipo; // 0 = espera, 1 = tiempo reacción
  int valor;
} mensajeRT;

mensajeRT msg;
esp_now_peer_info_t peer;
uint8_t masterMAC[] = {0x94, 0xE6, 0x86, 0x3C, 0x83, 0xB4}; // CAMBIAR A MAC DEL MAESTRO 94:E6:86:3C:83:B4

unsigned long tInicio;
bool esperandoGolpe = false;

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  mensajeRT incoming;
  memcpy(&incoming, data, sizeof(incoming));

  if (incoming.tipo == 0) {
    int espera = incoming.valor;
    Serial.print("Recibido esperar: ");
    Serial.println(espera);

    delay(espera);

    tInicio = millis();
    esperandoGolpe = true;
    digitalWrite(ledPin, HIGH);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(pulsadorPin, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);

  delay(2000);

  Serial.print("MAC Esclavo: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) return;
  
  esp_now_register_recv_cb(OnDataRecv);

  memcpy(peer.peer_addr, masterMAC, 6);
  peer.channel = 0;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  Serial.println("Esclavo listo");
}

void loop() {
  if (esperandoGolpe) {
    int valor = digitalRead(pulsadorPin);
    if (valor == 0) {
      int tiempo = millis() - tInicio;
      esperandoGolpe = false;
      digitalWrite(ledPin, LOW);

      Serial.print("Tiempo reacción local: ");
      Serial.println(tiempo);

      msg.tipo = 1;
      msg.valor = tiempo;
      esp_now_send(masterMAC, (uint8_t*)&msg, sizeof(msg));
    }
  }
}


