// Este código implementa um dispositivo IoT para monitoramento cardíaco.
// Ele conecta ao WiFi, publica dados de temperatura e batimentos cardíacos via MQTT,
// e inclui resiliência offline para armazenar dados quando desconectado.

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "DHTesp.h"

// --- Configurações do Broker Privado ---
// Configurações para conexão WiFi e MQTT. O broker é HiveMQ Cloud com autenticação.
const char* SSID = "Wokwi-GUEST";
const char* PASSWORD = "";
const char* MQTT_SERVER = "32a82037c8d6424aa6cdebc46d9fb3f6.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;
const char* MQTT_USER = "cardio-ia";
const char* MQTT_PASS = "@h9ziZcRszQ7EBT";
const char* CLIENT_ID = "CardioIA";

// --- Tópicos Definidos ---
// Tópicos MQTT para publicar dados dos sensores e status de conexão.
const char* TOPIC_TEMP = "cardioia/sensor/temperatura";
const char* TOPIC_BPM  = "cardioia/sensor/batimentos";
const char* TOPIC_STATUS = "cardioia/status/conexao";

// --- Pinos e Sensores ---
// Definição dos pinos para os sensores DHT22 e BPM (simulado via ADC).
const int PIN_DHT = 15;
const int PIN_BPM = 34; 
DHTesp dhtSensor;

// --- Objetos Globais ---
// Cliente seguro para WiFi (para MQTT over TLS).
WiFiClientSecure espClient;
// Cliente MQTT que usa o cliente WiFi seguro.
PubSubClient client(espClient);
// Buffer para armazenar dados quando offline (resiliência).
String dataBuffer = ""; 
// Flag para indicar se o dispositivo está online (conectado ao WiFi).
bool isOnline = false;

// --- Função de Configuração do WiFi ---
// Tenta conectar ao WiFi. Se falhar após 15 tentativas, inicia em modo offline.
// Define isOnline e configura o cliente seguro para ignorar certificados (inseguro para simulação).
void setup_wifi() {
  Serial.print("\nConectando ao WiFi...");
  WiFi.begin(SSID, PASSWORD);
  
  int counter = 0;
  // Loop para tentar conectar, com timeout de 15 tentativas (cerca de 7.5 segundos).
  while (WiFi.status() != WL_CONNECTED && counter < 15) {
    delay(500);
    Serial.print(".");
    counter++;
  }

  // Verifica se conectou com sucesso.
  if(WiFi.status() == WL_CONNECTED) {
    isOnline = true;
    Serial.println("\nWiFi OK!");
    // Configura o cliente para aceitar certificados auto-assinados (não seguro, apenas para simulação).
    espClient.setInsecure(); 
  } else {
    isOnline = false;
    Serial.println("\nIniciando em modo Offline");
  }
}

// --- Função de Reconexão MQTT ---
// Tenta reconectar ao broker MQTT se desconectado e WiFi estiver ok.
// Usa LWT para publicar "offline" se cair, e "online" ao conectar.
// Se havia dados no buffer offline, publica no tópico de resiliência.
void reconnect() {
  // Loop enquanto não conectado e WiFi ok.
  while (!client.connected() && WiFi.status() == WL_CONNECTED) {
    Serial.print("Tentando conexão MQTT...");
    // Conectando com LWT (Last Will and Testament) para avisar se cair
    if (client.connect(CLIENT_ID, MQTT_USER, MQTT_PASS, TOPIC_STATUS, 1, true, "offline")) {
      Serial.println("Conectado!");
      // Publica status online.
      client.publish(TOPIC_STATUS, "online", true); // Avisa que está online
      
      // Se havia dados offline, publica no tópico de resiliência.
      if (dataBuffer.length() > 0) {
        client.publish("cardioia/sensor/resiliencia", dataBuffer.c_str());
        dataBuffer = "";
      }
    } else {
      // Aguarda 5 segundos antes de tentar novamente.
      delay(5000);
    }
  }
}

// --- Função Setup ---
// Inicializa o dispositivo: serial, sensor DHT, WiFi e MQTT.
void setup() {
  // Inicializa comunicação serial para debug.
  Serial.begin(115200);
  // Configura o sensor DHT22 no pino definido.
  dhtSensor.setup(PIN_DHT, DHTesp::DHT22);
  // Tenta conectar ao WiFi.
  setup_wifi();
  // Configura o servidor MQTT no cliente.
  client.setServer(MQTT_SERVER, MQTT_PORT);
}

// --- Função Loop ---
// Loop principal: verifica conectividade, lê sensores, publica dados ou armazena offline.
// Executa a cada 5 segundos.
void loop() {
  // Verifica status do WiFi e atualiza flag online.
  if (WiFi.status() != WL_CONNECTED) {
    isOnline = false;
  } else {
    isOnline = true;
    // Se online mas MQTT desconectado, tenta reconectar.
    if (!client.connected()) reconnect();
  }

  // Lê dados do sensor DHT (temperatura e umidade).
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  // Simula leitura de BPM mapeando ADC de 0-4095 para 40-180 BPM.
  int bpm = map(analogRead(PIN_BPM), 0, 4095, 40, 180);

  // Se online e conectado ao MQTT, publica os dados.
  if (isOnline && client.connected()) {
    client.loop();
    
    // Envio segmentado por tópico
    client.publish(TOPIC_TEMP, String(data.temperature).c_str());
    client.publish(TOPIC_BPM, String(bpm).c_str());
    
    Serial.printf("Enviado -> Temp: %.2f | BPM: %d\n", data.temperature, bpm);
  } else {
    // Se offline, armazena dados no buffer em formato compacto.
    dataBuffer += "T:" + String(data.temperature) + ";B:" + String(bpm) + "|"; 
    Serial.println("Offline - Dados retidos no buffer.");
  }

  // Aguarda 5 segundos antes da próxima leitura.
  delay(5000); 
}