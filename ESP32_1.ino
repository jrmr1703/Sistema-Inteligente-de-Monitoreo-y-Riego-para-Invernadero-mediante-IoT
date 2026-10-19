#include <esp_wifi.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>

#define SOIL_PIN 34
#define RELE_BOMBA 26
#define RELE_LED 27

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;

typedef struct {
  float temperatura;
  float humedad;
  float presion;
  int suelo;
  bool bomba;
  bool led;
} DatosSensor;

DatosSensor datos;

typedef struct {
  bool bombaManual;
  bool ledManual;
  bool modoManual;
  int minSuelo;
  int maxSuelo;
} Comandos;

Comandos cmd;

int minSuelo = 30;
int maxSuelo = 60;

void recibirComandos(
    const esp_now_recv_info_t *info,
    const uint8_t *incomingData,
    int len)
{
   memcpy(&cmd, incomingData, sizeof(cmd));

  Serial.println("--------");
Serial.print("modoManual: ");
Serial.println(cmd.modoManual);

Serial.print("bombaManual: ");
Serial.println(cmd.bombaManual);

Serial.print("ledManual: ");
Serial.println(cmd.ledManual);

Serial.print("minSuelo: ");
Serial.println(cmd.minSuelo);

Serial.print("maxSuelo: ");
Serial.println(cmd.maxSuelo);

minSuelo = cmd.minSuelo;
maxSuelo = cmd.maxSuelo;

Serial.print("MIN = ");
Serial.println(minSuelo);

Serial.print("MAX = ");
Serial.println(maxSuelo);

Serial.println("COMANDO RECIBIDO");

Serial.print("modoManual = ");
Serial.println(cmd.modoManual);


// LED
if(cmd.ledManual)
{
    digitalWrite(RELE_LED, LOW);   
}
else
{
    digitalWrite(RELE_LED, HIGH);  
}

// BOMBA SOLO SI ESTA EN MODO MANUAL
if(cmd.modoManual)
{
  if(cmd.bombaManual)
  {
    digitalWrite(RELE_BOMBA, LOW);   
  }
  else
  {
    digitalWrite(RELE_BOMBA, HIGH);  
  }
}
}

uint8_t receptor[] = {
  0xA4, 0xF0, 0x0F,
  0x66, 0xA6, 0xD4
};

void setup() {

  Serial.begin(115200);

  pinMode(RELE_BOMBA, OUTPUT);
  pinMode(RELE_LED, OUTPUT);

  digitalWrite(RELE_BOMBA, HIGH);
digitalWrite(RELE_LED, HIGH);

  Wire.begin(21,22);

  aht.begin();
  bmp.begin(0x77);

  WiFi.mode(WIFI_STA);

  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error ESP-NOW");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receptor, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  esp_now_add_peer(&peerInfo);

  esp_now_register_recv_cb(recibirComandos);

  Serial.println("ESP32 #1 Lista");
}

void loop() {

  sensors_event_t hum, temp;
  aht.getEvent(&hum, &temp);

  datos.temperatura = temp.temperature;
  datos.humedad = hum.relative_humidity;
  datos.presion = bmp.readPressure() / 100.0F;

  int lectura = analogRead(SOIL_PIN);

  datos.suelo = map(
      lectura,
      3400,
      1600,
      0,
      100);

  datos.suelo = constrain(datos.suelo,0,100);

 // Bomba automática SOLO en modo AUTO
if(cmd.modoManual == false)
{
  if(datos.suelo < minSuelo)
    digitalWrite(RELE_BOMBA,LOW);    

if(datos.suelo > maxSuelo)
    digitalWrite(RELE_BOMBA,HIGH);   
}



  datos.bomba = !digitalRead(RELE_BOMBA);
datos.led = !digitalRead(RELE_LED);

  esp_now_send(
      receptor,
      (uint8_t *) &datos,
      sizeof(datos));

  delay(2000);
}