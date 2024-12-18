/*

   █████╗ ██╗  ██╗ ██████╗ ██╗
  ██╔══██╗╚██╗██╔╝██╔═══██╗██║
  ███████║ ╚███╔╝ ██║   ██║██║
  ██╔══██║ ██╔██╗ ██║   ██║██║
  ██║  ██║██╔╝ ██╗╚██████╔╝███████╗
  ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝

  ᓬ(• - •)ᕒ
  Axol sensing system.

   Code for Homehub Firmware. HomeHub connects to a mesh of sensors and serves as a relay for data coming to and from the sensor system.
   The device has a built in OLED screen for data visualization.

   Andres Rico - aricom@mit.edu

 */
#include <Arduino.h>
#include <WiFiManager.h>
#include <strings_en.h>

#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <esp_now.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "time.h"

#include <esp_wifi.h>
#include <Preferences.h>
#include "animations/draw.h"
#include "requests/retrievedata/retrievelocation.h"
#include "requests/homehub/homehub.h"
#include "globals/weather_location/get_complete_weather.h"
#include "requests/server_send/server_send.h"
#include "captiveportal/routes/routes.h"
#include "globals/globals.h"

/* FUNCTION HEADERS */
int get_buttons();
void get_time();
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
void get_system_stats();

/* HEADERS FOR 2.0v */
bool establishWiFiConnection();
void printNetworkInfo();
void onDemandPortal();

// Screen Variables
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Draw header to display animations on the screen
Draw draw(display);

#define WWIDTH 21 // Water Drop Size in pixels
#define WHEIGHT 30

/* GLOBAL VARIABLES FOR 2.0v*/
// WiFiManager wm;

void onDemandPortal()
{
  // Check connection...
  if (!establishWiFiConnection())
  {
    Serial.println("Not connected, opening Captive Portal...");
    return;
  }

  const int timeout = 300;
  wm.setConfigPortalTimeout(timeout);

  if (!wm.startConfigPortal("AxolOnDemand"))
  {
    Serial.println("Failed to connect");
  }

  Serial.println("onDemandPortal: Connected!");
}

bool establishWiFiConnection()
{
  return wm.autoConnect("Axol");
}

void printNetworkInfo()
{
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID()); // Nombre de la red WiFi

  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP()); // Dirección IP del ESP32

  Serial.print("Intensidad de señal (RSSI): ");
  Serial.println(WiFi.RSSI()); // Intensidad de la señal en dBm

  Serial.print("Dirección MAC: ");
  Serial.println(WiFi.macAddress()); // Dirección MAC del ESP32

  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP()); // Dirección IP del gateway

  Serial.print("Máscara de Subred: ");
  Serial.println(WiFi.subnetMask()); // Máscara de subred

  Serial.print("DNS Primario: ");
  Serial.println(WiFi.dnsIP()); // Dirección IP del DNS primario

  Serial.print("MAC del router: ");
  Serial.println(WiFi.BSSIDstr());
}

// WIFI Variables
const char *ssid = ""; // Change accordingly to connect to a WIFi network.
const char *password = "";

// Time Server Variables
String formattedDate;
String dayStamp;
String timeStamp;
const char *ntpServer = "pool.ntp.org";
long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;



// Control Variables
int bucket_count = 0;
int current_liters = 100;
bool received_message = false;

// ESP Now Communication Variables
typedef struct struct_message
{
  char id[50];
  int type;
  float data1;
  float data2;
  float data3;
  float data4;
  float data6;
  float data7;

  char ssid[32];
  char mac_addr[18];

} struct_message;

struct_message myData;

typedef struct pairing_data
{
  char ssid[32];
  char mac_addr[18];
} pairing_data;

struct pairing_data pairingData;

// Timed Event Variables - used to send
long current_time, elapsed_time, sent_time;
bool sending_climate = true;
bool sending_activity = false;

// Water Management Variables
int buckets, tanks, quality, envs, avail_storage, avail_liters;
float fill_percentage;
const char *dev_name;

int activity;

float up = 27;
float down = 15;
float right = 13;
float left = 14;
float a = 2;
float b = 4;

unsigned long previousMillis = 0; // WiFi Reconnecting Variables
unsigned long interval = 5000;

int get_buttons()
{ // Funtion returns int from 1 - 6

  /*
     1 - Up
     2 - Down
     3 - Right
     4 - Left
     5 - A
     6 - B
  */

  int touch_delay = 300;
  display.clearDisplay();

  if (!digitalRead(up))
  {
    // delay(touch_delay);
    // Serial.println(up_cap);
    return 1;
    // display.clearDisplay();
  }

  else if (!digitalRead(down))
  {
    // delay(touch_delay);
    return 2;
    // display.clearDisplay();
  }

  else if (!digitalRead(right))
  {
    // delay(touch_delay);
    // display.clearDisplay();
    return 3;
  }

  else if (!digitalRead(left))
  {
    // delay(touch_delay);
    return 4;
    // display.clearDisplay();
  }

  else if (!digitalRead(a))
  {
    // delay(touch_delay);
    return 5;
    // display.clearDisplay();
  }
  else if (!digitalRead(b))
  {
    // delay(touch_delay);
    return 6;
    // display.clearDisplay();
  }
}

