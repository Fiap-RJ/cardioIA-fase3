#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "DHTesp.h"

// --- Configurações do Broker Privado ---
const char* SSID = "Wokwi-GUEST";
const char* PASSWORD = "";
const char* MQTT_SERVER = "32a82037c8d6424aa6cdebc46d9fb3f6.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;
const char* MQTT_USER = "cardio-ia";
const char* MQTT_PASS = "@h9ziZcRszQ7EBT";
const char* CLIENT_ID = "CardioIA";

// --- Tópicos Definidos ---
const char* TOPIC_TEMP = "cardioia/sensor/temperatura";
const char* TOPIC_BPM  = "cardioia/sensor/batimentos";
const char* TOPIC_STATUS = "cardioia/status/conexao";

// --- Pinos e Sensores ---
const int PIN_DHT = 15;
const int PIN_BPM = 34; 
DHTesp dhtSensor;

WiFiClientSecure espClient;
PubSubClient client(espClient);

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
    espClient.setInsecure(); 
  } else {
    isOnline = false;
    Serial.println("\nIniciando em modo Offline");
  }
}

void reconnect() {
  while (!client.connected() && WiFi.status() == WL_CONNECTED) {
    Serial.print("Tentando conexão MQTT...");
    // Conectando com LWT (Last Will and Testament) para avisar se cair
    if (client.connect(CLIENT_ID, MQTT_USER, MQTT_PASS, TOPIC_STATUS, 1, true, "offline")) {
      Serial.println("Conectado!");
      client.publish(TOPIC_STATUS, "online", true); // Avisa que está online
      
      if (dataBuffer.length() > 0) {
        client.publish("cardioia/sensor/resiliencia", dataBuffer.c_str());
        dataBuffer = "";
      }
    } else {
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
  int bpm = map(analogRead(PIN_BPM), 0, 4095, 40, 180);

  if (isOnline && client.connected()) {
    client.loop();
    
    // Envio segmentado por tópico
    client.publish(TOPIC_TEMP, String(data.temperature).c_str());
    client.publish(TOPIC_BPM, String(bpm).c_str());
    
    Serial.printf("Enviado -> Temp: %.2f | BPM: %d\n", data.temperature, bpm);
  } else {
    // Armazena string compactada para economizar memória no buffer
    dataBuffer += "T:" + String(data.temperature) + ";B:" + String(bpm) + "|"; 
    Serial.println("Offline - Dados retidos no buffer.");
  }

  delay(5000); 
}