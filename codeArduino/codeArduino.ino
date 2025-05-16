#include <dht.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include "Adafruit_PM25AQI.h"

#define outPin 2

dht DHT;

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

// --- DHT11 ---
// #define DHTPIN 2
// #define DHTTYPE DHT11
// DHT dht(DHTPIN, DHTTYPE);
// --- MQ Sensors ---
#define MQ7_PIN A0    // CO
#define MQ135_PIN A1  // CO2, NO2, NH3
#define MQ131_PIN A2
// --- PMS5003 ---
SoftwareSerial pmsSerial(9, 10);  // RX, TX (TX del sensor a RX del Arduino)
SoftwareSerial nodemcu(5, 6);

void setup() {
  Serial.begin(9600);
  pmsSerial.begin(9600);
  // dht.begin();
  nodemcu.begin(9600);

  delay(2000);

  if (!aqi.begin_UART(&pmsSerial)) {
    Serial.println("Error al iniciar el sensor PMSA003I");
    while (1) delay(10);
  } else {
    Serial.println("PMS Iniciado");
  }
}

void loop() {
  JsonDocument data;
  float pm1, pm25, pm10;
  PM25_AQI_Data dataPms;

  // --- Lecturas PMS5003 ---
  pmsSerial.listen();
  if (aqi.read(&dataPms)) {
    pm1 = dataPms.pm10_standard;
    pm25 = dataPms.pm25_standard;
    pm10 = dataPms.pm100_standard;
    Serial.println("Dust Concentration");
    Serial.println("PM1.0: " + String(pm1) + "(ug/m3)");
    Serial.println("PM2.5: " + String(pm25) + "(ug/m3)");
    Serial.println("PM10 : " + String(pm10) + "(ug/m3)");

    data["pm1.0"] = pm1;
    data["pm2.5"] = pm25;
    data["pm10"] = pm10;
    delay(1000);
  } else {
    Serial.println("No read from PMS");
  }

  // --- Lecturas DHT11 ---
  int readData = DHT.read(outPin);
  float tmp = DHT.temperature;
  float hum = DHT.humidity;
  // --- Lecturas MQ ---
  int mq7 = analogRead(MQ7_PIN);      // CO
  int mq135 = analogRead(MQ135_PIN);  // CO2, NO2, NH3
  int mq131 = analogRead(MQ131_PIN);

  // Conversión estimada (puedes calibrar con curva real si deseas)
  float ppm_co = mq7 * (5.0 / 1023.0) * 20.0;
  float ppm_co2 = mq135 * (5.0 / 1023.0) * 15.0;
  float ppm_no2 = mq135 * (5.0 / 1023.0) * 7.0;
  float ppm_nh3 = mq135 * (5.0 / 1023.0) * 3.0;
  float ppm_o3 = mq131 * (5.0 / 1023.0) * 10.0;

  data["temperatura"] = tmp;
  data["humedad"] = hum;
  data["co"] = ppm_co;
  data["co2"] = ppm_co2;
  data["no2"] = ppm_no2;
  data["nh3"] = ppm_nh3;
  data["ozono"] = ppm_o3;

  String output;
  serializeJsonPretty(data, output);
  Serial.println(output);

  serializeJson(data, nodemcu);
  delay(3000);  // Esperar 3 segundos
}
