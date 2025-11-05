#include <WiFi.h>
#include <esp_now.h>

#define PiezoPin  34        //Pin del Piezo electrico
#define pinR      25
#define pinG      26
#define pinB      27

int sumaLecturas, lecturaMinima, umbral;       //Umbral del PiezoElectrico
unsigned long tInicio;          //Contador de tiempo
bool esperandoGolpe = false;    //Estado para activar el contador

// --- Configuración del ESP Now ---
typedef struct {
  int tipo; // 0 = espera, 1 = tiempo reacción
  int valor;
} mensajeRT;

mensajeRT msg;
esp_now_peer_info_t peer;
uint8_t masterMAC[] = {0x94, 0xE6, 0x86, 0x3C, 0x83, 0xB4}; //Mac del MAESTRO
//MAC ESP 1: 94:54:C5:B0:92:D4 (Maestro)
//MAC ESP 2: 94:54:C5:AF:0D:08 (Esclavo)
//MAC ESP Emi: 94:E6:86:3C:83:B4

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  mensajeRT incoming;
  memcpy(&incoming, data, sizeof(incoming));

  if (incoming.tipo == 0) {
    
    digitalWrite(pinR, LOW); digitalWrite(pinG, HIGH); digitalWrite(pinB, HIGH);// Rojo -> Tiempo de espera
    int espera = incoming.valor;
    Serial.print("Recibido esperar: "); Serial.println(espera); //Tiempo enviado por el maestro
    delay(espera);

    esperandoGolpe = true;
    digitalWrite(pinR, HIGH); digitalWrite(pinG, LOW); digitalWrite(pinB, HIGH); // Verde → señal de inicio 
    tInicio = millis();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(pinR, OUTPUT);
  pinMode(pinG, OUTPUT);
  pinMode(pinB, OUTPUT);

  digitalWrite(pinR, LOW);digitalWrite(pinG, HIGH);digitalWrite(pinB, HIGH); //Rojo -> Espera

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

  for (int i = 0; i < 100; i++) {
    int lectura = analogRead(PiezoPin); // Lee el valor del pin analógico (0 a 1023)
    if (i == 0) {
        lecturaMinima = lectura; // Inicializa con la primera lectura
    } else if (lectura < lecturaMinima) {
        lecturaMinima = lectura; // Actualiza si encuentra una más pequeña
    }
    sumaLecturas = sumaLecturas + lectura; // Suma la lectura al acumulador
    delay(1); // Pequeña pausa opcional para asegurar la separación de muestras
  }
  int promedio = (sumaLecturas/100);
  Serial.print("Promedio: "); Serial.println(promedio);
  umbral = promedio - 200;
  Serial.print("Umbral: "); Serial.println(umbral);
  delay(1000);

  Serial.println("Esclavo listo");
}

void loop() {
  if (esperandoGolpe) {
    int valor = analogRead(PiezoPin); //Leemos el valor del golpe
    Serial.print("Valor: "); Serial.println(valor);
    if (valor < umbral) {
      int tiempo = millis() - tInicio;
      esperandoGolpe = false;

      digitalWrite(pinR, HIGH); digitalWrite(pinG, HIGH); digitalWrite(pinB, LOW); // Azul -> golpe registrado
      Serial.print("Valor: "); Serial.print(valor); Serial.print(" - Tiempo reacción local: "); Serial.println(tiempo);

      msg.tipo = 1;
      msg.valor = tiempo;
      esp_now_send(masterMAC, (uint8_t*)&msg, sizeof(msg)); //Envia el dato del tiempo al maestro
    }
  }
}


