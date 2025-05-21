#include <Arduino.h>
#include "internet.h"
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

WiFiClient espClient;
PubSubClient client(espClient);
//MQTT
const char *mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char *mqtt_id = "esp32-senai134-luis";
const char *mqtt_topic_sub = "senai134/mesa07/esp_inscrito";
const char *mqtt_topic_pub = "senai134/mesa07/esp_publicando";

//tratamento leds
#define pinLed 2
bool estadoLed = 0;
bool modoPisca = false;
float tempoPisca = 1000;
const int VELOCIDADE_MIN = 50;
const int VELOCIDADE_MAX = 5000;

void callback(char *, byte *, unsigned int);
void mqttConnect(void);
void controleDosleds(void);

void setup()
{
  Serial.begin(9600);
  conectaWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  pinMode(pinLed, OUTPUT);
}

void loop()
{
  checkWiFi();
  if (!client.connected())
    mqttConnect();
  client.loop();

  static unsigned long tempoAnterior = 0;
  unsigned long tempoAtual = millis();
  if (tempoAtual - tempoAnterior > 3000)
  {
    client.publish(mqtt_topic_pub, "OLA MUNDO");
    tempoAnterior = tempoAtual;
  }
  controleDosleds();
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
  
  JsonDocument doc;
  deserializeJson(doc, mensagem);
  if(!doc["estadoLed"].isNull())
  {
    estadoLed = doc["estadoLed"];
    modoPisca =0;
  }
  if(!doc[modoPisca].isNull())
  {
modoPisca = doc["modoPisca"];
  }
if(!doc["velocidade"].isNull())
{
  tempoPisca = doc["velocidade"];
}



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
void controleDosleds()
{
  digitalWrite(pinLed, estadoLed);
  static unsigned long ultimaMudanca = 0;
  unsigned long agora = millis();

  if (modoPisca)
  {
    if (agora - ultimaMudanca > tempoPisca)
    {
      estadoLed = !estadoLed;
      ultimaMudanca = agora;
    }
  }
  digitalWrite(pinLed, estadoLed);
}