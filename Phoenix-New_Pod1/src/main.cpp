#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include "Adafruit_VEML6070.h"

#define BOARD_ID 1

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

uint8_t broadcastAddress[] = {0x8C, 0xaa, 0xb5, 0x86, 0x1C, 0x94};
Adafruit_VEML6070 uv = Adafruit_VEML6070();
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

  uv.begin(VEML6070_1_T); // pass in the integration time constant

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

  pinMode(0, INPUT);
  pinMode(0, INPUT);
}

void loop()
{
  counter = counter + 1;

  float temperature = 0;
  for (int x = 0; x < 200; x++)
  {

    temperature = (analogRead(A0)*0.0404);
  }
  float smoke = map(digitalRead(0), 0, 4095, 0, 100);
  int Uv = uv.readUV();
  
  Serial.print("BOARD_ID: ");
  Serial.println(BOARD_ID);
  Serial.print("Temp: ");
  Serial.print(temperature,1);
  Serial.println(" C");
  Serial.print("smoke: ");
  Serial.print(smoke,0);
  Serial.println();
  Serial.print("Uv ");
  Serial.print(Uv);
  Serial.println(" ");
  Serial.print("counter: ");
  Serial.print(counter);
  Serial.println();

  // Set values to send  Serial
  myData.id = BOARD_ID;
  myData.temperature = temperature;
  myData.smoke = smoke;
  myData.UV = Uv;
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