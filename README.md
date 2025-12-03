# ❄️ FROST — Sistema Embarcado de Monitoramento Térmico com Controle Inteligente

O **FROST** é um sistema embarcado desenvolvido para monitorar temperatura e umidade em tempo real, utilizando sensores ambientais, controle automático de resfriamento e comunicação MQTT.  
Ele inclui uma **interface web moderna**, responsiva e dinâmica, permitindo que o usuário visualize métricas em tempo real, acione ou desligue o sistema e acompanhe alertas de segurança.

---

## 📌 **Principais Funcionalidades**

- Monitoramento contínuo de **temperatura** e **umidade**
- Controle automático de **resfriamento**
- Emissão de alertas e desligamento em situações críticas
- Coleta de dados externos para ação **preditiva**
- Envio de telemetria via **MQTT**
- Interface web em tempo real (HTML, CSS e JavaScript)
- Comunicação com backend por endpoints REST
- Indicadores visuais e métricas:
  - Temperatura atual, externa, sensação térmica
  - Umidade relativa
  - Ponto de orvalho
  - Valores **mín**, **máx** e **média**
  - Velocidade do cooler (RPM)
  - Status do sistema e mensagens de alerta
  - Uptime do dispositivo

---

## 🧩 **Arquitetura Geral**

| Sistema Embarcado | MQTT | Broker | MQTT | Interface Web |

| (ESP + Sensor + Fan)| <-----> | (Mosquitto) | <-----> | HTML + CSS + JS |

| HTTP (status/toggle) |

---
