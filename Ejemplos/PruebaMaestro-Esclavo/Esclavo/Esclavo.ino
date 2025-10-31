#include <WiFi.h>
#include <esp_now.h>

String mac;

typedef struct struct_message {
  int valor;
} struct_message;

struct_message incomingData;

void OnDataRecv(const esp_now_recv_info_t * recv_info, const uint8_t * data, int len) {
  memcpy(&incomingData, data, sizeof(incomingData));
  Serial.print("Dato recibido: ");
  Serial.println(incomingData.valor);
}

void confgMac(){
  
  mac = WiFi.macAddress();

  Serial.print("MAC ESCLAVO: ");
  Serial.println(mac);
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("Iniciando programa...");

  WiFi.mode(WIFI_MODE_STA);

  WiFi.disconnect(true, true); // Borra la NVS

  delay(2000);

  confgMac();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error inicializando ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {

}


