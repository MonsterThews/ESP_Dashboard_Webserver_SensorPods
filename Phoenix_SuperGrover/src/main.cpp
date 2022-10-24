#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include "ESPAsyncWebServer.h"
#include <Arduino_JSON.h>
#include "Adafruit_GFX.h"
#include "Fonts/FreeMono12pt7b.h"
#include "Fonts/FreeMono9pt7b.h"
#include <MCUFRIEND_kbv.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define NTP_OFFSET -14400      // In seconds
#define NTP_INTERVAL 60 * 1000 // In miliseconds
#define NTP_ADDRESS "pool.ntp.org"
#define CYAN 0x07FF
#define YELLOW 0xFFE0
#define BLACK 0x0000
#define PINK 0xFC9F
#define AQUA 0x04FF
#define WHITE 0xFFFF

const char *ssid = "TP-Link_7C28";
const char *password = "64411811";

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
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
MCUFRIEND_kbv tft;
JSONVar board;
AsyncWebServer server(80);
AsyncEventSource events("/events");

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
{
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));

  board["id"] = myData.id;
  board["temperature"] = myData.temperature;
  board["humidity"] = myData.humidity;
  board["pressure"] = myData.pressure;
  board["UV"] = myData.UV;
  board["CO"] = myData.CO;
  board["CO2"] = myData.CO2;
  board["Gas"] = myData.Gas;
  board["smoke"] = myData.smoke;

  board["counter"] = String(myData.counter);
  String jsonString = JSON.stringify(board);
  events.send(jsonString.c_str(), "new_readings", millis());

  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  /*
  Serial.printf("m value: %4.2f \n", myData.moisture);
  Serial.printf("s value: %4.2f \n", myData.smoke);
  Serial.printf("u value: %4.2f \n", myData.UV);
  Serial.printf("g value: %4.2f \n", myData.Gas);
  Serial.printf("c value: %4.2f \n", myData.CO);
  Serial.printf("o value: %4.2f \n", myData.CO2);
  Serial.printf("v value: %4.2f \n", myData.VOC);
  Serial.printf("h value: %4.2f \n", myData.H2);
  Serial.printf("n value: %4.2f \n", myData.NH3);
  Serial.printf("2 value: %4.2f \n", myData.NO2);
  Serial.print("ID:"); 
  Serial.println(myData.counter);
  Serial.println();
*/
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Monster Laboratories SmartHome Air Quality</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="monsterlaboratories.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {  font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #2f4468; color: white; font-size: 1.7rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .reading { font-size: 2.8rem; }
    .packet { color: #bebebe; }
    .card.temperature { color: #fd7e14; }
    .card.humidity { color: #1b78e2; }
    .card.pressure { color: #4682B4; }
    .card.UV { color: #FF00FF; }
  </style>
</head>
<body>
  <div class="topnav">
    <h3>Monster Laboratories SmartHome</h3>
  </div>
	<table border=0 width=750>
	<tr><td valign="top">
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> Pod 2 - TEMPERATURE</h4><p><span class="reading"><span id="t2"></span> &deg;C</span></p><p class="packet">Reading ID: <span id="rt2"></span></p>
      </div> 
	  </td><td valign="top">
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i> Pod 2 - HUMIDITY</h4><p><span class="reading"><span id="h2"></span> &percnt;</span></p><p class="packet">Reading ID: <span id="rh2"></span></p>
      </div> 
	  </td><td valign="top">
      <div class="card pressure">
        <h4><i class="fas fa-tint"></i> Pod 2 - PRESSURE</h4><p><span class="reading"><span id="p2"></span> "Hg</span></p><p class="packet">Reading ID: <span id="rp2"></span></p>
      </div>
	  </td></tr>
	  <tr><td valign="top">
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> Pod 1 - TEMPERATURE</h4><p><span class="reading"><span id="t1"></span> &deg;C</span></p><p class="packet">Reading ID: <span id="rt1"></span></p>
      </div> 
	  	  </td><td valign="top">
	  </td><td valign="top">
         <div class="card UV">
        <h4><i class="fas fa-tint"></i> Pod 1 - UV</h4><p><span class="reading"><span id="u1"></span> </span></p><p class="packet">Reading ID: <span id="ru1"></span></p>
      </div>
      </td></tr>
	  </table>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('new_readings', function(e) {
  console.log("new_readings", e.data);
  var obj = JSON.parse(e.data);

  document.getElementById("t"+obj.id).innerHTML = obj.temperature.toFixed(1);
  document.getElementById("h"+obj.id).innerHTML = obj.humidity.toFixed(0);
  document.getElementById("p"+obj.id).innerHTML = obj.pressure.toFixed(0);
  document.getElementById("u"+obj.id).innerHTML = obj.UV.toFixed(0);
  document.getElementById("c"+obj.id).innerHTML = obj.CO.toFixed(0);
  document.getElementById("o"+obj.id).innerHTML = obj.CO2.toFixed(0);
  document.getElementById("g"+obj.id).innerHTML = obj.Gas.toFixed(0);
  document.getElementById("s"+obj.id).innerHTML = obj.smoke.toFixed(0);

  document.getElementById("rt"+obj.id).innerHTML = obj.counter;

 }, false);
}
</script>
</body>
</html>)rawliteral";

void setup()
{
  Serial.begin(9600);
  uint16_t ID = tft.readID();
  tft.begin(ID);

  WiFi.mode(WIFI_AP_STA);

  // Set device as a Wi-Fi Station
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html); });

  events.onConnect([](AsyncEventSourceClient *client)
                   {
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000); });
  server.addHandler(&events);
  server.begin();
}

