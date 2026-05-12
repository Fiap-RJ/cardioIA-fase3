// ==========================================
// Projeto CardioIA - Fase 3
// Módulo: IoT & Hardware no Wokwi
// Responsável: Michael
// ==========================================

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "DHTesp.h"

// Infraestrutura e Cloud (Herdado do Arthur)
const char* SSID = "Wokwi-GUEST";
const char* PASSWORD = "";
const char* MQTT_SERVER = "32a82037c8d6424aa6cdebc46d9fb3f6.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;
const char* MQTT_USER = "cardio-ia";
const char* MQTT_PASS = "@h9ziZcRszQ7EBT";
const char* CLIENT_ID = "CardioIA_Edge_Node";

// Hardware e Sensores
const int PIN_DHT = 15;
const int PIN_BPM = 34;

DHTesp dhtSensor;
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Edge Computing: Estado de conexão e Buffer
String dataBuffer = "";
bool isOnline = false;
unsigned long ultimoEnvio = 0;

void setup_wifi() {
  WiFi.begin(SSID, PASSWORD);
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter < 15) {
    delay(500);
    counter++;
  }
  if(WiFi.status() == WL_CONNECTED) {
    isOnline = true;
    espClient.setInsecure(); // Necessário para contornar validação SSL restrita no Wokwi
  } else {
    isOnline = false;
  }
}

void reconnect() {
  while (!client.connected() && WiFi.status() == WL_CONNECTED) {
    if (client.connect(CLIENT_ID, MQTT_USER, MQTT_PASS)) {
      // Handshake: Descarrega os dados cacheados na borda ao retomar conexão
      if (dataBuffer.length() > 0) {
        client.publish("cardioia/sinais/resiliencia", dataBuffer.c_str());
        dataBuffer = "";
      }
    } else {
      delay(5000); // Backoff para evitar flooding no broker
    }
  }
}

void setup() {
  Serial.begin(115200);
  dhtSensor.setup(PIN_DHT, DHTesp::DHT22);
  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
}

void loop() {
  // Monitoramento de Keep-Alive e Conexão
  if (WiFi.status() != WL_CONNECTED) {
    isOnline = false;
  } else {
    isOnline = true;
    if (!client.connected()) reconnect();
  }

  if (isOnline && client.connected()) client.loop();

  // Controle de amostragem não-bloqueante (5s) para não travar a thread de rede
  unsigned long tempoAtual = millis();
  if (tempoAtual - ultimoEnvio >= 5000) {
    ultimoEnvio = tempoAtual;

    // Início da aquisição de dados e formatação
    TempAndHumidity data = dhtSensor.getTempAndHumidity();
    int bpmRaw = analogRead(PIN_BPM);

    // Mapeamento linear: Converte o sinal analógico (0-4095) para range fisiológico em BPM
    int bpm = map(bpmRaw, 0, 4095, 40, 180);

    // Serialização JSON otimizada
    String payload = "{\"temperatura\":" + String(data.temperature, 2) +
                     ",\"umidade\":" + String(data.humidity, 1) +
                     ",\"bpm\":" + String(bpm) + "}";

    // Roteamento baseado no status da rede
    if (isOnline && client.connected()) {
      client.publish("cardioia/sinais/tempo_real", payload.c_str());
      Serial.println("Cloud (Real-time): " + payload);
    } else {
      // Estratégia de fallback: Retém na memória do ESP32 até a rede estabilizar
      dataBuffer += payload + " | ";
      Serial.println("Edge (Offline Cache): " + payload);
    }
  }
}