void get_time()
{ // Functiuons queries server to get current time. Activates time screen.

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }

  // int num_month = month(timeinfo);
  // Serial.println(num_month);

  char buffer[80]; // Buffer to hold the formatted string. Adjust the size as needed.
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  formattedDate = String(buffer);
  // Serial.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf(" ");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.print("DATE: ");
  // Serial.println(dayStamp);

  // Extract time
  timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 3);
  Serial.print("HOUR: ");
  // Serial.println(timeStamp);
}



void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{ // Fucntion is activated when ESP receives data on ESPNOW.
  // It copies the received message to memory and sets the received message variable to True to indicate that there is new data to be sent to the server.
  memcpy(&myData, incomingData, sizeof(myData));
  received_message = true;
  Serial.println("SE RECIBIO UN DATO NUEVO DE ALGUN SENSOR");
}

void get_system_stats()
{ // Send Data to PHP server
  // Function takes command as argument and sends a POST request to server.

  HTTPClient http;

  String server_main = "http://blindspot.media.mit.edu/homehubweb/hhdash.php?id=" + WiFi.macAddress();

  http.begin(server_main); // construct the URL
  Serial.println(server_main);
  int httpCode = http.GET(); // send request

  if (httpCode > 0)
  { // If received weather JSON

    String payload = http.getString();
    Serial.println(payload);

    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    fill_percentage = doc["percentage"];
    buckets = doc["bucketNum"];
    tanks = doc["tankNum"];
    avail_storage = doc["availStorage"];
    avail_liters = doc["availLiters"];
    quality = doc["qualityNum"];
    envs = doc["envNum"];
    dev_name = doc["name"];
    lat = doc["lat"];
    lon = doc["lon"];

    // Free resources
    http.end();
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpCode);
  }
}

void connect_to_saved_wifi_network()
{
  if (!establishWiFiConnection())
  {
    Serial.println("Couldn't connect to the network");
  }
  else
  {
    Serial.println("Connected!");
    printNetworkInfo();
  }
}

void setup()
{
  // Begin
  Serial.begin(115200);
  Serial.println("Hello, I'm the Pairing Home Hub!");

  pinMode(up, INPUT_PULLUP);
  pinMode(down, INPUT_PULLUP);
  pinMode(right, INPUT_PULLUP);
  pinMode(left, INPUT_PULLUP);
  pinMode(a, INPUT_PULLUP);
  pinMode(b, INPUT_PULLUP);
  char saved_ssid[32];

  /* SETTING UP SENSOR PAIRING  */

  // Connecting to saved wifi network to get ssid
  connect_to_saved_wifi_network();
  strcpy(saved_ssid, WiFi.SSID().c_str());

  Serial.print("Saved SSID: ");
  Serial.println(saved_ssid);

  // Disconnecting in order to establish communication between sensors without router intervention
  WiFi.disconnect();

  WiFi.mode(WIFI_STA);
  // Setting wifi channel
  const int wifi_channel = 13;
  esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);

  WiFi.printDiag(Serial);

  Serial.println("Starting ESP NOW Communication");
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // ESP-NOW Broadcast MAC Address
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  // Sending pairing data struct
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.encrypt = false;

  if (!esp_now_is_peer_exist(broadcastAddress))
  {
    esp_now_add_peer(&peerInfo);
  }

  Serial.println("Canal wifi: ");
  Serial.println(WiFi.channel());
  Serial.println("------------------------------------------------");
  Serial.println("------------------------------------------------");
  Serial.println("------------------------------------------------");

  // Formatting MAC Address to XX:XX:XX:XX:XX:XX
  strcpy(pairingData.ssid, saved_ssid);
  strcpy(pairingData.mac_addr, WiFi.macAddress().c_str());

  esp_err_t result = esp_now_send(broadcastAddress, (const uint8_t *)&pairingData, sizeof(pairingData));
  Serial.println(result == ESP_OK ? "Datos enviados por broadcast" : "Error al enviar datos");

  Serial.println("------------------------------------------------");
  Serial.println("------------------------------------------------");
  Serial.println("------------------------------------------------");

  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("Debug: fin");

  // Continue with programmed tasks...

  // webserver for captive portal!!
  wm.setWebServerCallback(bindServerCallback);

  // Start-up OLED Screen
  Serial.println("Initializing Screen");
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    // for (;;); // Don't proceed, loop forever
  }
  // Clear screen buffer
  display.clearDisplay();

  // CS Logo Animation
  draw.drawCS(); // Draw's City Science Logo

  display.invertDisplay(true);
  // delay(3000);
  display.invertDisplay(false);
  // delay(3000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);

  // Display Welcome Text
  display.println("Inicializando HomeHub"); //"Welcome to Home  Hub"
  display.display();
  // delay(2000);

  WiFi.mode(WIFI_AP_STA); // Optional
  // WiFi.mode(WIFI_STA);
  display.clearDisplay();
  display.print("Conectando a:"); //"Connecting to Wifi"
  Serial.print("Connecting to WiFi");
  display.display();
  // delay(2000);

  connect_to_saved_wifi_network();

  // display.clearDisplay();
  // display.print("Conectado a: "); //"Connected to: "
  // display.println(ssid);
  // display.print("Mi IP "); //"My IP Address is "
  // display.println(WiFi.localIP());
  // display.println(WiFi.macAddress());
  // display.display();
  // Serial.println("");
  // Serial.println("WiFi connected successfully");
  // Serial.print("Got IP: ");
  // Serial.println(WiFi.localIP()); // Show ESP32 IP on serial
  // Serial.print("Mi MAC Address: ");
  // Serial.println(WiFi.macAddress());
  // Serial.print("Wi-Fi Channel: ");
  // Serial.println(WiFi.channel());

  // Serial.println("Starting ESP NOW Communication");
  // if (esp_now_init() != ESP_OK)
  // {
  //   Serial.println("Error initializing ESP-NOW");
  //   return;
  // }

  // esp_now_register_recv_cb(OnDataRecv);

  // Get weather and location.
  Serial.println("Getting Weather and Location");
  get_system_stats();
  String greeting = dev_name;
  weather_location::get_complete_weather(lat, lon);

  // Initialize time server
  Serial.println("Initializing Time Server");
  gmtOffset_sec = timezone; // +-3600 per hour difference against GMT.
  Serial.println("Time client started");

  sending_climate = true;
  server_send();
  Serial.println(greeting);
  display.clearDisplay();
  display.setCursor(0, 4);
  display.setTextSize(2);
  display.println(greeting);
  display.display();

  // delay(3000);
  draw.draw_maindash();

  display.print("Hello, I'm the Pairinng Home Hub 2.0!");
  Serial.println("Setup is complete!");
}

