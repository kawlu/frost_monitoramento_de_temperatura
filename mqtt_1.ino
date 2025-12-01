#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <DNSServer.h>
#include <PubSubClient.h> 

// ================= CONFIGURAÇÕES DA REDE =================
const char* ap_ssid = "FROST_SYSTEM"; 
const char* ap_password = "12345678"; 

const char* home_ssid = "Alto_da_Boavista_5G"; 
const char* home_password = "VicGiu01";

// ================= MQTT =================
const char* mqtt_server = "broker.hivemq.com"; 
const int mqtt_port = 1883;
const char* mqtt_topic_status = "frost_system/status";
const char* mqtt_topic_set = "frost_system/set"; 

const byte DNS_PORT = 53;
DNSServer dnsServer;

// ================= HARDWARE =================
#define DHTPIN 4
#define DHTTYPE DHT22 
#define PIN_TRANSISTOR 26  

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);

// ================= VARIÁVEIS GLOBAIS =================
float tempAtual = 0, humAtual = 0;
float tempMax = -100, tempMin = 100, tempSoma = 0;
long leiturasCount = 0;
bool masterSwitch = false; 
int rpmDisplay = 0; 

// ================= SITE =================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>FROST</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <div class="container">
        <div class="header">
            <div class="logo">❄️</div>
            <div class="title">FROST</div>
        </div>
        <div id="localization" class="location">Rio de Janeiro - RJ</div>
        
        <div class="main-content">
            <div class="panel">
                <div class="panel-title">Temperatura</div>
                <div class="metric-row"><div class="metric-label">Temperatura atual:</div><div id="temp-atual" class="metric-value">--</div></div>
                <div class="metric-row"><div class="metric-label">Temperatura externa:</div><div id="temp-externa" class="metric-value">--</div></div>
                <div class="metric-row"><div class="metric-label">Sensação Térmica:</div><div id="sensacao-termica" class="metric-value">--</div></div>
                <div class="metric-row"><div class="metric-label">Ponto de Orvalho:</div><div id="ponto-orvalho" class="metric-value">--</div></div>
                <div class="metric-row"><div class="metric-label">Umidade Relativa:</div><div id="umidade-relativa" class="metric-value">--</div></div>
                <div class="stats-row">
                    <div class="stat-box"><div class="stat-label">MAX</div><div id="max-temp" class="stat-value">--</div></div>
                    <div class="stat-box"><div class="stat-label">MIN</div><div id="min-temp" class="stat-value">--</div></div>
                    <div class="stat-box"><div class="stat-label">AVG</div><div id="avg-temp" class="stat-value">--</div></div>
                </div>
            </div>

            <div class="panel">
                <div class="panel-title">Informações do Sistema</div>
                <div class="control-row" style="justify-content: center; margin-bottom: 20px;">
                    <button id="toggle-btn" class="toggle-btn" onclick="toggleCooler()">LIGAR SISTEMA</button>
                </div>
                <div class="control-row"><div class="control-label">RPM Atual:</div><div id="rpm" class="rpm-display">0 RPM</div></div>
                <div class="control-row" id="msg-row"><div class="control-label">Mensagem:</div><div id="status-msg" class="metric-value">Aguardando...</div></div>
                <div class="control-row"><div class="control-label">Status do Sistema:</div><div id="status-indicator" class="status-indicator"></div></div>
            </div>
        </div>
        
        <div class="footer">
            <div id="uptime-display" class="uptime">Tempo Ativo: Calculando...</div>
            <div class="timestamp">--:--:--</div>
        </div>
    </div>
    <script src="script.js"></script>
</body>
</html>
)rawliteral";

