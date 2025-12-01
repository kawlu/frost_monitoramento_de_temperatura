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
