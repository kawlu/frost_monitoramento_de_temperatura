// Simulação inicial
//TODO substituir por leitura do ESP32 futuramente)
dados = {
  atual: 0,
  externa: 0,
  sensacao: 0,
  orvalho: 0,
  umidade: 0,
  max: 0,
  avg: 0,
  min: 0,
  rpm_value: 3000,
};

// Espera o html carregar
document.addEventListener("DOMContentLoaded", initApp);

// Função principal
function initApp() {
  atualizarDados();
  atualizarData();

  // Atualiza a cada 1 segundo
  setInterval(() => {
    atualizarDados();
    atualizarStatus();
    atualizarData();
  }, 1000);

}

// Atualização de UI
function atualizarDados() {
  const metricas = {
    "temp-atual": `${dados.atual}° C`,
    "temp-externa": `${dados.externa}° C`,
    "sensacao-termica": `${dados.sensacao}° C`,
    "ponto-orvalho": `${dados.orvalho}° C`,
    "umidade-relativa": `${dados.umidade} %`,
    "max-temp": `${dados.max}° C`,
    "avg-temp": `${dados.avg}° C`,
    "min-temp": `${dados.min}° C`,
    rpm: `${dados.rpm_value} RPM`,
  };
  //Carrega dados sobre temperatura e valor de rpm
  for (const [id, valor] of Object.entries(metricas)) {
    const elemento = document.getElementById(id);
    if (elemento) elemento.textContent = valor;
  }
}

function atualizarData() {
  const now = new Date();

  const frase = now.toLocaleDateString() + " - " + now.toLocaleTimeString();

  const elemento = document.querySelector(".timestamp");
  if (elemento) elemento.textContent = frase;
}

function atualizarStatus() {
  const indicator = document.getElementById("status-indicator");
  const msg = document.getElementById("status-msg");

  if (!indicator || !msg) return;

  // TODO alterar regras corporativas de diagnóstico
  if (dados.atual >= 35) {
    indicator.className = "status-indicator error";
    msg.textContent = "Sobrecarga térmica. Cooler ativado!";
  } else if (dados.rpm_value < 500) {
    indicator.className = "status-indicator warn";
    msg.textContent = "RPM insuficiente. Eficiência degradada";
  } else {
    indicator.className = "status-indicator ok";
    msg.textContent = "Sistemas em Funcionamento Normal!";
  }
}