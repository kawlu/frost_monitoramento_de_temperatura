#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <DNSServer.h>
#include <PubSubClient.h> // <--- NOVA BIBLIOTECA MQTT

// ================= CONFIGURAÇÕES DA REDE (DUPLA) =================
// 1. Configuração do AP (A rede que o ESP cria)
const char* ap_ssid = "FROST_SYSTEM"; 
const char* ap_password = "12345678"; 

// 2. Configuração do Wi-Fi de Casa (Para o MQTT funcionar)
const char* home_ssid = "Reis-TIM"; //nome do wi-fi que ele vai se conectar 
const char* home_password = "santanareis"; // senha

// 3. Configurações MQTT
const char* mqtt_server = "192.168.1.100"; // <--- IP DO SEU BROKER MQTT
const int mqtt_port = 1883;
const char* mqtt_topic_status = "frost/status";
const char* mqtt_topic_set = "frost/set"; // Para receber comandos via MQTT

// Configurações do DNS (Portal Cativo)
const byte DNS_PORT = 53;
DNSServer dnsServer;

// ================= HARDWARE =================
#define DHTPIN 4
#define DHTTYPE DHT22 
#define PIN_TRANSISTOR 18  // Pino Base do Transistor
#define CANAL_PWM 0       

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);

// ================= VARIÁVEIS GLOBAIS =================
float tempAtual = 0, humAtual = 0;
float tempMax = -100, tempMin = 100, tempSoma = 0;
long leiturasCount = 0;

String modoOperacao = "Manual";
String targetTemp = "25";
String rpmMode = "Médio";
int rpmValue = 0;

