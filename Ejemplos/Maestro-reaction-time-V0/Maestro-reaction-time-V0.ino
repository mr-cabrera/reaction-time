#include <WiFi.h>         // Biblioteca para manejar WiFi en ESP32
#include <esp_now.h>      // Biblioteca para comunicación inalámbrica ESP-NOW
#include <WebServer.h>    // Biblioteca para crear un servidor web local

WebServer server(80);     // Crea un servidor web en el puerto 80 (HTTP estándar)

// --- Estructura para intercambio de datos por ESP-NOW ---
typedef struct {
  int tipo;   // 0 = orden enviada al esclavo, 1 = respuesta recibida del esclavo
  int valor;  // Puede representar tiempo de espera o tiempo medido en ms
  int modo;   // 1 = modo Acción, 2 = modo Reacción
} mensajeRT;

mensajeRT msg;  // Variable usada para enviar o recibir mensajes

// --- Dirección MAC del dispositivo esclavo ---
uint8_t receptorMAC[] = {0x94, 0x54, 0xC5, 0xAF, 0x0D, 0x08};
esp_now_peer_info_t peer; // Estructura para registrar al esclavo como "peer"

// --- Variables de control ---
int modoActual = 1;          // Define el modo actual: 1 = Acción, 2 = Reacción
bool esperandoRespuesta = false; // Indica si se espera una respuesta del esclavo
bool recibido = false;            // Indica si se recibió un dato del esclavo
int ultimoTiempo = 0;             // Almacena el último tiempo recibido

// =====================================================
// ============ INTERFAZ WEB HTML ======================
// =====================================================
String getHTML() {
  // Devuelve una página HTML completa con botones y visualización del tiempo
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>ESP32 Tiempo Reacción</title>";
  html += "<style>";
  html += "body { font-family: Arial; text-align:center; background:#111; color:#eee; }";
  html += "h1 { color:#4CAF50; }";
  html += "button { font-size:20px; margin:10px; padding:15px 25px; border:none; border-radius:10px; cursor:pointer; }";
  html += ".modo { background:#2196F3; }";  // Botón azul para cambiar modo
  html += ".inicio { background:#4CAF50; }"; // Botón verde para iniciar prueba
  html += "#tiempo { font-size:28px; color:#FFD700; margin-top:30px; }"; // Texto del resultado
  html += "</style></head><body>";

  // --- Contenido visible de la página ---
  html += "<h1>Control de Tiempo ESP32</h1>";
  html += "<p>Modo actual: <b>" + String(modoActual == 1 ? "Acción" : "Reacción") + "</b></p>";
  html += "<button class='modo' onclick=\"fetch('/modo')\">Cambiar modo</button>";
  html += "<button class='inicio' onclick=\"fetch('/inicio')\">Iniciar prueba</button>";
  html += "<div id='tiempo'>Tiempo: " + String(ultimoTiempo) + " ms</div>";

  // --- Script en JavaScript para actualizar el valor del tiempo cada 1 segundo ---
  html += "<script>setInterval(()=>fetch('/valor').then(r=>r.text()).then(t=>document.getElementById('tiempo').innerHTML=t),1000);</script>";

  html += "</body></html>";
  return html;
}

// =====================================================
// ============ MANEJO DE PETICIONES WEB ===============
// =====================================================

// --- Ruta principal "/" -> muestra la interfaz ---
void handleRoot() {
  server.send(200, "text/html", getHTML());  // Devuelve el HTML al navegador
}

// --- Ruta "/modo" -> cambia entre modo Acción y Reacción ---
void handleModo() {
  modoActual = (modoActual == 1) ? 2 : 1;  // Alterna entre 1 y 2
  Serial.print("Modo cambiado a ");
  Serial.println(modoActual);
  server.send(200, "text/plain", "OK");
}

// --- Ruta "/inicio" -> inicia una nueva prueba ---
void handleInicio() {
  if (!esperandoRespuesta) {  // Solo se puede iniciar si no se espera otro resultado
    int espera = random(1000, 6000);  // Genera un tiempo aleatorio entre 1 y 6 segundos

    // Prepara el mensaje para el esclavo
    msg.tipo = 0;        // Tipo 0 = orden
    msg.valor = espera;  // Tiempo que el esclavo debe esperar antes de activarse
    msg.modo = modoActual; // Envía también el modo actual

    // Muestra por monitor serial
    Serial.print("Enviando orden (modo ");
    Serial.print(modoActual);
    Serial.print("): esperar ");
    Serial.print(espera);
    Serial.println(" ms");

    // Envía el mensaje al esclavo por ESP-NOW
    esp_err_t result = esp_now_send(receptorMAC, (uint8_t*)&msg, sizeof(msg));
    if (result != ESP_OK) {
      Serial.println("❌ Error al enviar mensaje");
    }

    // Marca que está esperando respuesta
    esperandoRespuesta = true;
    recibido = false;
  }

  server.send(200, "text/plain", "Iniciada");
}

// --- Ruta "/valor" -> devuelve el último valor recibido al navegador ---
void handleValor() {
  String texto = "Tiempo: " + String(ultimoTiempo) + " ms<br>Modo: ";
  texto += (modoActual == 1 ? "Acción" : "Reacción");
  server.send(200, "text/html", texto);
}

// =====================================================
// ============ CALLBACK DE RECEPCIÓN ESP-NOW ==========
// =====================================================

// Esta función se ejecuta automáticamente cuando llega un mensaje por ESP-NOW
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  mensajeRT incoming;
  memcpy(&incoming, data, sizeof(incoming));  // Copia los datos recibidos a la estructura local

  // Si el mensaje recibido es de tipo 1 (respuesta del esclavo)
  if (incoming.tipo == 1) {
    ultimoTiempo = incoming.valor;    // Guarda el tiempo recibido
    recibido = true;                  // Marca que llegó un dato
    esperandoRespuesta = false;       // Ya no se espera más respuesta
    Serial.printf("⏱ Tiempo recibido (modo %d): %d ms\n", incoming.modo, incoming.valor);
  }
}

// =====================================================
// ============ CONFIGURACIÓN INICIAL ==================
// =====================================================
void setup() {
  Serial.begin(115200);

  // --- Configuración de red ---
  WiFi.mode(WIFI_AP_STA);                     // Modo dual: punto de acceso + estación
  WiFi.softAP("ESP-Reaccion", "12345678");    // Crea una red WiFi llamada "ESP-Reaccion"
  delay(2000);

  Serial.print("🌐 AP iniciado. Conéctate a: ");
  Serial.println("ESP-Reaccion");
  Serial.print("IP local: ");
  Serial.println(WiFi.softAPIP());

  // --- Inicialización de ESP-NOW ---
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al iniciar ESP-NOW");
    return;
  }

  // Registra la función que se ejecuta al recibir datos
  esp_now_register_recv_cb(OnDataRecv);

  // Configura el esclavo como "peer" autorizado
  memcpy(peer.peer_addr, receptorMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  // --- Configuración del servidor web ---
  server.on("/", handleRoot);     // Página principal
  server.on("/modo", handleModo); // Cambio de modo
  server.on("/inicio", handleInicio); // Inicio de prueba
  server.on("/valor", handleValor);   // Último valor medido
  server.begin();

  Serial.println("Servidor web iniciado");
}

// =====================================================
// ============ BUCLE PRINCIPAL ========================
// =====================================================
void loop() {
  // Atiende continuamente las peticiones HTTP del navegador
  server.handleClient();
}
