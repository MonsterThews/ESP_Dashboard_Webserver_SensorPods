#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include "ESPAsyncWebServer.h"
#include "Adafruit_GFX.h"
#include "Fonts/FreeMono12pt7b.h"
#include "Fonts/FreeMono9pt7b.h"
#include <MCUFRIEND_kbv.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "SPIFFS.h"

#define NTP_OFFSET -14400      // In seconds
#define NTP_INTERVAL 60 * 1000 // In miliseconds
#define NTP_ADDRESS "pool.ntp.org"
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define RED 0xF800
#define YELLOW 0xFFE0
#define BLACK 0x0000
#define PINK 0xFC9F
#define AQUA 0x04FF
#define WHITE 0xFFFF

template <typename valueT>
void PrintHex(Stream &stream, const valueT val)
{
  const uint8_t digits = sizeof(valueT) << 1;
  uint8_t i = 0;
  while (i < digits)
  {
    valueT v = (val >> ((digits - i - 1) << 2)) & valueT(0x0F); // Each nibble stores 1 digit
    stream.print(v, HEX);
    ++i;
  }
}

template <typename addrT = size_t, uint8_t bytesPerRow = 16>
void HexDump(Stream &stream, void *buff, size_t len, addrT base = 0)
{
  uint8_t *p = reinterpret_cast<uint8_t *>(buff);
  const size_t rows = (len + bytesPerRow - 1) / bytesPerRow;

  for (size_t r = 0; r < rows; ++r)
  {
    PrintHex<addrT>(stream, base + p - reinterpret_cast<uint8_t *>(buff));
    stream.print(F(": "));

    char *pc = reinterpret_cast<char *>(p);
    const size_t cols = len < bytesPerRow ? len : bytesPerRow;
    for (size_t c = 0; c < bytesPerRow; ++c)
    {
      if (c < cols)
      {
        PrintHex<uint8_t>(stream, *p++);
      }
      else
      {
        stream.print(F("  "));
      }
      stream.print(F(" "));
    }
    stream.print(F(" "));
    yield();

    for (size_t i = 0; i < cols; ++i)
    {
      char c = *pc++;
      if (c >= ' ')
        stream.print(c);
      else
        stream.print('.');
    }
    stream.println(F(""));
    len -= bytesPerRow;
    yield();
  }
}

// Replace with your network credentials (STATION)
const char *ssid = "TP-Link_7C28";
const char *password = "64411811";
unsigned int readingId = 0;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message
{
  int id;
  float temperature;
  float humidity;
  float pressure;
  float Gas;
  float CO;
  float smoke;
  float moisture;
  float UV;
  float CO2;
  float NH3;
  float NO2;
  float VOC;
  float H2;
  float EtOH;
  unsigned int readingId;
} struct_message;