// ================= HTML / CSS / JS (MINIFICADO PARA ECONOMIZAR ESPAÇO) =================
// Mantive o mesmo HTML do seu código anterior, apenas referenciando as variáveis PROGMEM
// (Assumindo que você já tem o conteúdo index_html, style_css e script_js do passo anterior)
// Vou omitir aqui para focar na lógica C++, mas imagine que estão aqui.
const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE html><html lang="pt-BR"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0"><title>FROST</title><link rel="stylesheet" href="style.css"></head><body><div class="container"><div class="header"><div class="logo">❄️</div><div class="title">FROST</div></div><div id="localization" class="location">Modo Híbrido (AP + MQTT)</div><div class="main-content"><div class="panel"><div class="panel-title">Temperatura</div><div class="metric-row"><div class="metric-label">Temp. Atual:</div><div id="temp-atual" class="metric-value">--</div></div><div class="metric-row"><div class="metric-label">Umidade:</div><div id="umidade-relativa" class="metric-value">--</div></div><div class="metric-row"><div class="metric-label">Sensação:</div><div id="sensacao-termica" class="metric-value">--</div></div><div class="metric-row"><div class="metric-label">Orvalho:</div><div id="ponto-orvalho" class="metric-value">--</div></div><div class="stats-row"><div class="stat-box"><div class="stat-label">MAX</div><div id="max-temp" class="stat-value">--</div></div><div class="stat-box"><div class="stat-label">MIN</div><div id="min-temp" class="stat-value">--</div></div><div class="stat-box"><div class="stat-label">AVG</div><div id="avg-temp" class="stat-value">--</div></div></div></div><div class="panel"><div class="panel-title">Resfriamento</div><div class="control-row"><div class="control-label">Modo:</div><select id="mode-dropdown" class="dropdown"><option>Automático</option><option>Manual</option></select></div><div class="control-row"><div class="control-label">Acionar em:</div><select id="temp-dropdown" class="dropdown"><option value="40">40°C</option><option value="35">35°C</option><option value="30">30°C</option><option value="25">25°C</option></select></div><div class="control-row"><div class="control-label">Potência:</div><select id="rpm_power-dropdown" class="dropdown"><option>Automático</option><option>Máximo</option><option>Médio</option><option>Mínimo</option></select></div><div class="control-row"><div class="control-label">RPM (Virtual):</div><div id="rpm" class="rpm-display">0 RPM</div></div><button class="reset-button" onclick="handleReset()">Resetar Stats</button><button class="save-button" onclick="handleSave()">Salvar</button></div></div><div class="timestamp"></div></div><script src="script.js"></script></body></html>)rawliteral";
const char style_css[] PROGMEM = R"rawliteral(*{margin:0;padding:0;box-sizing:border-box}body{font-family:'Segoe UI',sans-serif;background:linear-gradient(135deg,#1a1a2e 0%,#16213e 100%);color:#fff;padding:10px;min-height:100vh}.container{max-width:1200px;margin:0 auto}.header{display:flex;align-items:center;margin-bottom:20px;gap:10px;justify-content:center}.logo,.title{font-size:32px;font-weight:bold}.location{text-align:center;font-size:18px;margin-bottom:15px;opacity:.8}.main-content{display:grid;grid-template-columns:1fr;gap:20px}@media (min-width:768px){.main-content{grid-template-columns:1fr 1fr}}.panel{background:rgba(48,56,89,.6);border-radius:15px;padding:15px;backdrop-filter:blur(10px)}.panel-title{font-size:20px;font-weight:600;margin-bottom:15px;text-align:center;border-bottom:1px solid rgba(255,255,255,.1);padding-bottom:10px}.metric-row,.control-row{display:flex;justify-content:space-between;align-items:center;margin-bottom:12px}.metric-value{background:#2d3561;padding:5px 10px;border-radius:15px;min-width:80px;text-align:center;font-weight:bold;font-size:14px}.stats-row{display:grid;grid-template-columns:repeat(3,1fr);gap:5px;margin-top:15px;border-top:1px solid rgba(255,255,255,.1);padding-top:10px}.stat-box{text-align:center}.stat-label{font-size:10px;opacity:.7}.dropdown{padding:5px;border-radius:5px;min-width:100px;font-size:14px}.save-button{background:#4ade80;color:#1a1a2e;border:none;padding:12px;width:100%;border-radius:10px;font-weight:bold;cursor:pointer;margin-top:10px;font-size:16px}.reset-button{background:transparent;color:#fff;border:1px solid #fff;padding:8px;width:100%;border-radius:10px;cursor:pointer;margin-top:10px;font-size:12px}.timestamp{text-align:center;margin-top:20px;font-size:12px;opacity:.5})rawliteral";
const char script_js[] PROGMEM = R"rawliteral(document.addEventListener('DOMContentLoaded',initApp);function initApp(){document.getElementById("mode-dropdown").addEventListener("change",toggleDropdown);toggleDropdown();setInterval(fetchData,2000);setInterval(updateClock,1000);fetchData()}function toggleDropdown(){let e="Automático"===document.getElementById("mode-dropdown").value;document.getElementById("temp-dropdown").disabled=e,document.getElementById("rpm_power-dropdown").disabled=e}function updateClock(){const e=new Date;document.querySelector('.timestamp').textContent=e.toLocaleDateString()+' - '+e.toLocaleTimeString()}function fetchData(){fetch('/status').then(e=>e.json()).then(e=>{document.getElementById("temp-atual").innerHTML=e.atual.toFixed(1)+"°C",document.getElementById("umidade-relativa").innerHTML=e.umidade.toFixed(1)+"%",document.getElementById("sensacao-termica").innerHTML=e.sensacao.toFixed(1)+"°C",document.getElementById("ponto-orvalho").innerHTML=e.orvalho.toFixed(1)+"°C",document.getElementById("max-temp").innerHTML=e.max.toFixed(1)+"°C",document.getElementById("min-temp").innerHTML=e.min.toFixed(1)+"°C",document.getElementById("avg-temp").innerHTML=e.avg.toFixed(1)+"°C",document.getElementById("rpm").innerHTML=e.rpm+" RPM"}).catch(e=>console.error(e))}function handleSave(){const e=document.getElementById("mode-dropdown").value,t=document.getElementById("temp-dropdown").value,n=document.getElementById("rpm_power-dropdown").value;fetch(`/save?mode=${e}&target=${t}&rpm=${n}`).then(e=>{e.ok&&alert('Salvo no ESP32!')})}function handleReset(){confirm('Resetar Stats?')&&fetch('/reset').then(()=>alert('Resetado!'))})rawliteral";


// ================= FUNÇÕES AUXILIARES =================

// Controle do Transistor (Hardware)
void aplicarVelocidadeNoMotor(int rpmVirtual) {
    int dutyCycle = 0;
    if (rpmVirtual >= 3000) dutyCycle = 255;      
    else if (rpmVirtual >= 1500) dutyCycle = 180; 
    else if (rpmVirtual >= 800) dutyCycle = 100;  
    else dutyCycle = 0;                           
    
    // Envia sinal para o transistor
    ledcWrite(CANAL_PWM, dutyCycle); 
}

float calculateDewPoint(float temp, float hum) {
  float a = 17.27, b = 237.7;
  float alpha = ((a * temp) / (b + temp)) + log(hum / 100.0);
  return (b * alpha) / (a - alpha);
}

// Leitura dos dados + Controle
void readSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  
  if (!isnan(t) && !isnan(h)) {
    tempAtual = t; humAtual = h;
    if (t > tempMax) tempMax = t;
    if (t < tempMin) tempMin = t;
    tempSoma += t; leiturasCount++;
    
    // Lógica Automática
    if (modoOperacao == "Automático") {
      if (t > 28) rpmValue = 3000; 
      else if (t > 25) rpmValue = 1500; 
      else rpmValue = 0;
      aplicarVelocidadeNoMotor(rpmValue); // Aplica no Transistor
    }
  }
}

// ================= LÓGICA MQTT =================

