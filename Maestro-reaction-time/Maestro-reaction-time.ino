#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>

WebServer server(80);

// --- Estructura de mensaje ESP-NOW ---
typedef struct {
  int tipo;   // 0 = orden, 1 = respuesta
  int valor;  // tiempo en ms
  int modo;   // informativo
} mensajeRT;

mensajeRT msg;

// --- Dirección MAC del esclavo ---
uint8_t receptorMAC[] = {0x94, 0x54, 0xC5, 0xAF, 0x0D, 0x08};
esp_now_peer_info_t peer;

// --- Configuración de datos ---
#define MAX_DATOS 20
int tiemposAccion[MAX_DATOS];
int tiemposReaccion[MAX_DATOS];
int indiceAccion = 0, indiceReaccion = 0;
bool llenoAccion = false, llenoReaccion = false;

// --- Variables de control ---
int modoActual = 1;  // 1 = Acción, 2 = Reacción
bool esperandoRespuesta = false;
bool recibido = false;
int ultimoTiempo = 0;

// =====================================================
// =============== FUNCIONES DE ESTADÍSTICA ============
// =====================================================
float promedio(int *arr, int tam, bool lleno) {
  int n = lleno ? MAX_DATOS : tam;
  if (n == 0) return 0;
  long suma = 0;
  for (int i = 0; i < n; i++) suma += arr[i];
  return (float)suma / n;
}

int minimo(int *arr, int tam, bool lleno) {
  int n = lleno ? MAX_DATOS : tam;
  if (n == 0) return 0;
  int m = arr[0];
  for (int i = 1; i < n; i++) if (arr[i] < m) m = arr[i];
  return m;
}

int maximo(int *arr, int tam, bool lleno) {
  int n = lleno ? MAX_DATOS : tam;
  if (n == 0) return 0;
  int m = arr[0];
  for (int i = 1; i < n; i++) if (arr[i] > m) m = arr[i];
  return m;
}

// =====================================================
// =============== FUNCIONES DE BORRADO ================
// =====================================================
void borrarUltimo() {
  if (modoActual == 1) {
    if (indiceAccion > 0 || llenoAccion) {
      if (llenoAccion && indiceAccion == 0) indiceAccion = MAX_DATOS;
      indiceAccion--;
      tiemposAccion[indiceAccion] = 0;
      if (indiceAccion == 0 && llenoAccion) llenoAccion = false;
      Serial.println("🗑 Última medición de Acción borrada");
    }
  } else {
    if (indiceReaccion > 0 || llenoReaccion) {
      if (llenoReaccion && indiceReaccion == 0) indiceReaccion = MAX_DATOS;
      indiceReaccion--;
      tiemposReaccion[indiceReaccion] = 0;
      if (indiceReaccion == 0 && llenoReaccion) llenoReaccion = false;
      Serial.println("🗑 Última medición de Reacción borrada");
    }
  }
}

void borrarTodo() {
  for (int i = 0; i < MAX_DATOS; i++) {
    tiemposAccion[i] = 0;
    tiemposReaccion[i] = 0;
  }
  indiceAccion = indiceReaccion = 0;
  llenoAccion = llenoReaccion = false;
  Serial.println("🧹 Todos los datos borrados");
}

// =====================================================
// =============== INTERFAZ WEB ========================
// =====================================================
String generarTablasHTML() {
  String html = "";
  html += "<div class='tabla'><h3 style='color:#4CAF50'>Modo Acción</h3>";
  html += "<table><tr><th>#</th><th>Tiempo (ms)</th></tr>";
  int nA = llenoAccion ? MAX_DATOS : indiceAccion;
  for (int i = 0; i < nA; i++)
    html += "<tr><td>" + String(i + 1) + "</td><td>" + String(tiemposAccion[i]) + "</td></tr>";
  html += "</table>";
  html += "<p>Promedio: " + String(promedio(tiemposAccion, indiceAccion, llenoAccion), 1) +
          " ms | Mín: " + String(minimo(tiemposAccion, indiceAccion, llenoAccion)) +
          " | Máx: " + String(maximo(tiemposAccion, indiceAccion, llenoAccion)) + "</p></div>";

  html += "<div class='tabla'><h3 style='color:#2196F3'>Modo Reacción</h3>";
  html += "<table><tr><th>#</th><th>Tiempo (ms)</th></tr>";
  int nR = llenoReaccion ? MAX_DATOS : indiceReaccion;
  for (int i = 0; i < nR; i++)
    html += "<tr><td>" + String(i + 1) + "</td><td>" + String(tiemposReaccion[i]) + "</td></tr>";
  html += "</table>";
  html += "<p>Promedio: " + String(promedio(tiemposReaccion, indiceReaccion, llenoReaccion), 1) +
          " ms | Mín: " + String(minimo(tiemposReaccion, indiceReaccion, llenoReaccion)) +
          " | Máx: " + String(maximo(tiemposReaccion, indiceReaccion, llenoReaccion)) + "</p></div>";
  return html;
}

