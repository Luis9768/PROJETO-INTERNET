#include <Arduino.h>
#include "internet.h"
#include <PubSubClient.h>
#include <WiFi.h>

WiFiClient espClient;
PubSubClient client(espClient);

const char *mqtt_server= "broker.hivemq.com";
const int mqtt_port= 1883;
const char *mqtt_id= "esp32-senai134-luis";
const char *mqtt_topic_sub = "senai134/mesa07/esp_inscrito";
const char *mqtt_topic_pub = "senai134/mesa07/esp_publicando";


/*void callback(char*, byte*, unsigned int)
{

}*/
void setup()
{
  Serial.begin(9600);
  conectaWiFi();
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
    char c =(char)payload[i];
    mensagem += c;
  }
Serial.println(mensagem);
}