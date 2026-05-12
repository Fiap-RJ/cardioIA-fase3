# Documentação Técnica: Infraestrutura e Resiliência Edge-to-Cloud

## 1. Visão Geral da Arquitetura

A base tecnológica desta fase sustenta-se no modelo de **Computação em Camadas (Edge-Fog-Cloud)**. O objetivo é garantir que os sinais vitais coletados (Temperatura, Umidade e Frequência Cardíaca simulada) sejam transmitidos com integridade e segurança, mesmo em cenários de instabilidade de rede.

### Fluxo de Comunicação:

1. **Edge (ESP32):** Captura e processamento local dos dados.
2. **Protocolo:** MQTT (Message Queuing Telemetry Transport) sobre TLS.
3. **Cloud (HiveMQ):** Broker privado para orquestração de mensagens.
4. **Fog/Application (Node-RED):** Processamento de regras de negócio e visualização.

---

## 2. Conectividade Segura e Cloud

Para garantir a privacidade e a segurança dos dados médicos (conforme diretrizes da LGPD), a infraestrutura foi implementada utilizando um cluster privado no **HiveMQ Cloud**.

* **Endpoint:** `32a82037c8d6424aa6cdebc46d9fb3f6.s1.eu.hivemq.cloud`
* **Porta:** `8883` (MQTTS - Seguro)
* **Protocolo de Segurança:** Implementação de **TLS/SSL** para criptografia de ponta a ponta.
* **Autenticação:** Acesso restrito via credenciais exclusivas (`cardio-ia`), impedindo conexões não autorizadas ao ecossistema hospitalar.

---

## 3. Lógica de Resiliência (Edge Computing)

Dada a criticidade de um sistema cardiológico, a perda de conexão não pode resultar em perda de dados. Foi desenvolvida uma lógica de **Resiliência Offline** diretamente no firmware do dispositivo:

### Mecanismos Implementados:

* **Detecção Automática de Estado:** O sistema monitora em tempo real o status da conexão Wi-Fi e a pulsação do Broker (*Keep Alive*).
* **Modo de Cache (Contingência):** Ao detectar estado offline, o ESP32 interrompe as tentativas de publicação e inicia o armazenamento das leituras em um buffer temporário (Edge Storage).
* **Handshake de Sincronização:** No momento da reconexão, o sistema executa um procedimento de *dump* do buffer. Os dados retidos são enviados em lote para o tópico de resiliência antes de retomar o fluxo de tempo real, garantindo que o histórico médico permaneça contínuo e sem "buracos" temporais.

---

## 4. Estrutura de Dados (Payload JSON)

Os dados são transmitidos em formato **JSON**, otimizando o *parsing* para futuros modelos de Machine Learning e facilitando a integração com dashboards.

**Exemplo de Payload:**

```json
{
  "temperatura": 36.5,
  "umidade": 45.0,
  "bpm": 72
}

```

---