/*

   █████╗ ██╗  ██╗ ██████╗ ██╗     ██╗  ██╗ ██████╗ ███╗   ███╗███████╗    ██╗  ██╗██╗   ██╗██████╗
  ██╔══██╗╚██╗██╔╝██╔═══██╗██║     ██║  ██║██╔═══██╗████╗ ████║██╔════╝    ██║  ██║██║   ██║██╔══██╗
  ███████║ ╚███╔╝ ██║   ██║██║     ███████║██║   ██║██╔████╔██║█████╗      ███████║██║   ██║██████╔╝ 
  ██╔══██║ ██╔██╗ ██║   ██║██║     ██╔══██║██║   ██║██║╚██╔╝██║██╔══╝      ██╔══██║██║   ██║██╔══██╗
  ██║  ██║██╔╝ ██╗╚██████╔╝███████╗██║  ██║╚██████╔╝██║ ╚═╝ ██║███████╗    ██║  ██║╚██████╔╝██████╔╝ 
  ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝    ╚═╝  ╚═╝ ╚═════╝ ╚═════╝ 
                 
                
               
                
               


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
#include "globals/weather_location/weather_location.h"

#include "requests/server_send/server_send.h"
#include "captiveportal/routes/routes.h"
#include "globals/globals.h"
#include "globals/management/management.h"
#include "globals/timeserver/timeserver.h"

/* FUNCTION HEADERS */
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void broadcast();
bool connectToSavedNetwork();
void printMacAddress(const uint8_t *mac);

/* HEADERS FOR 2.0v */
void onDemandPortal();
void disconnectWiFi();
void setWiFiChannel();

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

// ESP-NOW Broadcast MAC Address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const int wifi_channel = 13;
bool data_sent = false;

void setWiFiChannel()
{
  Serial.println("[main.cpp: setWiFiChannel] switching to WIFI_AP_STA mode");
  WiFi.mode(WIFI_AP_STA);

  // Set wifi channel to 13
  Serial.println("[main.cpp: setWiFiChannel] Setting wifi channel to 13");
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  delay(200);
}

void disconnectWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("[main.cpp] Disconnecting from WiFi...");
    WiFi.disconnect();
  }

  setWiFiChannel();

  Serial.println("[main.cpp] TEST Disconnected WiFi config:");
  WiFi.printDiag(Serial);

  Serial.print("[main.cpp] WiFi status: ");
  Serial.println(WiFi.status() == WL_CONNECTED ? "Connected to WiFi" : "NOT connected to WiFi");
}

void onDemandPortal()
{
  // Check connection...
  if (!connectToSavedNetwork())
  {
    Serial.println("[main.cpp] Couldn't connect to the internet for Captive Portal on demand.");
  }

  const int timeout = 300;
  wm.setConfigPortalTimeout(timeout);

  if (!wm.startConfigPortal("AxolOnDemand"))
  {
    Serial.println("[main.cpp] Captive Portal on demand Failed to open");
  }

  Serial.println("[main.cpp] Closing Captive Portal on demand");

  // Disconnect WiFi after Captive Portal
  disconnectWiFi();
}

void printMacAddress(const uint8_t *mac)
{
  Serial.print("[main.cpp: printMacAddress] Printing mac address: ");
  for (int i = 0; i < 6; i++)
  {
    if (i > 0)
      Serial.print(":");
    Serial.print(mac[i], HEX);
  }
  Serial.println();
}

void broadcast()
{
  /* SETTING UP SENSOR PAIRING  */
  ////////
  WiFi.mode(WIFI_STA);
  ////////////
  Serial.println("[main.cpp: broadcast] WiFi config: ");
  WiFi.printDiag(Serial);
  Serial.println("----------------");

  // // // strcpy(pairingData.ssid, saved_ssid);
  strcpy(pairingData.mac_addr, WiFi.macAddress().c_str());
  delay(100);

  // Send message via ESP-NOW
  Serial.println("[send_espnow] Sending data via ESP-NOW...");
  Serial.println("----------");
  Serial.println("[send_espnow] Sending to... ");
  printMacAddress(broadcastAddress);
  Serial.println("----------");

  esp_err_t result = esp_now_send(broadcastAddress, (const uint8_t *)&pairingData, sizeof(pairingData));

  // Espera confirmación o timeout
  unsigned long start = millis();
  while (!data_sent && millis() - start < 200)
  {
    delay(10);
  }

  if (!data_sent)
  {
    Serial.println("[send_espnow] No confirmación de envío");
  }

  if (result != ESP_OK)
  {
    Serial.print("[send_espnow] Failed to send data, error code: ");
    Serial.println(result);
  }

  delay(100);
  Serial.println(result == ESP_OK ? "Datos enviados por broadcast" : "Error al enviar datos");

  Serial.println("Broadcasting Complete");
  delay(200);
}

