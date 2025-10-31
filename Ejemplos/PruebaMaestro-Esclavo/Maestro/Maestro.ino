#include <WiFi.h>
#include <esp_now.h>

uint8_t receptorMAC[] = {0x94, 0xE6, 0x86, 0x3C, 0x83, 0xB4}; // CAMBIAR POR LA MAC DEL RECEPTOR

typedef struct struct_message {
  int valor;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Envio: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Exitoso" : "Fallido");
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  WiFi.mode(WIFI_STA);
  delay(2000);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error inicializando ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  memcpy(peerInfo.peer_addr, receptorMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("No se pudo agregar el peer");
    return;
  }
}

void loop() {
  myData.valor++;
  esp_err_t result = esp_now_send(receptorMAC, (uint8_t *) &myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Dato enviado");
  } else {
    Serial.println("Error enviando");
  }

  delay(1000);
}