struct_message myData;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
MCUFRIEND_kbv tft;
AsyncWebServer server(80);
AsyncEventSource events("/events");

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
{
  // Copies the sender mac address to a string
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));

  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  Serial.printf("t value: %4.2f \n", myData.temperature);
  Serial.printf("h value: %4.2f \n", myData.humidity);
  Serial.printf("p value: %4.2f \n", myData.pressure);
  Serial.printf("ID: %u \n", myData.readingId);
  Serial.println();
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Monster Laboratories Smart Home</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="data/monsterlaboratories.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {  font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #2f4468; color: white; font-size: 1.7rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 700; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(700px, 1fr)); }
    .reading { font-size: 2.8rem; }
    .packet { color: #bebebe; }
    .card.temperature { color: #fa8602; }
    .card.humidity { color: #02fab8; }
    .card.pressure { color: #204773; }
    .card.UV { color: #e502fa; }
    .card.CO { color: #4c5959; }
    .card.CO2 { color: #47555e; }
    .card.Gas { color: #62376e; }
    .card.VOC { color: #45112e; }
  </style>
</head>
<body>
  <div class="topnav">
    <h3>Monster Laboratories SmartHome- Air Quality</h3>
  </div>
<table width = 950  border = 0 class="cards" >
<tr><td valign = "top">
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> Pod 1 - TEMPERATURE</h4>
        <p><span class="reading"><span id="t1"></span> &deg;C</span></p>
        <p class="packet">ID: <span id="rt1"></span></p>
      </div>
</td><td valign = "top">
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> Pod 2 - TEMPERATURE</h4>
        <p><span class="reading"><span id="t2"></span> &deg;C</span></p>
        <p class="packet">ID: <span id="rt2"></span></p>
      </div>
</td><td valign = "top">
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> Pod 3 - TEMPERATURE</h4>
        <p><span class="reading"><span id="t3"></span> &deg;C</span></p>
        <p class="packet">ID: <span id="rt3"></span></p>
      </div>
</td><td valign = "top">
      <div class="card UV">
        <h4><i class="fas fa-radiation"></i> Pod 3 - UVindex</h4>
        <p><span class="reading"><span id="u3"></span>  x</span></p>
        <p class="packet">ID: <span id="ru3"></span></p>
      </div>
</td></tr>
<tr>
<td valign = "top">   
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i> Pod 1 - HUMIDITY</h4>
        <p><span class="reading"><span id="h1"></span> &percnt;</span></p>
        <p class="packet">ID: <span id="rh1"></span></p>
      </div>    
</td>
<td valign = "top">   
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i> Pod 2 - HUMIDITY</h4>
        <p><span class="reading"><span id="h2"></span> &percnt;</span></p>
        <p class="packet">ID: <span id="rh2"></span></p>
      </div>    
</td>
<td valign = "top">   
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i> Pod 3 - HUMIDITY</h4>
        <p><span class="reading"><span id="h3"></span> &percnt;</span></p>
        <p class="packet">ID: <span id="rh3"></span></p>
      </div>    
</td>
<td valign = "top">   
      <div class="card smoke">
        <h4><i class="fas fa-tint"></i> Pod 3 - SMOKE</h4>
        <p><span class="reading"><span id="s3"></span>  </span></p>
        <p class="packet">ID: <span id="rs3"></span></p>
      </div>    
</td></tr>
<tr>
<td valign = "top">
      <div class="card pressure">
        <h4><i class='fas fa-cloud-download-alt'></i> Pod 1 - PRESSURE</h4>
        <p><span class="reading"><span id="p1"></span> "Hg</span></p>
        <p class="packet">ID: <span id="rp1"></span></p>
      </div>  
</td>
<td valign = "top">
      <div class="card pressure">
        <h4><i class='fas fa-cloud-download-alt'></i> Pod 2 - PRESSURE</h4>
        <p><span class="reading"><span id="p2"></span> "Hg</span></p>
        <p class="packet">ID: <span id="rp2"></span></p>
      </div>  
</td><td valign = "top">
      <div class="card pressure">
        <h4><i class='fas fa-cloud-download-alt'></i> Pod 3 - PRESSURE</h4>
        <p><span class="reading"><span id="p3"></span> "Hg</span></p>
        <p class="packet">ID: <span id="rp3"></span></p>
      </div>  
</td></tr>
<tr>
<td valign = "top">
      <div class="card voc">
        <h4><i class='fas fa-cloud-download-alt'></i> Pod 1 - VOC</h4>
        <p><span class="reading"><span id="v1"></span> </span></p>
        <p class="packet">ID: <span id="rv1"></span></p>
      </div>  
</td><td valign = "top">
      <div class="card voc">
        <h4><i class='fas fa-cloud-download-alt'></i> Pod 2 - VOC</h4>
        <p><span class="reading"><span id="v2"></span> </span></p>
        <p class="packet">ID: <span id="rv2"></span></p>
      </div>  
</td><td valign = "top">
      <div class="card voc">
        <h4><i class='fas fa-cloud-download-alt'></i> Pod 3 - VOC</h4>
        <p><span class="reading"><span id="v3"></span> </span></p>
        <p class="packet">ID: <span id="rv3"></span></p>
      </div>  
</td></tr>
<tr>
<td valign = "top">
     <div class="card CO">
        <h4><i class="fas fa-skull-crossbones"></i> Pod 2 - CO</h4>
        <p><span class="reading"><span id="c2"></span> </span></p>
        <p class="packet">ID: <span id="rc2"></span></p>
      </div>     
</td><td valign = "top">
     <div class="card Gas" >
        <h4><i class="fa fa-cloud"></i> Pod 2 - Gas</h4>
        <p><span class="reading"><span id="g2"></span> </span></p>
        <p class="packet">ID: <span id="rg2"></span></p>
      </div>  
</td></tr></table>
    </div>
  </div>

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

  document.getElementById("t"+obj.id).innerHTML = obj.temperature.toFixed(0);
  document.getElementById("h"+obj.id).innerHTML = obj.humidity.toFixed(0);
  document.getElementById("p"+obj.id).innerHTML = obj.pressure.toFixed(0);
  document.getElementById("u"+obj.id).innerHTML = obj.UV.toFixed(0);
  document.getElementById("c"+obj.id).innerHTML = obj.CO.toFixed(0); 
  document.getElementById("o"+obj.id).innerHTML = obj.CO2.toFixed(0);
  document.getElementById("g"+obj.id).innerHTML = obj.Gas.toFixed(0);
  document.getElementById("v"+obj.id).innerHTML = obj.VOC.toFixed(0);
  document.getElementById("s"+obj.id).innerHTML = obj.smoke.toFixed(0); 

  
  document.getElementById("rt"+obj.id).innerHTML = obj.readingId; 
  document.getElementById("rh"+obj.id).innerHTML = obj.readingId; 
  document.getElementById("rp"+obj.id).innerHTML = obj.readingId; 
  document.getElementById("ru"+obj.id).innerHTML = obj.readingId; 
  document.getElementById("rc"+obj.id).innerHTML = obj.readingId; 
  document.getElementById("ro"+obj.id).innerHTML = obj.readingId; 
  document.getElementById("rg"+obj.id).innerHTML = obj.readingId; 
  document.getElementById("rv"+obj.id).innerHTML = obj.readingId; 
  document.getElementById("rs"+obj.id).innerHTML = obj.readingId; 
  console.log(obj.readingId);

 }, false);
}
</script>
 </body>
</html>)rawliteral";

void setup()
{
  // Initialize Serial Monitor
  Serial.begin(9600);
  uint16_t ID = tft.readID();
  tft.begin(ID);

  // Set the device as a Station and Soft Access Point simultaneously
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
  float dewpoint = (myData.temperature - (100 - myData.humidity)/5);
  float heatIndex = 0.5 * (myData.temperature + 61.0) + ((myData.temperature - 68.0) * 1.2) + (myData.humidity * 0.094);
  float UV = 0;
  UV = (myData.UV/700);
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
  }
  else if (myData.id == 2)
  {
    tft.setTextColor(PINK);
  }
  else if (myData.id == 3)
  {
    tft.setTextColor(AQUA);
  }
  tft.print("Board ID:    ");
  tft.println(myData.id);
  tft.print("Temperature: ");
  tft.print(myData.temperature, 0);
  tft.println("*C");
  tft.print("Heat Index: ");
  tft.print(heatIndex, 0);
  tft.println();
  tft.print("Pressure:    ");
  tft.print(myData.pressure, 0);
  tft.println(" in Hg");
  tft.print("Humidity:    ");
  tft.print(myData.humidity, 0);
  tft.println("%");  
  tft.print("Dewpoint:    ");
  tft.print(dewpoint, 0);
  tft.println("*C");
  tft.print("Moisture:    ");
  tft.print(myData.moisture, 0);
  tft.println("%");
  tft.print("Smoke:       ");
  tft.println(myData.smoke, 0);
  tft.print("Uv index:    ");
  tft.print(UV, 0);
  tft.setCursor(1, 290);
  tft.print("Gas: ");
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
  tft.print("EtOH:");
  tft.println(myData.EtOH, 0);
  tft.setCursor(150, 340);
  tft.print("NH3: ");
  tft.println(myData.NH3, 0);
  tft.setCursor(150, 365);
  tft.print("NO2: "),
  tft.println(myData.NO2, 0);
  tft.print("Reading ID:  ");
  tft.println(myData.readingId);
  tft.println();

  delay(15000);
}