#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_NeoPixel.h>

#define PiezoPin    34        //Pin del Piezo electrico
#define LED_PIN     18        //Pin tira de led
#define NUM_PIXELS  3         //Numero de led

const int umbral = 400;       //Umbral del PiezoElectrico
unsigned long tInicio;        //Contador de tiempo
bool esperandoGolpe = false;  //Estado para activar el contador

// --- Configuración de la tira LED ---
Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- Configuración del ESP Now ---
typedef struct {
  int tipo; // 0 = espera, 1 = tiempo reacción
  int valor;
} mensajeRT;

mensajeRT msg;
esp_now_peer_info_t peer;
uint8_t masterMAC[] = {0x94, 0xE6, 0x86, 0x3C, 0x83, 0xB4}; 
//MAC ESP 1:
//MAC ESP 2:
//MAC ESP 3:

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  mensajeRT incoming;
  memcpy(&incoming, data, sizeof(incoming));

  if (incoming.tipo == 0) {
    setColor(255, 0, 0); // Rojo -> Tiempo de espera
    int espera = incoming.valor;
    Serial.print("Recibido esperar: "); Serial.println(espera); //Tiempo enviado por el maestro
    delay(espera);

    esperandoGolpe = true;
    setColor(0, 255, 0); // Verde → señal de inicio 
    tInicio = millis();
  }
}

void setColor(int r, int g, int b) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

void setup() {
  Serial.begin(115200);
  pixels.begin();
  pixels.clear();
  pixels.show();

  setColor(255, 0, 0); // Rojo -> Tiempo de espera

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
    int valor = analogRead(PiezoPin); //Leemos el valor del golpe
    if (valor > umbral) {
      int tiempo = millis() - tInicio;
      esperandoGolpe = false;

      setColor(0, 0, 255);  // Azul -> golpe registrado
      Serial.print("Tiempo reacción local: "); Serial.println(tiempo);

      msg.tipo = 1;
      msg.valor = tiempo;
      esp_now_send(masterMAC, (uint8_t*)&msg, sizeof(msg)); //Envia el dato del tiempo al maestro
    }
  }
}