// GPIO27 -> Up
// GPIO15 -> Down
// GPIO13 -> Right
// GPIO14 -> Left
// GPIO4 -> B
// GPIO2 -> A

void loop()
{
  current_time = millis();
  elapsed_time = current_time - sent_time;
  if (elapsed_time >= 28800000)
  { // Updates and Sends Climate Data every 8 hours
    sending_climate = true;
    server_send();
    Serial.println("Sent Climate Data To Server");
  }

  if ((WiFi.status() != WL_CONNECTED) && (current_time - previousMillis >= interval))
  {
    Serial.println("Reconnecting to WiFi!");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = current_time;
  }

  if (received_message)
  {
    draw.draw_receiveddata();
    server_send();
    received_message = false;
  }
  if (get_buttons() == 1)
  { // Shows Clock Screen When Up Arrow is Pressed
    Serial.print("Presionaste: ");
    Serial.println(get_buttons());
    Serial.println("Borrando credenciales de Wi-Fi...");
    wm.resetSettings(); // Borra las credenciales de Wi-Fi
    ESP.restart();      // Reinicia el ESP32
    sending_activity = true;
    activity = 1;

    // Update weather and then draw the information
    get_time();
    weather_location::get_complete_weather(lat, lon);
    draw.draw_clockdash(timeStamp, dayStamp, city_name, main_temp, main_temp_max, main_temp_min, weather_0_icon);

    server_send();
    sending_activity = false;
  }
  if (get_buttons() == 2)
  { // Shows Water Dashboard
    Serial.print("Presionaste: ");
    Serial.println(get_buttons());
    sending_activity = true;
    activity = 2;

    // Update system stats and then draw the information
    get_system_stats();
    draw.draw_waterdash(fill_percentage, avail_liters, avail_storage);

    server_send();
    sending_activity = false;
  }
  if (get_buttons() == 3)
  { // Shows Virtual Axol
    Serial.print("Presionaste: ");
    Serial.println(get_buttons());
    sending_activity = true;
    activity = 3;

    // Updating system stats and drawing draw_axol
    get_system_stats();
    draw.draw_axol(fill_percentage);

    server_send();
    sending_activity = false;
  }
  if (get_buttons() == 4)
  { // Clear Display
    Serial.print("Presionaste: ");
    Serial.println(get_buttons());
    sending_activity = true;
    activity = 4;

    draw.draw_system(buckets, tanks, quality, envs);

    server_send();
    sending_activity = false;
  }
  if (get_buttons() == 5)
  { // Clear Display
    Serial.print("Presionaste: ");
    Serial.println(get_buttons());
    Serial.println("Abriendo portal en demanda");
    onDemandPortal();

    // sending_activity = true;
    // activity = 5;
    // display.clearDisplay();
    // server_send();
    // sending_activity = false;
  }
}