String getHTML() {
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>ESP32 Tiempo Reacción</title><style>";
  html += "body{font-family:Arial;text-align:center;background:#121212;color:#eee;margin:0;padding:0}";
  html += "h1{background:#1e1e1e;padding:15px 0;margin:0;color:#00e5ff;box-shadow:0 2px 5px rgba(0,0,0,0.5)}";
  html += "button{font-size:17px;margin:8px;padding:10px 20px;border:none;border-radius:8px;cursor:pointer;transition:0.3s}";
  html += "button:hover{opacity:0.85;transform:scale(1.05)}";
  html += ".modo{background:#2196F3}.inicio{background:#4CAF50}";
  html += ".borrar{background:#f44336}.borrarTodo{background:#9C27B0}";
  html += "table{border-collapse:collapse;margin:auto;width:70%;color:#ddd}";
  html += "th,td{border:1px solid #555;padding:6px}";
  html += ".tabla{background:#1f1f1f;padding:10px;border-radius:12px;box-shadow:0 0 10px rgba(0,0,0,0.4);margin:15px}";
  html += "</style></head><body>";
  html += "<h1>⏱ Control de Tiempo ESP32</h1>";
  html += "<p id='modo'>Modo actual: <b>" + String(modoActual == 1 ? "Acción" : "Reacción") + "</b></p>";
  html += "<button class='modo' onclick=\"fetch('/modo')\">Cambiar modo</button>";
  html += "<button class='inicio' onclick=\"fetch('/inicio')\">Iniciar prueba</button>";
  html += "<button class='borrar' onclick=\"fetch('/borrar?modo=ultimo')\">Borrar último</button>";
  html += "<button class='borrarTodo' onclick=\"if(confirm('¿Seguro?'))fetch('/borrar?modo=todo')\">Borrar todo</button>";
  html += "<div id='tablas'>" + generarTablasHTML() + "</div>";
  html += "<script>setInterval(()=>{fetch('/valor').then(r=>r.text()).then(t=>document.getElementById('tablas').innerHTML=t);"
          "fetch('/modoActual').then(r=>r.text()).then(m=>document.getElementById('modo').innerHTML=m);},1000);</script>";
  html += "</body></html>";
  return html;
}

// =====================================================
// =============== MANEJO DE PETICIONES =================
// =====================================================
void handleRoot() { server.send(200, "text/html", getHTML()); }

void handleModo() {
  modoActual = (modoActual == 1) ? 2 : 1;
  Serial.printf("🔄 Modo cambiado a %s\n", modoActual == 1 ? "Acción" : "Reacción");
  server.send(200, "text/plain", "OK");
}

void handleInicio() {
  if (!esperandoRespuesta) {
    int espera = random(1000, 6000);
    msg.tipo = 0;
    msg.valor = espera;
    msg.modo = modoActual;
    esp_now_send(receptorMAC, (uint8_t*)&msg, sizeof(msg));
    esperandoRespuesta = true;
    Serial.printf("📤 Enviando orden (%s): esperar %d ms\n", modoActual == 1 ? "Acción" : "Reacción", espera);
  }
  server.send(200, "text/plain", "Iniciada");
}

void handleValor() { server.send(200, "text/html", generarTablasHTML()); }

void handleModoActual() {
  String txt = "Modo actual: <b>" + String(modoActual == 1 ? "Acción" : "Reacción") + "</b>";
  server.send(200, "text/html", txt);
}

void handleBorrar() {
  if (server.hasArg("modo")) {
    String m = server.arg("modo");
    if (m == "ultimo") borrarUltimo();
    else if (m == "todo") borrarTodo();
    server.send(200, "text/plain", "Borrado");
  } else server.send(400, "text/plain", "Falta modo");
}

// =====================================================
// =============== RECEPCIÓN ESP-NOW ====================
// =====================================================
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  mensajeRT incoming;
  memcpy(&incoming, data, sizeof(incoming));
  ultimoTiempo = incoming.valor;
  recibido = true;
  esperandoRespuesta = false;

  if (modoActual == 1) {
    if (indiceAccion < MAX_DATOS) tiemposAccion[indiceAccion++] = incoming.valor;
    else { llenoAccion = true; indiceAccion = 0; tiemposAccion[indiceAccion++] = incoming.valor; }
  } else {
    if (indiceReaccion < MAX_DATOS) tiemposReaccion[indiceReaccion++] = incoming.valor;
    else { llenoReaccion = true; indiceReaccion = 0; tiemposReaccion[indiceReaccion++] = incoming.valor; }
  }

  Serial.printf("⏱ Tiempo recibido (%s): %d ms\n", modoActual == 1 ? "Acción" : "Reacción", incoming.valor);
}

// =====================================================
// =============== CONFIGURACIÓN ========================
// =====================================================
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("ESP-Reaccion", "12345678");
  delay(1000);
  Serial.println(WiFi.softAPIP());

  if (esp_now_init() != ESP_OK) { Serial.println("Error al iniciar ESP-NOW"); return; }
  esp_now_register_recv_cb(OnDataRecv);
  memcpy(peer.peer_addr, receptorMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  server.on("/", handleRoot);
  server.on("/modo", handleModo);
  server.on("/inicio", handleInicio);
  server.on("/valor", handleValor);
  server.on("/modoActual", handleModoActual);
  server.on("/borrar", handleBorrar);
  server.begin();
  Serial.println("Servidor web iniciado ✅");
}

void loop() { server.handleClient(); }

