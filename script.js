//Espera o html carregar
document.addEventListener('DOMContentLoaded', initApp);


// Função principal
function initApp() {


const saveButton = document.querySelector('.save-button');
const resetButton = document.querySelector('.reset-button');

// Eventos
saveButton.addEventListener('click', handleSave);
resetButton.addEventListener('click', handleReset);

// Simulação inicial 
//TODO substituir por leitura do ESP32 futuramente)
const dadosSimulados = {
    atual: 32,
    externa: 35,
    sensacao: 35,
    orvalho: 10,
    umidade: 2,
    max: 30,
    avg: 20,
    min: 10,
    rpm: 3000
};

atualizarDados(dadosSimulados);
atualizarData();
}


// Manipuladores de evento

function handleSave() {
    // TODO: salvar alterações no backend / ESP32
    alert('Configurações salvas com sucesso!');
}

function handleReset() {
    if (confirm('Deseja realmente restaurar as configurações de fábrica?')) {
        // TODO: restaurar para as configurações padrão
        alert('Configurações restauradas!');
    }
}

function handleGet() {
    // TODO: acessa e define os componentes com as escolhas já salvas

    //mode-dropdown
    //power-dropdown
    //temp-dropdown
}


// Atualização de UI
function atualizarDados(dados) {
    const metricas = {
        "temp-atual": `${dados.atual}° C`,
        "temp-externa": `${dados.externa}° C`,
        "sensacao-termica": `${dados.sensacao}° C`,
        "ponto-orvalho": `${dados.orvalho}° C`,
        "umidade-relativa": `${dados.umidade} %`,
        "max-temp": `${dados.max}° C`,
        "avg-temp": `${dados.avg}° C`,
        "min-temp": `${dados.min}° C`,
        "rpm": `${dados.rpm} RPM`
    };

    for (const [id, valor] of Object.entries(metricas)) {
        const elemento = document.getElementById(id);
        if (elemento) elemento.textContent = valor;
    }
}

function atualizarData() {
    const now = new Date()

    const year = now.getFullYear();
    const month = now.getMonth() + 1; // jan = 0, logo +1
    const day = now.getDate();
    const hours = now.getHours();
    const minutes = now.getMinutes();

    const frase = ` ${day}/${month}/${year} - ${hours}:${minutes}`;

    const elemento = document.querySelector('.timestamp');
    if (elemento) elemento.textContent = frase;
    
}
