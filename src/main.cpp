#include <Arduino.h>      //biblioteca do arduino
#include "internet.h"     //biblioteca pra vc se conectar a internet
#include <PubSubClient.h> //pra vc publica no MQTT
#include <WiFi.h>         //pra vc se conectar com o WIFI
#include <ArduinoJson.h>  // Pra ele ler e fazer umas paradas com o Json
#include <Bounce2.h>      //Bounce pra não ter ruido no botao
#include <ezTime.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// DHT
#define DHTPIN 5
#define DHTTYPE DHT22

WiFiClient espClient;            // Objeto do WIFI para vc se conectar
PubSubClient client(espClient);  // Se conecta ao mqtt
Bounce botaoDebounce = Bounce(); // objeto do bounce para evitar ruido
Timezone tempo;
DHT dht (DHTPIN, DHTTYPE);

// MQTT
const char *mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char *mqtt_id = "esp32-senai134-luis";
const char *mqtt_topic_sub = "senai134/mesa07/esp_inscrito";
const char *mqtt_topic_pub = "senai134/mesa07/esp_publicando";

// tratamento leds
#define pinLed 2
#define pinBotao 0
bool estadoLed = 0;
bool modoPisca = false;
float tempoPisca = 1000;
bool estadoBotao = 0;
static bool estadoAnteriorBotao = 0;

//voids
void callback(char *, byte *, unsigned int);
void mqttConnect(void);
void controleDosleds(void);

void setup()
{
  Serial.begin(9600); // aciona a Serial
  dht.begin();
  conectaWiFi();                                // conecta ao WIfi
  client.setServer(mqtt_server, mqtt_port);     // vc se conecta ao mqtt
  client.setCallback(callback);                 // vc ativa o callback
  pinMode(pinLed, OUTPUT);                      // ativa o 2 como entrada e faz o led piscar
  botaoDebounce.attach(pinBotao, INPUT_PULLUP); // o botaoDebounce server como um pinMode ele vem da biblioteca bounce2 e evita ruido do botao
  botaoDebounce.interval(25);                   // para evitar o ruido a cada 25ms desde quando vc aperta ele ignora qualquer comando

  waitForSync();
  tempo.setLocation("America/Sao_Paulo");
}

void loop()
{
  checkWiFi();             
  if (!client.connected()) 
    mqttConnect();         
  client.loop();           

  botaoDebounce.update(); 
  if (botaoDebounce.fell())
  {
    JsonDocument doc;
    doc["botao"] = true;
    doc["msg"] = "Ola Senai";
    String mensagem;
    serializeJson(doc, mensagem);
    client.publish(mqtt_topic_pub, mensagem.c_str());
  }

  // Lê temperatura e umidade a cada 2 segundos
  static unsigned long ultimoTempoLeitura = 0;
  unsigned long agora = millis();
  if (agora - ultimoTempoLeitura >= 3000) {
    ultimoTempoLeitura = agora;

    float temperatura = dht.readTemperature();
    float humidade = dht.readHumidity();

    if (isnan(temperatura) || isnan(humidade)) {
      Serial.println("falha na leitura do sensor");
    } else {
      Serial.print("Temperatura: ");
      Serial.print(temperatura);
      Serial.println(" °C");
      Serial.print("Humidade: ");
      Serial.print(humidade);
      Serial.println(" %");
      Serial.print("date time: ");
      Serial.println(dateTime());

      JsonDocument doc;
      doc["temperatura"] = temperatura;
      doc["humidade"] = humidade;
      doc["timestamp"] = tempo.dateTime();

      String mensagem;
      serializeJson(doc, mensagem);
      client.publish(mqtt_topic_pub, mensagem.c_str());
    }
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
  if (!doc["estadoLed"].isNull())
  {
    estadoLed = doc["estadoLed"];
    modoPisca = 0;
  }
  if (!doc["modoPisca"].isNull())
  {
    modoPisca = doc["modoPisca"];
  }
  if (!doc["velocidade"].isNull())
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