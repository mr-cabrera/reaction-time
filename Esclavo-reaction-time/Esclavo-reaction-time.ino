#include <WiFi.h>       // Biblioteca para manejo de WiFi en ESP32
#include <esp_now.h>    // Biblioteca para comunicación inalámbrica ESP-NOW

// --- Definición de pines ---
#define PiezoPin  34        // Pin analógico conectado al sensor piezoeléctrico
#define pinR      25         // Pin para LED rojo
#define pinG      26         // Pin para LED verde
#define pinB      27         // Pin para LED azul

// --- Variables globales ---
int sumaLecturas, lecturaMinima, umbral;   // Variables para calibrar el sensor piezoeléctrico
unsigned long tInicio;                     // Almacena el tiempo en que comienza la medición
int modoActual = 1;                        // Modo de operación actual (recibido desde el maestro)
bool Accion = false;                       // Bandera que indica si el sistema está esperando un golpe

// --- Estructura de datos para comunicación por ESP-NOW ---
typedef struct {
  int tipo;   // 0 = señal de espera, 1 = resultado de tiempo de reacción
  int valor;  // Valor enviado (puede ser tiempo o tiempo de espera)
  int modo;   // Modo actual del sistema
} mensajeRT;

mensajeRT msg;                    // Variable para enviar mensajes
esp_now_peer_info_t peer;         // Información del dispositivo remoto (maestro)
uint8_t masterMAC[] = {0x94, 0xE6, 0x86, 0x3C, 0x83, 0xB4}; // Dirección MAC del maestro

// --- Función de callback: se ejecuta cuando se recibe un mensaje por ESP-NOW ---
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  mensajeRT incoming;
  memcpy(&incoming, data, sizeof(incoming));   // Copia los datos recibidos a la estructura local

  // Actualizamos el modo recibido desde el maestro
  modoActual = incoming.modo;

  // Si el tipo recibido es "0", significa que el maestro indica un tiempo de espera
  if (incoming.tipo == 0) {
    int espera = incoming.valor;   // Tiempo de espera recibido
    Serial.print("Recibido esperar: ");
    Serial.print(espera);
    Serial.print(" ms  |  Modo: ");
    Serial.println(modoActual);
    
    // LED rojo encendido → Indica fase de espera
    digitalWrite(pinR, LOW);
    digitalWrite(pinG, HIGH);
    digitalWrite(pinB, HIGH);

    // Espera la cantidad de milisegundos indicada por el maestro
    delay(espera);

    // Luego del tiempo de espera, habilita la detección del golpe si corresponde
    if (modoActual == 1 || modoActual == 2) {
      // Activa la detección del golpe con el piezoeléctrico
      Accion = true;

      // LED verde encendido → Indica que está listo para detectar el golpe
      digitalWrite(pinR, HIGH);
      digitalWrite(pinG, LOW);
      digitalWrite(pinB, HIGH);

      // Guarda el tiempo de inicio para medir la reacción
      tInicio = millis();
    }
  }
}

// --- Configuración inicial ---
void setup() {
  Serial.begin(115200);  // Inicializa la comunicación serial para depuración

  // Configura los pines del LED RGB como salida
  pinMode(pinR, OUTPUT);
  pinMode(pinG, OUTPUT);
  pinMode(pinB, OUTPUT);

  // LED rojo encendido → Estado inicial: esperando instrucciones
  digitalWrite(pinR, LOW);
  digitalWrite(pinG, HIGH);
  digitalWrite(pinB, HIGH);

  // Configura el modo WiFi en estación (requerido para ESP-NOW)
  WiFi.mode(WIFI_STA);
  delay(2000);

  // Muestra la dirección MAC del dispositivo esclavo
  Serial.print("MAC Esclavo: ");
  Serial.println(WiFi.macAddress());

  // Inicializa ESP-NOW
  if (esp_now_init() != ESP_OK) return;
  
  // Registra la función callback para recibir datos
  esp_now_register_recv_cb(OnDataRecv);

  // Configura el maestro como "peer" (dispositivo conocido)
  memcpy(peer.peer_addr, masterMAC, 6);
  peer.channel = 0;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  // --- Calibración del sensor piezoeléctrico ---
  // Se toman 100 muestras para calcular un promedio y establecer el umbral
  for (int i = 0; i < 100; i++) {
    int lectura = analogRead(PiezoPin); // Lee el valor del piezo (0 a 4095 en ESP32)

    if (i == 0) {
      lecturaMinima = lectura;  // Inicializa el mínimo
    } else if (lectura < lecturaMinima) {
      lecturaMinima = lectura;  // Actualiza el mínimo si se encuentra uno menor
    }

    sumaLecturas += lectura; // Acumula las lecturas
    delay(1); // Pequeña pausa entre muestras
  }

  // Calcula el promedio de las lecturas
  int promedio = (sumaLecturas / 100);
  Serial.print("Promedio: "); Serial.println(promedio);

  // Define un umbral: se considera golpe cuando la lectura baja 200 unidades del promedio
  umbral = promedio - 200;
  Serial.print("Umbral: "); Serial.println(umbral);
  delay(1000);

  Serial.println("Esclavo listo");
}

// --- Bucle principal ---
void loop() {
  // Solo entra si se activó la bandera Accion (modo de detección)
  if (Accion) {
    int valor = analogRead(PiezoPin); // Lee el valor actual del sensor piezoeléctrico
    Serial.print("Valor: "); Serial.println(valor);

    // Si el valor cae por debajo del umbral → se detecta un golpe
    if (valor < umbral) {
      int tiempo = millis() - tInicio; // Calcula el tiempo de reacción
      Accion = false; // Detiene la espera de golpe

      // LED azul encendido → Golpe detectado
      digitalWrite(pinR, HIGH);
      digitalWrite(pinG, HIGH);
      digitalWrite(pinB, LOW);

      Serial.print("Valor: ");
      Serial.print(valor);
      Serial.print(" - Tiempo local: ");
      Serial.println(tiempo);

      // Envía al maestro el tiempo medido
      msg.tipo = 1;        // Tipo 1 = resultado de tiempo de reacción
      msg.valor = tiempo;  // Envía el valor del tiempo
      esp_now_send(masterMAC, (uint8_t*)&msg, sizeof(msg)); // Transmite los datos al maestro
    }
  }
}

