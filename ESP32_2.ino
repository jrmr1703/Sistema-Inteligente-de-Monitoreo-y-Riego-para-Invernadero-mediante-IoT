#include <WiFi.h>
#include <esp_now.h>
#include <HardwareSerial.h>
#include <Preferences.h>
#include <PubSubClient.h>

HardwareSerial nextion(2);

WiFiClient espClient;
PubSubClient client(espClient);

typedef struct {
  float temperatura;
  float humedad;
  float presion;
  int suelo;
  bool bomba;
  bool led;
} DatosSensor;

typedef struct {
  bool bombaManual;
  bool ledManual;
  bool modoManual;
  int minSuelo;
  int maxSuelo;
} Comandos;

DatosSensor datos;
Comandos cmd;

Preferences prefs;

bool modoManual = false;

int minSuelo = 30;
int maxSuelo = 60;

uint8_t esp1[] = {
  0x78, 0x42, 0x1C,
  0x67, 0xA2, 0x90
};

const char* mqtt_server = "stem.ubidots.com";
const int mqtt_port = 1883;

const char* TOKEN = "BBUS-91SFgfZJacEbV2sEU27yPpSvMli7xX";

void fin() {
  nextion.write(0xFF);
  nextion.write(0xFF);
  nextion.write(0xFF);
}

void actualizarPantalla() {

  nextion.print("temp.txt=\"");
  nextion.print(datos.temperatura,1);
  nextion.print(" C\"");
  fin();

  nextion.print("tHum.txt=\"");
  nextion.print((int)datos.humedad);
  nextion.print(" %\"");
  fin();

  nextion.print("tSuelo.txt=\"");
  nextion.print(datos.suelo);
  nextion.print(" %\"");
  fin();

  nextion.print("tTempM.txt=\"");
nextion.print(datos.temperatura,1);
nextion.print("\"");
fin();

nextion.print("tHumM.txt=\"");
nextion.print((int)datos.humedad);
nextion.print("\"");
fin();

nextion.print("tSueloM.txt=\"");
nextion.print(datos.suelo);
nextion.print("\"");
fin();

  nextion.print("tBomba.txt=\"");
  nextion.print(datos.bomba ? "ON" : "OFF");
  nextion.print("\"");
  fin();

  nextion.print("tEstadoBomba.txt=\"");
  nextion.print(datos.bomba ? "ON" : "OFF");
  nextion.print("\"");
  fin();

  if(modoManual)
{
  nextion.print("tModo.txt=\"MANUAL\"");
  fin();
}
else
{
  nextion.print("tModo.txt=\"AUTO\"");
  fin();
}

  if(datos.suelo < 30)
  {
    nextion.print("tAlerta.txt=\"SUELO SECO\"");
  }
  else if(datos.bomba)
  {
    nextion.print("tAlerta.txt=\"RIEGO\"");
  }
  else
  {
    nextion.print("tAlerta.txt=\"NORMAL\"");
  }

  fin();
}

void bombaON()
{
  cmd.modoManual = true;
  cmd.bombaManual = true;

  esp_now_send(
      esp1,
      (uint8_t*)&cmd,
      sizeof(cmd));

  Serial.println("BOMBA ON");
}

void bombaOFF()
{
  cmd.modoManual = true;
  cmd.bombaManual = false;

  esp_now_send(
      esp1,
      (uint8_t*)&cmd,
      sizeof(cmd));

  Serial.println("BOMBA OFF");
}
uint32_t leerNumeroNextion(String objeto)
{
  while(nextion.available())
    nextion.read();

  nextion.print("get ");
  nextion.print(objeto);
  nextion.print(".val");
  fin();

  uint32_t valor = 0;
  unsigned long t = millis();

  while(millis() - t < 500)
  {
    if(nextion.available() >= 8)
    {
      if(nextion.read() == 0x71)
      {
        valor = nextion.read();
        valor |= nextion.read() << 8;
        valor |= nextion.read() << 16;
        valor |= nextion.read() << 24;

        nextion.read();
        nextion.read();
        nextion.read();

        return valor;
      }
    }
  }

  return 0;
}

void actualizarAutomatico()
{
  nextion.print("nMin.val=");
  nextion.print(minSuelo);
  fin();

  nextion.print("nMax.val=");
  nextion.print(maxSuelo);
  fin();

  nextion.print("j0.val=");
  nextion.print(minSuelo);
  fin();

  nextion.print("j1.val=");
  nextion.print(maxSuelo);
  fin();
}