// ================= CSS (ESTILOS) =================
const char style_css[] PROGMEM = R"rawliteral(
* { margin: 0; padding: 0; box-sizing: border-box; }
body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%); color: #ffffff; padding: 20px; min-height: 100vh; }
.container { max-width: 1200px; margin: 0 auto; }
.header { display: flex; align-items: center; margin-bottom: 30px; gap: 15px; }
.logo { font-size: 48px; }
.title { font-size: 48px; font-weight: bold; letter-spacing: 2px; }
.location { text-align: right; font-size: 32px; font-weight: 600; margin-bottom: 20px; }
.main-content { display: flex; gap: 30px; align-items: stretch; }
.panel { background: rgba(48, 56, 89, 0.6); border-radius: 20px; padding: 30px; backdrop-filter: blur(10px); display: flex; flex-direction: column; gap: 20px; flex: 1; }
.panel-title { font-size: 28px; font-weight: 600; text-align: center; margin-bottom: 10px; }
.toggle-btn { width: 100%; padding: 15px; font-size: 18px; font-weight: 700; border: none; border-radius: 12px; cursor: pointer; transition: all .3s ease; text-transform: uppercase; }
.btn-on { background-color: #ef4444; color: #fff; box-shadow: 0 0 15px rgba(239,68,68,.4); }
.btn-off { background-color: #3b82f6; color: #fff; }
.btn-on:hover { background-color: #dc2626; }
.btn-off:hover { background-color: #2563eb; }
#msg-row { flex-direction: column; align-items: flex-start; gap: 10px !important; margin: 10px 0 10px 0 !important; }
#status-msg { white-space: normal; word-break: break-word; width: 100%; padding: 14px 20px; border-radius: 12px; background: #2d3561; font-size: 18px; }
.status-indicator { width: 20px; height: 20px; border-radius: 50%; }
.status-indicator.ok { background: #4ade80; box-shadow: 0 0 10px #4ade80; }
.status-indicator.warn { background: #facc15; box-shadow: 0 0 10px #facc15; }
.status-indicator.error { background: #ef4444; box-shadow: 0 0 10px #ef4444; }
.metric-row, .control-row { display: flex; justify-content: space-between; align-items: center; }
.metric-label, .control-label { font-size: 20px; font-weight: 500; }
.metric-value { background: #2d3561; padding: 10px 25px; border-radius: 25px; font-size: 20px; font-weight: 600; min-width: 120px; text-align: center; }
.rpm-display { background: #2d3561; padding: 10px 25px; border-radius: 25px; font-size: 20px; font-weight: 700; text-align: center; }
.stats-row { display: grid; grid-template-columns: repeat(3,1fr); gap: 15px; margin-top: 10px; padding-top: 20px; border-top: 2px solid rgba(255,255,255,.1); }
.stat-box { text-align: center; }
.stat-label { font-size: 16px; font-weight: 600; margin-bottom: 8px; }
.stat-value { font-size: 22px; font-weight: 700; }
.footer { display: flex; justify-content: space-between; align-items: center; margin-top: 30px; padding-top: 15px; border-top: 1px solid rgba(255,255,255,.1); }
.uptime { font-size: 18px; color: #4ade80; font-weight: 600; }
.timestamp { font-size: 18px; opacity: 0.8; font-weight: 600; }
@media (max-width:900px){ body{padding:10px} .main-content{flex-direction:column;gap:20px} .panel{padding:20px;gap:15px} .title{font-size:36px} .location{font-size:22px;text-align:left} .metric-label,.control-label{font-size:16px} .metric-value,.rpm-display{font-size:16px;min-width:auto} .stats-row{grid-template-columns:1fr 1fr 1fr;gap:10px} }
@media (max-width:480px){ .metric-row,.control-row{flex-direction:column;align-items:flex-start;gap:6px} .metric-value{width:100%} .footer{flex-direction:column;gap:10px;text-align:center} }
)rawliteral";

// ================= JAVASCRIPT =================
const char script_js[] PROGMEM = R"rawliteral(
let dados = {};
document.addEventListener("DOMContentLoaded", initApp);
function initApp() {
    atualizarDados();
    setInterval(() => { fetchData(); atualizarData(); }, 1000);
}
function fetchData() {
    fetch("/status").then(r => r.json()).then(d => {
        dados = d; atualizarDados(); atualizarStatus(); atualizarBotao();
    }).catch(e => console.error(e));
}
function toggleCooler() { fetch("/toggle").then(() => fetchData()); }
function atualizarBotao() {
    const btn = document.getElementById("toggle-btn");
    if (dados.masterSwitch) {
        btn.textContent = "SISTEMA ATIVADO (AUTO)"; btn.className = "toggle-btn btn-on";
    } else {
        btn.textContent = "SISTEMA DESLIGADO"; btn.className = "toggle-btn btn-off";
    }
}
function atualizarDados() {
    if (!dados.atual) return;
    const metricas = {
        "temp-atual": `${dados.atual.toFixed(1)}° C`, "temp-externa": `${dados.externa.toFixed(1)}° C`,
        "sensacao-termica": `${dados.sensacao.toFixed(1)}° C`, "ponto-orvalho": `${dados.orvalho.toFixed(1)}° C`,
        "umidade-relativa": `${dados.umidade.toFixed(1)} %`, "max-temp": `${dados.max.toFixed(1)}° C`,
        "avg-temp": `${dados.avg.toFixed(1)}° C`, "min-temp": `${dados.min.toFixed(1)}° C`, "rpm": `${dados.rpm_value} RPM`,
    };
    for (const [id, valor] of Object.entries(metricas)) {
        const el = document.getElementById(id); if (el) el.textContent = valor;
    }
    const upEl = document.getElementById("uptime-display");
    if (upEl && dados.uptime) upEl.textContent = "Tempo Ativo: " + dados.uptime;
}
function atualizarStatus() {
    const ind = document.getElementById("status-indicator");
    const msg = document.getElementById("status-msg");
    if (!ind || !msg) return;
    if (!dados.masterSwitch) {
        ind.className = "status-indicator warn"; msg.textContent = "Sistema Desligado Manualmente.";
    } else if (dados.atual >= 35) {
        ind.className = "status-indicator error"; msg.textContent = "Temperatura Alta! Cooler Refrigerando.";
    } else {
        ind.className = "status-indicator ok"; msg.textContent = "Temperatura Estável. Cooler em desligando em 30cº.";
    }
}
function atualizarData() {
    const now = new Date(); document.querySelector(".timestamp").textContent = now.toLocaleDateString() + " - " + now.toLocaleTimeString();
}
)rawliteral";

// ================= LÓGICA DE HARDWARE =================

void atualizarHardware() {
    // 1. Se estiver desligado manualmente
    if (!masterSwitch) {
        ledcWrite(PIN_TRANSISTOR, 0);
        rpmDisplay = 0;
        return;
    }

    // 2. Lógica com Histerese (Estável)
    // SÓ LIGA se passar de 35
    if (tempAtual >= 35.0) {
        ledcWrite(PIN_TRANSISTOR, 255); // Liga 100%
        rpmDisplay = 3000;
    } 
    // SÓ DESLIGA se cair abaixo de 30
    else if (tempAtual <= 30.0) {
        ledcWrite(PIN_TRANSISTOR, 0); 
        rpmDisplay = 0;
    }
    // OBS: Entre 30.1 e 34.9 ele não faz nada (mantém o que estava)
}

float calculateDewPoint(float temp, float hum) {
  float a = 17.27, b = 237.7;
  float alpha = ((a * temp) / (b + temp)) + log(hum / 100.0);
  return (b * alpha) / (a - alpha);
}

void readSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  
  if (!isnan(t) && !isnan(h)) {
    tempAtual = t; humAtual = h;
    if (t > tempMax) tempMax = t;
    if (t < tempMin) tempMin = t;
    tempSoma += t; leiturasCount++;
    atualizarHardware();
  }
}

// ================= MQTT =================
void reconnectMQTT() {
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    String clientId = "FROST-ESP32-System"; 
    if (client.connect(clientId.c_str())) {
      client.publish(mqtt_topic_status, "ESP32 Online");
      client.subscribe(mqtt_topic_set);
    }
  }
}

void publishData() {
  if (client.connected()) {
    String msg = "{\"temp\":" + String(tempAtual) + ", \"hum\":" + String(humAtual) + ", \"rpm\":" + String(rpmDisplay) + "}";
    client.publish(mqtt_topic_status, msg.c_str());
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
    // Callback vazia
}

// ================= ROTAS WEB =================
void handleRoot() { server.send(200, "text/html", index_html); }
void handleCss() { server.send(200, "text/css", style_css); }
void handleJs() { server.send(200, "application/javascript", script_js); }

void handleStatus() {
  float heatIndex = dht.computeHeatIndex(tempAtual, humAtual, false);
  float dewPoint = calculateDewPoint(tempAtual, humAtual);
  float avg = (leiturasCount > 0) ? (tempSoma / leiturasCount) : 0;

  unsigned long now = millis();
  unsigned long seconds = now / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  
  String uptimeStr = String(days) + "d " + String(hours % 24) + "h " + String(minutes % 60) + "m " + String(seconds % 60) + "s";

  String json = "{";
  json += "\"atual\":" + String(tempAtual) + ",";
  json += "\"externa\":" + String(tempAtual) + ","; 
  json += "\"umidade\":" + String(humAtual) + ",";
  json += "\"sensacao\":" + String(heatIndex) + ",";
  json += "\"orvalho\":" + String(dewPoint) + ",";
  json += "\"max\":" + String(tempMax) + ",";
  json += "\"min\":" + String(tempMin) + ",";
  json += "\"avg\":" + String(avg) + ",";
  json += "\"rpm_value\":" + String(rpmDisplay) + ",";
  json += "\"masterSwitch\":" + String(masterSwitch ? "true" : "false") + ",";
  json += "\"uptime\":\"" + uptimeStr + "\""; 
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleToggle() {
    masterSwitch = !masterSwitch; 
    atualizarHardware(); 
    server.send(200, "text/plain", "OK");
}

void handleCaptivePortal() {
  server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
  server.send(302, "text/plain", "");   
}

// ================= SETUP E LOOP =================
void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // PWM padrão (estável)
  ledcAttach(PIN_TRANSISTOR, 25000, 8); 
  ledcWrite(PIN_TRANSISTOR, 0); 

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, ap_password);
  WiFi.begin(home_ssid, home_password);
  
  delay(100); 

  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  server.on("/", handleRoot);
  server.on("/style.css", handleCss);
  server.on("/script.js", handleJs);
  server.on("/status", handleStatus);
  server.on("/toggle", handleToggle); 
  server.onNotFound(handleCaptivePortal);

  server.begin();
}

unsigned long lastRead = 0;

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  if (WiFi.status() == WL_CONNECTED) {
     if (!client.connected()) {
         static unsigned long lastMqttAttempt = 0;
         if (millis() - lastMqttAttempt > 5000) {
             lastMqttAttempt = millis();
             reconnectMQTT();
         }
     } else {
         client.loop(); 
     }
  }

  if (millis() - lastRead > 2000) {
    lastRead = millis();
    readSensor();
    publishData();
  }
}