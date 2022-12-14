#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "Adafruit_SGP30.h"

#define BOARD_ID 2
#define SEALEVELPRESSURE_HPA (1026.25)

uint8_t broadcastAddress[] = {0x8C, 0xaa, 0xb5, 0x86, 0x1C, 0x94};
Adafruit_BME680 bme; // I2C
Adafruit_SGP30 sgp;

typedef struct struct_message
{
  int id;
  float temperature;
  float humidity;
  float pressure;
  int Gas;
  int CO;
  int smoke;
  float moisture;
  int UV;
  int CO2;
  int NH3;
  int NO2;
  int VOC;
  int H2;
  unsigned int counter;
} struct_message;

struct_message myData;

unsigned int counter = 0;
constexpr char WIFI_SSID[] = "TP-Link_7C28";

int32_t getWiFiChannel(const char *ssid)
{
  if (int32_t n = WiFi.scanNetworks())
  {
    for (uint8_t i = 0; i < n; i++)
    {
      if (!strcmp(ssid, WiFi.SSID(i).c_str()))
      {
        return WiFi.channel(i);
      }
    }
  }
  return 0;
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup()
{
  Serial.begin(9600);

  Serial.println(F("BME680 test"));
  if (!bme.begin())
  {
   Serial.println("BME680 sensor test failed.");
   while (1)
     ;
  }
  if (!sgp.begin())
  {
    Serial.println("SGP30 sensor not found :(");
    while (1)
      ;
  }
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  // Set device as a Wi-Fi Station and set channel
  WiFi.mode(WIFI_STA);

  int32_t channel = getWiFiChannel(WIFI_SSID);

  WiFi.printDiag(Serial); // Uncomment to verify channel number before
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  WiFi.printDiag(Serial); // Uncomment to verify channel change after

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop()
{
  counter = counter + 1;

  unsigned long endTime = bme.beginReading();
  if (endTime == 0)
  {
    Serial.println(F("BME680 measurement failed :("));
    return;
  }
  if (!bme.endReading())
  {
    Serial.println(F("BME680 failed to complete reading :("));
    return;
  }
  if (!sgp.IAQmeasureRaw())
  {
    Serial.println("SGP30 Measurement failed");
    return;
  }

  float pressure = (bme.pressure / 3377);
  float VOC = (bme.gas_resistance / 1000);
  float LPG = analogRead(36) / 4095;
  float CO = analogRead(34) / 4095;
  float NH3 = analogRead(38) / 4095;
  float NO2 = analogRead(39) / 4095;

  Serial.print("BOARD_ID: ");
  Serial.println(BOARD_ID);
  Serial.print("Temp: ");
  Serial.print(bme.temperature, 0);
  Serial.println(" C");
  Serial.print("Humidity: ");
  Serial.print(bme.humidity, 0);
  Serial.println(" %");
  Serial.print("Pressure: ");
  Serial.print(pressure, 0);
  Serial.println(" in Hg");
  Serial.print("VOC: ");
  Serial.println(1/VOC, 0);
  Serial.print("LPG: ");
  Serial.print(LPG, 0);
  Serial.println(" PPM");
  Serial.print("CO: ");
  Serial.print(CO, 0);
  Serial.println(" PPM");
  Serial.print("NH3: ");
  Serial.print(NH3, 0);
  Serial.println(" PPB");
  Serial.print("NO2: ");
  Serial.print(NO2, 0);
  Serial.println(" PPB");
  Serial.print("counter: ");
  Serial.print(counter);
  Serial.println();

  // Set values to send  Serial
  myData.id = BOARD_ID;
  myData.temperature = bme.temperature;
  myData.humidity = bme.humidity;
  myData.pressure = pressure;
  myData.VOC = VOC;
  myData.CO2 = sgp.eCO2;
  myData.H2 = sgp.rawH2;
  myData.Gas = LPG;
  myData.CO = CO;
  myData.NH3 = NH3;
  myData.NO2 = NO2;
  myData.counter = counter;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  if (result == ESP_OK)
  {
    Serial.println("Sent with success");
  }
  else
  {
    Serial.println("Error sending the data");
  }
  delay(5000);
}