void loop()
{
  float dewpoint = (myData.temperature - (100 - myData.humidity) / 5);
  timeClient.update();
  String formattedTime = timeClient.getFormattedTime();

  tft.setFont(&FreeMono9pt7b);
  tft.fillScreen(BLACK);
  tft.setTextSize(1);
  tft.setTextColor(YELLOW);
  tft.setCursor(1, 20);
  tft.print("Station IP : ");
  tft.println(WiFi.localIP());
  tft.setFont(&FreeMono12pt7b);
  tft.setTextColor(WHITE);
  tft.print("Time: ");
  tft.println(formattedTime);
  if (myData.id == 1)
  {
    tft.setTextColor(CYAN);
    tft.print("Board ID:    ");
    tft.println(myData.id);
    tft.print("Temperature: ");
    tft.print(myData.temperature, 0);
    tft.println("*C");
    tft.print("Moisture:    ");
    tft.print(myData.moisture, 0);
    tft.println("%");
    tft.print("Smoke:       ");
    tft.println(myData.smoke, 0);
    tft.print("Uv index:    ");
    tft.println(myData.UV, 0);
    tft.print("Reading ID:  ");
    tft.println(myData.counter);
    tft.println();
  }
  else if (myData.id == 2)
  {
    tft.setTextColor(PINK);
    tft.print("Pod ID:    ");
    tft.println(myData.id);
    tft.print("Temperature: ");
    tft.print(myData.temperature, 0);
    tft.println("*C");
    tft.print("Dewpoint:    ");
    tft.print(dewpoint, 0);
    tft.println("*C");
    tft.print("Pressure:    ");
    tft.print(myData.pressure, 0);
    tft.println(" in Hg");
    tft.print("Humidity:    ");
    tft.print(myData.humidity, 0);
    tft.println("%");
    tft.print("LPG: ");
    tft.println(myData.Gas, 0);
    tft.print("CO:  ");
    tft.println(myData.CO, 0);
    tft.print("CO2: "),
    tft.println(myData.CO2, 0);
    tft.print("VOC: ");
    tft.println(myData.VOC, 0);
    tft.print("H2:  ");
    tft.println(myData.H2, 0);
    tft.print("NH3: ");
    tft.println(myData.NH3, 0);
    tft.print("NO2: "),
    tft.println(myData.NO2, 0);
    tft.setCursor(1, 385);
    tft.print("Reading ID: ");
    tft.println(myData.counter);
    tft.println();
  }
 /* else if (myData.id == 3)
  {
    tft.setTextColor(AQUA);
  tft.print("Board ID:    ");
  tft.println(myData.id);
  tft.print("Temperature: ");
  tft.print(myData.temperature, 0);
  tft.println("*C");
  tft.print("Dewpoint:    ");
  tft.print(dewpoint, 0);
  tft.println("*C");
  tft.print("Pressure:    ");
  tft.print(myData.pressure, 0);
  tft.println(" in Hg");
  tft.print("Humidity:    ");
  tft.print(myData.humidity, 0);
  tft.println("%");
  tft.print("Moisture:    ");
  tft.print(myData.moisture, 0);
  tft.println("%");
  tft.print("Smoke:       ");
  tft.println(myData.smoke, 0);
  tft.print("Uv index:    ");
  tft.println(myData.UV, 0);
  tft.setCursor(1, 290);
  tft.print("LPG: ");
  tft.println(myData.Gas, 0);
  tft.print("CO:  ");
  tft.println(myData.CO, 0);
  tft.print("CO2: "),
  tft.println(myData.CO2, 0);
  tft.print("VOC: ");
  tft.println(myData.VOC, 0);
  tft.setCursor(150, 290);
  tft.print("H2:  ");
  tft.println(myData.H2, 0);
  tft.setCursor(150, 315);
  tft.print("NH3: ");
  tft.println(myData.NH3, 0);
  tft.setCursor(150, 340);
  tft.print("NO2: "),
  tft.println(myData.NO2, 0);
  tft.setCursor(1, 385);
  tft.print("Reading ID:  ");
  tft.println(myData.counter);
  tft.println();
  }*/

    delay(15000);
}