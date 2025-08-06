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
#include <esp_now.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "time.h"

#include <esp_wifi.h>
#include <Preferences.h>
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

  // Setting wifi channel
  disconnectWiFi();
}

bool establishWiFiConnection()
{
  return wm.autoConnect("Axol");
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
bool received_message = false;

int activity;

float STU = 7;
float redLED = 4;
float blueLED = 5;

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

void redBlink()
{
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(redLED, LOW);
    delay(200);
    digitalWrite(redLED, HIGH);
    delay(200);
  }
}

void blueBlink()
{
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(blueLED, LOW);
    delay(200);
    digitalWrite(blueLED, HIGH);
    delay(200);
  }
}

void redPulse()
{
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 255; j++)
    {
      analogWrite(redLED, j);
      delay(5);
    }
    for (int j = 255; j > 0; j--)
    {
      analogWrite(redLED, j);
      delay(5);
    }
  }
}

void bluePulse()
{
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 255; j++)
    {
      analogWrite(blueLED, j);
      delay(5);
    }
    for (int j = 255; j > 0; j--)
    {
      analogWrite(blueLED, j);
      delay(5);
    }
  }
}

void lightsOff()
{
  digitalWrite(redLED, LOW);
  digitalWrite(blueLED, LOW);
}

void lightsOn()
{
  digitalWrite(redLED, HIGH);
  digitalWrite(blueLED, HIGH);
}

/// @brief Detects button presses and holds
/// @details Detects single, double, and triple presses, as well as long presses.

const unsigned long longPressTime = 5000; // 5 seconds hold
const unsigned long debounceTime = 50;    // Debounce time in ms
const unsigned long pressTimeout = 400;   // Max time between quick presses

bool buttonState = HIGH; // Active LOW (INPUT_PULLUP)
unsigned long pressStartTime = 0;
unsigned long lastPressTime = 0;
int pressCount = 0;
bool longPressDetected = false;

void detectButtonPress()
{
  bool currentState = digitalRead(STU);

  // Button Pressed
  if (currentState == LOW)
  {
    if (pressStartTime == 0)
    {
      pressStartTime = millis(); // Record when the press started
    }

    if ((millis() - pressStartTime > longPressTime) && !longPressDetected)
    {
      longPressDetected = true;
      Serial.println("Long Press Detected");
      digitalWrite(blueLED, HIGH);
      int touch_delay = 300;

      Serial.println("Opening Captive Portal on demand...");

      // Open captive portal on demand
      onDemandPortal();

      lightsOff();
    }
  }

  // Button Released
  else
  {
    if (pressStartTime > 0 && !longPressDetected)
    {
      unsigned long pressDuration = millis() - pressStartTime;

      if (pressDuration < longPressTime)
      {
        pressCount++; // Count quick presses
        lastPressTime = millis();
      }
    }

    pressStartTime = 0;
    longPressDetected = false;
  }

  // Detect quick presses
  if (pressCount > 0 && (millis() - lastPressTime > pressTimeout))
  {
    if (pressCount == 1)
    {
      Serial.println("Single Press Detected");
      blueBlink();
      Serial.println("BEFORE");
      WiFi.printDiag(Serial);
      Serial.println("-----------------");
      broadcast();
      Serial.println("AFTER");
      WiFi.printDiag(Serial);
      Serial.println("-----------------");
      lightsOff();
    }
    else if (pressCount == 2)
    {
      Serial.println("Double Press Detected");
      redBlink();
      lightsOff();
    }
    else if (pressCount == 3)
    {
      Serial.println("Triple Press Detected");
      lightsOn();

      // switching to debug endpoints
      setDebugEndpoints();
      
      delay(1000);
      lightsOff();
      
    }
    pressCount = 0; // Reset after detection
  }
}

void setup()
{
  // Begin
  Serial.begin(460800);
  delay(1000);
  Serial.println("Hello, I'm Homehub Mini!");

  pinMode(STU, INPUT_PULLUP);
  pinMode(redLED, OUTPUT);
  pinMode(blueLED, OUTPUT);

  bool setup_pressed = digitalRead(STU);

  // Initializing endpoints
  initEndpoints();

  /// webserver for captive portal!!
  Serial.println("Activating root for captive-portal");
  wm.setWebServerCallback(bindServerCallback);

  // WiFi.mode(WIFI_AP_STA);

  Serial.println("[main.cpp: setup] Setting wifi channel");
  setWiFiChannel();

  Serial.print("[main.cpp: setup] wifi channel is: ");
  Serial.println(WiFi.channel());

  Serial.print("[main.cpp: setup] Mi MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Get weather and location.
  Serial.println("[main.cpp: setup] Getting Weather and Location");

  // Open captive portal if button pressed in startup
  if (setup_pressed == LOW)
  {
    Serial.println("[main.cpp: setup] STU Pressed at start.");
    digitalWrite(blueLED, HIGH);
    Serial.println("[main.cpp: setup] Opening Captive Portal on demand...");

    // Open captive portal on demand
    onDemandPortal();

    lightsOff();
  }
  else
  {
    Serial.println("[main.cpp: setup] Button not pressed, connecting to saved network");
  }

  // connect to the internet
  if (!connectToSavedNetwork())
  {
    Serial.println("[main.cpp: setup] Couldn't connect to the internet for Captive Portal on demand.");
  }

  homehub::getSystemStats();

  // Disconnect from the internet
  disconnectWiFi();

  String greeting = waterManager.dev_name;
  // weather_location::get_complete_weather(weather_location::lat, weather_location::lon);
  server_send();

  // Disconnect from the internet
  disconnectWiFi();

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

  /*Serial.println("----------");
  Serial.println("[main.cpp: setup] Peer address:");
  printMacAddress(peerInfo.peer_addr);
  Serial.println("----------");
  Serial.print("[main.cpp: setup] Peer channel:");
  Serial.println(peerInfo.channel);
  Serial.print("[main.cpp: setup] Peer encrypt:");
  Serial.println(peerInfo.encrypt);
  Serial.print("[main.cpp: setup] Peer ifidx:");
  Serial.println(peerInfo.ifidx);
  Serial.println("[main.cpp: setup] Peer info registered.");*/

  // Setting wifi channel
  Serial.println("[main.cpp: setup] WiFi info");
  WiFi.printDiag(Serial);
  Serial.println("****************");

  Serial.println("Setup is complete!");
}

// TESTING VARIABLES FOR TIME
long startTime = 0;
long intervalTime = 1000 * 10; // 10 seconds

void loop()
{

  detectButtonPress();

  //If channel is not 13, turn red LED on
  if (WiFi.channel() != wifi_channel)
  {
    digitalWrite(redLED, HIGH);
    Serial.println("[main.cpp: loop] WiFi channel is not 13");
  }
  /*else
  {
    digitalWrite(redLED, LOW);
    Serial.println("[main.cpp: loop] WiFi channel is 13");
  }*/

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
    redBlink();
    server_send();
    received_message = false;

    // Disconnect from the internet
    disconnectWiFi();
    lightsOff();
  }

  detectButtonPress();
}