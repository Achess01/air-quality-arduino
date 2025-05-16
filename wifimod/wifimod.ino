#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "ssid-name";
const char* password = "wifi-password";
const char* TOKEN = "UBIDOTS-TOKEN";
const char* DEVICE_LABEL = "DEVICE-LABEL";

SoftwareSerial nodemcu(D6, D5);

WiFiClient client;

void setup() {
  Serial.begin(9600);
  nodemcu.begin(9600);
  while (!Serial) continue;

  WiFi.begin(ssid, password);


  Serial.print("Conectando a WiFi...");
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.println(WiFi.status());
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado!!!");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nNo se pudo conectar a WiFi");
  }
}

void loop() {
  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, nodemcu);
  if (error) {
    Serial.print("Hubo un error: ");
    Serial.println(error.c_str());
    return;
  }

  char payload[256];
  serializeJson(doc, payload);
  Serial.println("Json recibido");
  Serial.println(payload);
  if (WiFi.status() == WL_CONNECTED) {
    

    HTTPClient http;
    String url =
      "http://industrial.api.ubidots.com/api/v1.6/devices/";
    url += DEVICE_LABEL;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Auth-Token", TOKEN);

    int httpCode = http.POST(payload);
    http.end();
    Serial.println("Datos enviados");

    delay(5000);
  } else {
    Serial.println("No se pudo enviar la información: ");
  }
}