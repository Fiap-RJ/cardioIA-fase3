# FIAP - Faculdade de Informática e Administração Paulista

<p align="center">
<a href= "https://www.fiap.com.br/"><img src="assets/logo-fiap.png" alt="FIAP - Faculdade de Informática e Admnistração Paulista" border="0" width=40% height=40%></a>
</p>

<br>

# CardioIA: A Nova Era da Cardiologia Inteligente

## Nome do grupo

## 👨‍🎓 Integrantes: 
- <a href="https://www.linkedin.com/in/arthur-alentejo">Arthur Guimarães Alentejo</a>
- <a href="https://www.linkedin.com/in/michaelrodriguess">Michael Rodrigues</a>
- <a href="https://www.linkedin.com/in/nathalia-vasconcelos-18a390292/">Nathalia Vasconcelos</a> 

## 👩‍🏫 Professores:
### Tutor(a) 
- <a href="https://www.linkedin.com/company/inova-fusca">Lucas Gomes Moreira</a>
### Coordenador(a)
- <a href="https://www.linkedin.com/company/inova-fusca">Andre Godoi</a>
## 📜 Descrição
O **CardioIA** é um ecossistema de saúde digital projetado para modernizar o monitoramento cardiológico através da integração de IoT, Inteligência Artificial e Data Science. O projeto simula o ciclo completo de assistência médica inteligente, desde a captura de sinais vitais na "borda" (Edge Computing) até a análise avançada de dados na nuvem (Cloud Computing).

Nesta fase, o foco é o monitoramento contínuo e a resiliência de dados, garantindo que o fluxo de informações entre sensores médicos e dashboards de visualização seja seguro, ininterrupto e escalável.

---

## 🚀 Entregas da Fase 2 (Monitoramento Contínuo)

### 1. Monitoramento IoT (Wokwi)
Protótipo funcional desenvolvido no simulador Wokwi utilizando uma placa **ESP32** integrada aos seguintes componentes:
*   **Sensor DHT22:** Responsável pela leitura de temperatura e umidade.
*   **Potenciômetro:** Simulação de frequência cardíaca (BPM) variável para testes de estresse do sistema.
*   **Conectividade:** Integração com Wi-Fi simulado e protocolo MQTT.
*   **Link do Projeto:** [Wokwi CardioIA](https://wokwi.com/projects/463863987113416705)

### 2. Infraestrutura e Cloud
Implementação de comunicação segura via Broker **HiveMQ Cloud**, utilizando protocolos criptografados para garantir a privacidade dos dados de saúde.
*   **Broker:** 32a82037c8d6424aa6cdebc46d9fb3f6.s1.eu.hivemq.cloud
*   **Porta:** 8883 (MQTTS - TLS/SSL)
*   **Tópicos Ativos:**
    * `cardioia/sensor/temperatura`: Telemetria térmica em tempo real.
    * `cardioia/sensor/batimentos`: Monitoramento de BPM.
    * `cardioia/status/conexao`: Monitoramento de disponibilidade do dispositivo (LWT).

### 3. Edge Computing e Resiliência
Desenvolvimento de lógica de contingência para cenários offline. O sistema detecta a perda de conectividade e retém os dados em um buffer local, realizando o "dump" e a sincronização automática assim que o link com a nuvem é restabelecido, prevenindo lacunas no histórico clínico do paciente.

### 4. Dashboard de Visualização (Node-RED)
Interface interativa para acompanhamento médico contendo:
*   **Gráficos de Linha:** Histórico de batimentos.
*   **Gauges:** Visualização instantânea do estado atual do paciente.
*   **Alertas Visuais:** Notificações automáticas baseadas em limiares críticos (ex: Temperatura > 37.5°C ou BPM fora da faixa normal).

---

## 🛠️ Tecnologias Utilizadas
*   **C++/Arduino:** Programação do firmware do ESP32.
*   **MQTT:** Protocolo de comunicação leve para IoT.
*   **HiveMQ Cloud:** Broker MQTT gerenciado.
*   **Node-RED:** Orquestração de fluxos e interface de usuário.
*   **Wokwi:** Simulação de hardware e sensores.

## Estrutura de pastas

```text
cardioIA-fase3/
├── README.md
├── assets
│   └── logo-fiap.png
├── document
│   └── edge-computing.md
├── src
│   ├── iot-wokwi
│   │   ├── diagram.json
│   │   ├── libraries.txt
│   │   └── main.cpp
│   └── readme.md
```
