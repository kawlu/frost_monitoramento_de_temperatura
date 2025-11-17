// Simulação inicial 
//TODO substituir por leitura do ESP32 futuramente)
dados = {
    atual: 32,
    externa: 35,
    sensacao: 35,
    orvalho: 10,
    umidade: 2,
    max: 30,
    avg: 20,
    min: 10,
    rpm_value: 3000,
    modo:"Manual",     //Automático | Manual
    temp_target:"35", // 15, 20, 25 , 30, 35, 40
    rpm_mode:"Médio", // Automático, Máximo, Médio, Mínimo

};

//Espera o html carregar
document.addEventListener('DOMContentLoaded', initApp);

// Função principal
function initApp() {

const saveButton = document.querySelector('.save-button');
const resetButton = document.querySelector('.reset-button');

// Eventos
saveButton.addEventListener('click', handleSave);
resetButton.addEventListener('click', handleReset);

// Listener
document.getElementById("mode-dropdown")
    .addEventListener("change", () => toggleDropdown());

toggleDropdown();
atualizarDados();
atualizarData();

//Atualiza a cada 1 segundo
setInterval(atualizarData, 1000);
setInterval(atualizarDados, 1000)

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
}

function toggleDropdown(){
    let condition = Boolean(document.getElementById("mode-dropdown").value === "Automático")
    let dropdowns = ["temp-dropdown", "rpm_power-dropdown"]

    for (const id of dropdowns) {
        document.getElementById(id).disabled = condition;
    }

    //TODO fazer ir pro default
    
}

function atualizarConfiguracao(){
    toggleDropdown()
    //TODO rever essa prr
    if (metricas['dropdown']['mode-dropdown'] == "Manual") {

        for (const [id, valor] of Object.entries(metricas['dropdown'])) {
            const elemento = document.getElementById(id);
            if (elemento) elemento.value = valor;
        }
    }

    else {
        //coloca os valores default e impossibilita a edição dos dropdown
    }

}

// Atualização de UI
function atualizarDados() {
    const metricas = {
        "normal": {
            "temp-atual": `${dados.atual}° C`,
            "temp-externa": `${dados.externa}° C`,
            "sensacao-termica": `${dados.sensacao}° C`,
            "ponto-orvalho": `${dados.orvalho}° C`,
            "umidade-relativa": `${dados.umidade} %`,
            "max-temp": `${dados.max}° C`,
            "avg-temp": `${dados.avg}° C`,
            "min-temp": `${dados.min}° C`,
            "rpm": `${dados.rpm_value} RPM`
        },
        "dropdown": {
            "mode-dropdown":`${dados.modo}`,
            "temp-dropdown":`${dados.temp_target}`,
            "power-dropdown":`${dados.rpm_mode}`
        },
    };

    //Carrega dados sobre temperatura e valor de rpm
    for (const [id, valor] of Object.entries(metricas['normal'])) {
            const elemento = document.getElementById(id);
            if (elemento) elemento.textContent = valor;
        }
}

//TODO ver loop
function atualizarData() {
    const now = new Date()

    const frase = now.toLocaleDateString() + ' - ' + now.toLocaleTimeString() ;

    const elemento = document.querySelector('.timestamp');
    if (elemento) elemento.textContent = frase;
}