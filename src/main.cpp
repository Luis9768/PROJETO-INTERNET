#include <Arduino.h>
#include "internet.h"
#include <PubSubClient.h>
#include <WiFi.h>

WiFiClient espClient;
PubSubClient client(espClient);

const char *mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char *mqtt_id = "esp32-senai134-luis";
const char *mqtt_topic_sub = "senai134/mesa07/esp_inscrito";
const char *mqtt_topic_pub = "senai134/mesa07/esp_publicando";

void callback(char *, byte *, unsigned int);
void mqttConnect(void);

void setup()
{
  Serial.begin(9600);
  conectaWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop()
{
  checkWiFi();
}

void callback(char *topic, byte *payload, unsigned int lenght)
{
  Serial.printf("mensagem recebida em %s", topic);

  String mensagem = "";
  for (unsigned int i = 0; i < lenght; i++)
  {
    char c = (char)payload[i];
    mensagem += c;
  }
  Serial.println(mensagem);
}
void mqttConnect()
{
  while (!client.connected())
  {
    Serial.println("Conectando ao MQTT...");

    if (client.connect(mqtt_id))
    {
      Serial.println("Conectado com Sucesso");
      client.subscribe(mqtt_topic_sub);
    }
    else
    {
      Serial.printf("falha, rc=");
      Serial.println(client.state());
      Serial.println("tentando novament em 5 segundos");
      delay(5000);
    }
  }
}