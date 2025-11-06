#include <WiFi.h>
#include <esp_now.h>

#define BOTON_MODO    13     // Cambia entre modo 1 y 2
#define BOTON_INICIO  14      // Inicia la prueba

typedef struct {
  int tipo; // 0 = espera, 1 = tiempo reacción
  int valor;
  int modo;
} mensajeRT;

mensajeRT msg;
uint8_t receptorMAC[] = {0x94, 0x54, 0xC5, 0xAF, 0x0D, 0x08}; // MAC del esclavo
//MAC ESP 1: 94:54:C5:B0:92:D4 (Maestro)
//MAC ESP 2: 94:54:C5:AF:0D:08 (Esclavo)
//MAC ESP 3:

esp_now_peer_info_t peer;

bool recibido = false;
bool esperandoRespuesta = false;
int modoActual = 1; 

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  mensajeRT incoming;
  memcpy(&incoming, data, sizeof(incoming));
  
  if (incoming.tipo == 1) {
    Serial.print("Tiempo recibido: ");
    Serial.print(incoming.valor);
    Serial.println(" ms");

    recibido = true; // 🔹 Marca que el esclavo ya respondió
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BOTON_MODO, INPUT_PULLUP);
  pinMode(BOTON_INICIO, INPUT_PULLUP);
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
  static unsigned long ultimoCambioModo = 0;

  // --- Cambio de modo ---
  if (digitalRead(BOTON_MODO) == LOW && millis() - ultimoCambioModo > 500) {
    modoActual = (modoActual == 1) ? 2 : 1;
    Serial.print("Modo cambiado a: ");
    Serial.println(modoActual);
    ultimoCambioModo = millis();
  }

  // --- Inicio de prueba ---
  if (digitalRead(BOTON_INICIO) == LOW && !esperandoRespuesta) {
    int espera = random(1000, 6000);

    msg.tipo = 0;  
    msg.valor = espera;
    msg.modo = modoActual;

    Serial.print("Enviando orden (modo ");
    Serial.print(modoActual);
    Serial.print("): esperar ");
    Serial.print(espera);
    Serial.println(" ms");

    esp_err_t result = esp_now_send(receptorMAC, (uint8_t*)&msg, sizeof(msg));
    if (result != ESP_OK) {
      Serial.println("❌ Error al enviar mensaje");
    }

    esperandoRespuesta = true;
    recibido = false;
    delay(300);  // Anti-rebote
  }

  // --- Cuando llega la respuesta ---
  if (esperandoRespuesta && recibido) {
    esperandoRespuesta = false;
    Serial.println("Listo para nueva prueba...\n");
    delay(1500);
  }
}