void callback(char* topic, byte* payload, unsigned int length) {
  // Aqui você pode receber comandos via MQTT no futuro
  // Ex: Se receber "LIGAR" no topico frost/set, muda o modo.
}

void reconnectMQTT() {
  // Tenta conectar se o Wi-Fi de casa estiver OK
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    Serial.print("Tentando conectar MQTT...");
    if (client.connect("FROST_ESP32_CLIENT")) {
      Serial.println("Conectado!");
      client.subscribe(mqtt_topic_set);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
    }
  }
}

void publishData() {
  if (client.connected()) {
    // Cria um JSON simples para enviar
    String msg = "{\"temp\":" + String(tempAtual) + 
                 ", \"hum\":" + String(humAtual) + 
                 ", \"rpm\":" + String(rpmValue) + "}";
    client.publish(mqtt_topic_status, msg.c_str());
  }
}

// ================= ROTAS WEB (IGUAIS) =================
void handleRoot() { server.send(200, "text/html", index_html); }
void handleCss() { server.send(200, "text/css", style_css); }
void handleJs() { server.send(200, "application/javascript", script_js); }
void handleCaptivePortal() {
  server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
  server.send(302, "text/plain", "");   
}

void handleStatus() {
  float heatIndex = dht.computeHeatIndex(tempAtual, humAtual, false);
  float dewPoint = calculateDewPoint(tempAtual, humAtual);
  float avg = (leiturasCount > 0) ? (tempSoma / leiturasCount) : 0;
  
  String json = "{\"atual\":" + String(tempAtual) + ",\"umidade\":" + String(humAtual) + 
                ",\"sensacao\":" + String(heatIndex) + ",\"orvalho\":" + String(dewPoint) + 
                ",\"max\":" + String(tempMax) + ",\"min\":" + String(tempMin) + 
                ",\"avg\":" + String(avg) + ",\"rpm\":" + String(rpmValue) + "}";
  server.send(200, "application/json", json);
}

void handleSave() {
  if (server.hasArg("mode")) modoOperacao = server.arg("mode");
  if (server.hasArg("target")) targetTemp = server.arg("target");
  if (server.hasArg("rpm")) rpmMode = server.arg("rpm");
  
  if (modoOperacao == "Manual") {
     if (rpmMode == "Máximo") rpmValue = 3000;
     else if (rpmMode == "Médio") rpmValue = 1500;
     else if (rpmMode == "Mínimo") rpmValue = 800;
     else rpmValue = 0;
     
     aplicarVelocidadeNoMotor(rpmValue); // Aplica no Transistor
  }
  server.send(200, "text/plain", "OK");
}

void handleResetStats() {
  tempMax = -100; tempMin = 100; tempSoma = 0; leiturasCount = 0;
  server.send(200, "text/plain", "OK");
}

// ================= SETUP E LOOP =================

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Configuração do PWM (Transistor)
  // ledcSetup(CANAL_PWM, 25000, 8);
  // ledcAttachPin(PIN_TRANSISTOR, CANAL_PWM);

  // 1. Inicia Wi-Fi em Modo DUPLO (AP + Station)
  WiFi.mode(WIFI_AP_STA);
  
  // Configura o AP (Sua rede local FROST)
  WiFi.softAP(ap_ssid, ap_password);
  
  // Conecta ao Roteador (Para internet/MQTT)
  WiFi.begin(home_ssid, home_password);
  
  delay(100); 

  IPAddress IP_AP = WiFi.softAPIP();
  Serial.print("AP Iniciado: "); Serial.println(IP_AP);
  
  // Configura DNS (Captive Portal)
  dnsServer.start(DNS_PORT, "*", IP_AP);

  // Configura Servidor MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Rotas Web
  server.on("/", handleRoot);
  server.on("/style.css", handleCss);
  server.on("/script.js", handleJs);
  server.on("/status", handleStatus);
  server.on("/save", handleSave);
  server.on("/reset", handleResetStats);
  server.onNotFound(handleCaptivePortal);

  server.begin();
}

unsigned long lastRead = 0;
unsigned long lastMqtt = 0;

void loop() {
  // Lógica Web e DNS
  dnsServer.processNextRequest();
  server.handleClient();

  // Lógica MQTT (Tenta reconectar se caiu)
  if (!client.connected()) {
     // Só tenta reconectar ao MQTT a cada 5 segundos para não travar o loop
     static unsigned long lastReconnectAttempt = 0;
     if (millis() - lastReconnectAttempt > 5000) {
       lastReconnectAttempt = millis();
       if (WiFi.status() == WL_CONNECTED) {
          reconnectMQTT();
       }
     }
  }
  client.loop();

  // Leitura do Sensor (A cada 2s)
  if (millis() - lastRead > 2000) {
    lastRead = millis();
    readSensor();
    
    // Envia MQTT (A cada leitura do sensor)
    publishData();
  }
}