void recibirDatos(const esp_now_recv_info_t *info,
                  const uint8_t *incomingData,
                  int len)
{
  memcpy(&datos, incomingData, sizeof(datos));

  Serial.println("Datos recibidos");

  actualizarPantalla();

   
  
}

void conectarMQTT()
{
  while (!client.connected())
  {
    Serial.print("Conectando MQTT...");

    if (client.connect("ESP32_INVERNADERO", TOKEN, TOKEN))
    {
      Serial.println(" OK");
    }
    else
    {
      Serial.print(" Error: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {

  Serial.begin(115200);
   
    WiFi.begin("INFINITUM4286", "y3EXu7eTDg");

  Serial.print("Conectando WiFi");

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  Serial.print("Canal WiFi: ");
  Serial.println(WiFi.channel());

  client.setServer(mqtt_server, mqtt_port);

  prefs.begin("invernadero", false);

   minSuelo = prefs.getInt("min", 30);
   maxSuelo = prefs.getInt("max", 60);
   modoManual = prefs.getBool("modo", false); 

  nextion.begin(9600, SERIAL_8N1, 5, 21);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(recibirDatos);

  esp_now_peer_info_t peerInfo = {};

  memcpy(peerInfo.peer_addr,
         esp1,
         6);

  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  esp_now_add_peer(&peerInfo);

  Serial.println("ESP32 #2 lista");
}


void loop()
{
  
  if (!client.connected())
{
    conectarMQTT();
}

client.loop();
  
  if(nextion.available())
  {
    uint8_t c = nextion.read();

    if(c == 0x65)
    {
      Serial.println("BOTON NEXTION");

      delay(10);

      uint8_t pagina = nextion.read();
      uint8_t componente = nextion.read();

      nextion.read();

      Serial.print("Pagina: ");
      Serial.print(pagina);

      Serial.print(" Componente: ");
      Serial.println(componente);

      // GUARDAR AUTOMATICO

if(pagina == 3 && componente == 9)
{
  Serial.println("GUARDAR AUTOMATICO");

  minSuelo = leerNumeroNextion("nMin");
  maxSuelo = leerNumeroNextion("nMax");

  prefs.putInt("min", minSuelo);
  prefs.putInt("max", maxSuelo);
  prefs.putBool("modo", modoManual);

  cmd.minSuelo = minSuelo;
  cmd.maxSuelo = maxSuelo;

    esp_now_send(
    esp1,
    (uint8_t*)&cmd,
    sizeof(cmd));


  Serial.print("MIN GUARDADO: ");
  Serial.println(minSuelo);

  Serial.print("MAX GUARDADO: ");
  Serial.println(maxSuelo);

  nextion.print("vis p0,1");
  fin();
}

      // CONFIGURACION

     if(pagina == 2 && componente == 2)
{
  modoManual = false;

  prefs.putBool("modo", false);

  cmd.modoManual = false;

  esp_now_send(
      esp1,
      (uint8_t*)&cmd,
      sizeof(cmd));

  nextion.print("tModo.txt=\"AUTO\"");
  fin();

  actualizarAutomatico();

  Serial.println("MODO AUTO");
}

      if(pagina == 2 && componente == 3)
{
  modoManual = true;

  prefs.putBool("modo", true);

  cmd.modoManual = true;

  esp_now_send(
      esp1,
      (uint8_t*)&cmd,
      sizeof(cmd));

  nextion.print("tModo.txt=\"MANUAL\"");
  fin();

  Serial.println("MODO MANUAL");
}

      // MANUAL

      if(pagina == 4 && componente == 3)
      {
        bombaON();

        nextion.print("tEstadoBomba.txt=\"ON\"");
        fin();

        Serial.println("BOMBA ON");
      }

      if(pagina == 4 && componente == 4)
      {
        bombaOFF();

        nextion.print("tEstadoBomba.txt=\"OFF\"");
        fin();

        Serial.println("BOMBA OFF");
      }

      // ILUMINACION

if(pagina == 7 && componente == 3)
{
  
  cmd.ledManual = true;

  esp_now_send(
      esp1,
      (uint8_t*)&cmd,
      sizeof(cmd));

  Serial.println("LED ON");
}

if(pagina == 7 && componente == 4)
{
  cmd.ledManual = false;

  esp_now_send(
      esp1,
      (uint8_t*)&cmd,
      sizeof(cmd));

  Serial.println("LED OFF");
}
    }
  }
}