// Control Variables
int bucket_count = 0;
int current_liters = 100;
bool received_message = false;

float up = 27;
float down = 15;
float right = 13;
float left = 14;
float a = 2;
float b = 4;

unsigned long previousMillis = 0; // WiFi Reconnecting Variables
unsigned long interval = 5000;

void showDataReceived(const uint8_t *mac)
{

  Serial.print("[main.cpp: showDataReceived] received_message status: ");
  Serial.println(received_message);

  // Show data packet received
  Serial.print("Received from: ");
  for (int i = 0; i < 6; i++)
  {
    if (i > 0)
      Serial.print(":");
    if (mac[i] < 0x10)
      Serial.print("0");

    Serial.print(mac[i], HEX);
  }
  Serial.println();

  Serial.print("Data: ");
  Serial.println("myData.id: ");
  Serial.println(myData.id);
  Serial.println("myData.type: ");
  Serial.println(myData.type);
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{ // Function is activated when ESP receives data on ESPNOW.
  // It copies the received message to memory and sets the received message variable to True to indicate that there is new data to be sent to the server.
  memcpy(&myData, incomingData, sizeof(myData));
  delay(100);
  received_message = true;
  delay(100);
  Serial.println("[main.cpp: OnDataRecv] SE RECIBIO UN DATO NUEVO DE ALGUN SENSOR");

  showDataReceived(mac);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\n[main.cpp: OnDataSent] Last Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  data_sent = true;
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

  // webserver for captive portal!!
  Serial.println("Activating root for captive-portal");
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

  // WiFi.mode(WIFI_STA);
  // WiFi.mode(WIFI_AP_STA);

  // display.print("Conectando a:"); //"Connecting to Wifi"
  // Serial.println("Connecting to WiFi");
  // display.display();
  // delay(2000);

  // display.clearDisplay();
  // display.setTextSize(1);
  // display.setTextColor(WHITE);
  // display.setCursor(0, 10);
  // // display.println("Red abierta: ");
  // // display.println("Axol");
  // display.display();

  Serial.println("[main.cpp: setup] Setting wifi channel");
  setWiFiChannel();

  Serial.print("[main.cpp: setup] wifi channel is: ");
  Serial.println(WiFi.channel());

  Serial.print("[main.cpp: setup] Mi MAC Address: ");
  Serial.println(WiFi.macAddress());

  display.clearDisplay();
  display.print("Canal de WiFi: ");
  display.println(WiFi.channel());

  display.print("MAC Address: "); //"My IP Address is "
  display.println(WiFi.macAddress());

  display.display();
  // Serial.println("");
  // Serial.print("Got IP: ");
  // Serial.println(WiFi.localIP()); // Show ESP32 IP on serial
  // Serial.print("Mi MAC Address: ");
  // Serial.println(WiFi.macAddress());
  // Serial.print("Wi-Fi Channel: ");
  // Serial.println(WiFi.channel());

  // Get weather and location.
  Serial.println("Getting Weather and Location");

  // connect to the internet
  if (!connectToSavedNetwork())
  {
    Serial.println("[main.cpp: setup] Couldn't connect to the internet for Captive Portal on demand.");
  }

  homehub::getSystemStats();

  // Disconnect from the internet
  disconnectWiFi();
  // String greeting = waterManager.dev_name;

  server_send(); // initial value for eventVariables.sending_climate = true

  // Disconnect from the internet
  disconnectWiFi();

  // Initialize time server
  // Serial.println("Initializing Time Server");
  // timeserver::gmtOffset_sec = timezone; // +-3600 per hour difference against GMT.
  // Serial.println("Time client started");

  // eventVariables.sending_climate = true;
  // delay(200);
  // server_send();
  // Serial.println(greeting);
  // display.clearDisplay();
  // display.setCursor(0, 4);
  // display.setTextSize(2);
  // display.println(greeting);
  // display.display();

  // delay(3000);
  draw.draw_maindash();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 11);
  display.println("Hello, I'm the Pairing Home Hub 2.0!");
  display.display();
  delay(5000);
  display.clearDisplay();
  display.display();

  Serial.println("Starting ESP NOW Communication");
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // registering callback functions
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  // Sending pairing data struct
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = wifi_channel;
  peerInfo.encrypt = false;

  if (!esp_now_is_peer_exist(broadcastAddress))
  {
    if (esp_now_add_peer(&peerInfo) == ESP_OK)
    {
      Serial.println("[setup]: si se agrego el peer");
    }
    else
    {
      Serial.println("[setup] NO SE PUDO AGREGAR EL PEER");
    }
  }

  Serial.println("----------");
  Serial.println("[main.cpp: setup] Peer address:");
  printMacAddress(peerInfo.peer_addr);
  Serial.println("----------");
  Serial.print("[main.cpp: setup] Peer channel:");
  Serial.println(peerInfo.channel);
  Serial.print("[main.cpp: setup] Peer encrypt:");
  Serial.println(peerInfo.encrypt);
  Serial.print("[main.cpp: setup] Peer ifidx:");
  Serial.println(peerInfo.ifidx);
  Serial.println("[main.cpp: setup] Peer info registered.");

  // Checking wifi configuration
  Serial.println("[main.cpp: setup] WiFi info");
  WiFi.printDiag(Serial);
  Serial.println("****************");

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
  eventVariables.current_time = millis();
  eventVariables.elapsed_time = eventVariables.current_time - eventVariables.sent_time;
  if (eventVariables.elapsed_time >= 28800000)
  { // Updates and Sends Climate Data every 8 hours

    // Reconnect to the internet to send data received
    eventVariables.sending_climate = true;
    server_send();

    // Disconnect from the internet
    disconnectWiFi();
  }

  if (received_message)
  {
    // Reconnect to the internet to send data received
    server_send();
    received_message = false;

    // Disconnect from the internet
    disconnectWiFi();

    draw.draw_receiveddata();
  }

  if (!digitalRead(up))
  {
    /* THIS SECTION IS DEPRECATED FOR THE NEWER VERSION  */

    // Shows Clock Screen When Up Arrow is Pressed
    int touch_delay = 300;
    // Update weather and then draw the information
    timeserver::get_time();
    weather_location::get_complete_weather(weather_location::lat, weather_location::lon);
    draw.draw_clockdash(timeserver::timeStamp, timeserver::dayStamp, weather_location::city_name, weather_location::main_temp, weather_location::main_temp_max, weather_location::main_temp_min, weather_location::weather_0_icon);

    server_send();
    eventVariables.sending_activity = false;
  }

  if (!digitalRead(down))
  {
    /* THIS SECTION IS DEPRECATED FOR THE NEWER VERSION  */
    // Shows Water Dashboard
    int touch_delay = 300;
    display.clearDisplay();

    eventVariables.sending_activity = true;
    eventVariables.activity = 2;

    // Update system stats and then draw the information
    homehub::getSystemStats();
    draw.draw_waterdash(waterManager.fill_percentage, waterManager.avail_liters, waterManager.avail_storage);

    server_send();
    eventVariables.sending_activity = false;
  }
  if (!digitalRead(right))
  {

    // DEBUG: switch to debug mode to see if the endpoints change
    Serial.println("[main.cpp] Switching to debug mode");
    isDebugMode = true;
  }

  if (!digitalRead(left))
  {
    /* THIS SECTION IS DEPRECATED FOR THE NEWER VERSION  */
    // Clear Display
    int touch_delay = 300;
    display.clearDisplay();

    eventVariables.sending_activity = true;
    eventVariables.activity = 4;

    draw.draw_system(waterManager.buckets, waterManager.tanks, waterManager.quality, waterManager.envs);

    server_send();
    eventVariables.sending_activity = false;
  }
  if (!digitalRead(a))
  { // Clear Display
    int touch_delay = 300;
    display.clearDisplay();

    Serial.println("Opening Captive Portal on demand...");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("Red abierta: ");
    display.println("AxolOnDemand");
    display.display();

    onDemandPortal();

    display.clearDisplay();
    display.display();
  }
  if (!digitalRead(b))
  { // Clear Display
    int touch_delay = 300;
    display.clearDisplay();

    Serial.println("BEFORE");
    WiFi.printDiag(Serial);
    Serial.println("-----------------");
    broadcast();
    Serial.println("AFTER");
    WiFi.printDiag(Serial);
    Serial.println("-----------------");
  }
}