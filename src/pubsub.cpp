#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "DHTesp.h"

// --- Configurações do Broker Privado (Arthur) ---
const char* SSID = "Wokwi-GUEST";
const char* PASSWORD = "";
const char* MQTT_SERVER = "32a82037c8d6424aa6cdebc46d9fb3f6.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;
const char* MQTT_USER = "cardio-ia";
const char* MQTT_PASS = "@h9ziZcRszQ7EBT";
const char* CLIENT_ID = "CardioIA_Arthur_Pro";

// --- Pinos e Sensores ---
const int PIN_DHT = 15;
const int PIN_BPM = 34; 
DHTesp dhtSensor;

WiFiClientSecure espClient;
PubSubClient client(espClient);

// --- Lógica de Resiliência ---
String dataBuffer = ""; 
bool isOnline = false;

void setup_wifi() {
  Serial.print("\nConectando ao WiFi...");
  WiFi.begin(SSID, PASSWORD);
  
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter < 15) {
    delay(500);
    Serial.print(".");
    counter++;
  }

  if(WiFi.status() == WL_CONNECTED) {
    isOnline = true;
    Serial.println("\nWiFi OK!");
    // Configura o cliente seguro para o HiveMQ Cloud
    espClient.setInsecure(); 
  } else {
    isOnline = false;
    Serial.println("\nIniciando em modo Offline (Resiliência)");
  }
}

void reconnect() {
  while (!client.connected() && WiFi.status() == WL_CONNECTED) {
    Serial.print("Tentando conexão MQTT Privada...");
    if (client.connect(CLIENT_ID, MQTT_USER, MQTT_PASS)) {
      Serial.println("Conectado com Sucesso!");
      if (dataBuffer.length() > 0) {
        Serial.println("Descarregando buffer de resiliência...");
        client.publish("cardioia/sinais/resiliencia", dataBuffer.c_str());
        dataBuffer = "";
      }
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5s");
      delay(5000);
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
  if (WiFi.status() != WL_CONNECTED) {
    isOnline = false;
  } else {
    isOnline = true;
    if (!client.connected()) reconnect();
  }

  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  int bpmRaw = analogRead(PIN_BPM);
  int bpm = map(bpmRaw, 0, 4095, 40, 180);

  // Formata o JSON
  String payload = "{\"temperatura\":" + String(data.temperature) + 
                   ",\"umidade\":" + String(data.humidity) + 
                   ",\"bpm\":" + String(bpm) + "}";

  if (isOnline && client.connected()) {
    client.loop();
    client.publish("cardioia/sinais/tempo_real", payload.c_str());
    Serial.println("Enviado para Cloud: " + payload);
  } else {
    // Edge Computing: Salva no buffer caso esteja offline
    dataBuffer += payload + " | "; 
    Serial.println("Offline - Armazenado no Buffer: " + payload);
  }

  delay(5000); 
}