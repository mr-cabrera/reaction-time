#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>

WebServer server(80);

typedef struct {
  int tipo;   // 0 = orden, 1 = respuesta
  int valor;  // tiempo en ms
  int modo;   // 1 = acción, 2 = reacción
} mensajeRT;

mensajeRT msg;

uint8_t receptorMAC[] = {0x94, 0x54, 0xC5, 0xAF, 0x0D, 0x08}; // MAC del esclavo
esp_now_peer_info_t peer;

int modoActual = 1;          // 1 = Acción, 2 = Reacción
bool esperandoRespuesta = false;
bool recibido = false;
int ultimoTiempo = 0;

// --- HTML básico de la interfaz ---
String getHTML() {
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>ESP32 Tiempo Reacción</title>";
  html += "<style>";
  html += "body { font-family: Arial; text-align:center; background:#111; color:#eee; }";
  html += "h1 { color:#4CAF50; }";
  html += "button { font-size:20px; margin:10px; padding:15px 25px; border:none; border-radius:10px; cursor:pointer; }";
  html += ".modo { background:#2196F3; }";
  html += ".inicio { background:#4CAF50; }";
  html += "#tiempo { font-size:28px; color:#FFD700; margin-top:30px; }";
  html += "</style></head><body>";
  html += "<h1>Control de Tiempo ESP32</h1>";
  html += "<p>Modo actual: <b>" + String(modoActual == 1 ? "Acción" : "Reacción") + "</b></p>";
  html += "<button class='modo' onclick=\"fetch('/modo')\">Cambiar modo</button>";
  html += "<button class='inicio' onclick=\"fetch('/inicio')\">Iniciar prueba</button>";
  html += "<div id='tiempo'>Tiempo: " + String(ultimoTiempo) + " ms</div>";
  html += "<script>setInterval(()=>fetch('/valor').then(r=>r.text()).then(t=>document.getElementById('tiempo').innerHTML=t),1000);</script>";
  html += "</body></html>";
  return html;
}

// --- Respuesta a / (página principal) ---
void handleRoot() {
  server.send(200, "text/html", getHTML());
}

// --- Cambio de modo ---
void handleModo() {
  modoActual = (modoActual == 1) ? 2 : 1;
  Serial.print("Modo cambiado a ");
  Serial.println(modoActual);
  server.send(200, "text/plain", "OK");
}

// --- Iniciar prueba ---
void handleInicio() {
  if (!esperandoRespuesta) {
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
  }
  server.send(200, "text/plain", "Iniciada");
}

// --- Mostrar último valor ---
void handleValor() {
  String texto = "Tiempo: " + String(ultimoTiempo) + " ms<br>Modo: ";
  texto += (modoActual == 1 ? "Acción" : "Reacción");
  server.send(200, "text/html", texto);
}

// --- Callback de recepción ESP-NOW ---
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  mensajeRT incoming;
  memcpy(&incoming, data, sizeof(incoming));

  if (incoming.tipo == 1) {
    ultimoTiempo = incoming.valor;
    recibido = true;
    esperandoRespuesta = false;
    Serial.printf("⏱ Tiempo recibido (modo %d): %d ms\n", incoming.modo, incoming.valor);
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("ESP-Reaccion", "12345678");
  delay(2000);
  Serial.print("🌐 AP iniciado. Conéctate a: ");
  Serial.println("ESP-Reaccion");
  Serial.print("IP local: ");
  Serial.println(WiFi.softAPIP());

  // --- Inicializar ESP-NOW ---
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al iniciar ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  memcpy(peer.peer_addr, receptorMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  // --- Servidor web ---
  server.on("/", handleRoot);
  server.on("/modo", handleModo);
  server.on("/inicio", handleInicio);
  server.on("/valor", handleValor);
  server.begin();

  Serial.println("Servidor web iniciado");
}

void loop() {
  server.handleClient();
}
