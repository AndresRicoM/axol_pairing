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
#include "requests/system/systemStats.h"
#include "globals/timeserver/timeserver.h"

/* FUNCTION HEADERS */
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void broadcast();
bool connectToSavedNetwork();
void printMacAddress(const uint8_t *mac);

/* HEADERS FOR 2.0v */
// bool establishWiFiConnection();      ya no se usa
void printNetworkInfo();
void onDemandPortal();
void disconnectWiFi();
void set_wifi_channel();

// ESP-NOW Broadcast MAC Address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const int wifi_channel = 13;
bool data_sent = false;

void set_wifi_channel()
{
  Serial.println("Setting wifi channel");

  // Set wifi channel to 13
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  delay(200);
}

void disconnectWiFi()
{
  Serial.println("[main.cpp] Switching to WIFI_STA mode...");
  WiFi.mode(WIFI_STA);
  Serial.println("[main.cpp] Disconnecting from WiFi...");

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("[main.cpp] Disconnecting from WiFi...");
    WiFi.disconnect();
  }

  set_wifi_channel();

  Serial.println("[main.cpp] Disconnected WiFi config:");
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

  // // Disconnecting in order to establish communication between sensors without router intervention
  disconnectWiFi();

  esp_now_deinit();  // <- Limpia la instancia anterior
  if (esp_now_init() != ESP_OK) {
    Serial.println("[main.cpp: broadcast] Error al inicializar ESP-NOW");
    return;
  }

  Serial.println("[debug] ESP-NOW re-initialized successfully.");

  // delay(100);
  // WiFi.mode(WIFI_STA);
  // delay(100);

  // Setting wifi channel
  esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);

  Serial.println("[main.cpp: broadcast] WiFi config: ");
  WiFi.printDiag(Serial);
  Serial.println("----------------");

  // Formatting MAC Address to XX:XX:XX:XX:XX:XX

  // // // strcpy(pairingData.ssid, saved_ssid);
  strcpy(pairingData.mac_addr, WiFi.macAddress().c_str());
  delay(100);

  Serial.println("[main.cpp: broadcast] Registering peer...");
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = wifi_channel; // Set the channel to the same as the sender
  peerInfo.ifidx = WIFI_IF_STA;    // Station Interface
  peerInfo.encrypt = false;

  Serial.println("----------");
  Serial.println("[send_espnow] Peer address:");
  printMacAddress(peerInfo.peer_addr);
  Serial.println("----------");
  Serial.print("[send_espnow] Peer channel:");
  Serial.println(peerInfo.channel);
  Serial.print("[send_espnow] Peer encrypt:");
  Serial.println(peerInfo.encrypt);
  Serial.print("[send_espnow] Peer ifidx:");
  Serial.println(peerInfo.ifidx);
  Serial.println("[send_espnow] Peer info registered.");

  // Add peer
  esp_err_t addPeerResult = esp_now_add_peer(&peerInfo);
  if (addPeerResult != ESP_OK)
  {
    Serial.print("[main.cpp: broadcast] Failed to add peer, error code: ");
    Serial.println(addPeerResult);
  }

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

  Serial.println("Returning WiFi Mode to WIFI_STA)");
  delay(100);

  Serial.println("Broadcasting Complete");
  delay(3000);
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
  // Show data packet received
  Serial.print("Received from: ");
  for (int i = 0; i < 6; i++)
  {
    Serial.print(mac[i], HEX);
    if (i < 5)
      Serial.print(":");
  }

  Serial.print(" Data: ");
  Serial.println("myData.id: ");
  Serial.println(myData.id);
  Serial.println("myData.type: ");
  Serial.println(myData.type);
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{ // Function is activated when ESP receives data on ESPNOW.
  // It copies the received message to memory and sets the received message variable to True to indicate that there is new data to be sent to the server.
  memcpy(&myData, incomingData, sizeof(myData));
  received_message = true;
  Serial.println("[main.cpp: OnDataRecv] SE RECIBIO UN DATO NUEVO DE ALGUN SENSOR");

  showDataReceived(mac);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\n[main.cpp: OnDataSent] Last Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  data_sent = true;
}

bool connectToSavedNetwork()
{

  // WiFi Reconnecting Variables
  const unsigned long timeout = 30000; // Tiempo de espera de 30 segundos
  unsigned long startAttemptTime = millis();
  int attemptCounter = 0;
  int totalReconnectAttempts = 0;

  Serial.println("[main.cpp] Connecting to saved wifi network...");

  Serial.println("[main.cpp] Initialize WiFi...");
  WiFi.begin();
  Serial.println("[main.cpp] Setting WIFI_STA...");
  WiFi.mode(WIFI_STA);
  Serial.println("[main.cpp] WiFi information after initialization:");
  WiFi.printDiag(Serial);

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout && totalReconnectAttempts < 3)
  {
    delay(1000);
    Serial.println("[main.cpp] Trying to connect to saved network...");

    attemptCounter++;
    if (attemptCounter % 5 == 0)
    { // every 5 attemps, restart wifi connection
      totalReconnectAttempts++;
      Serial.println("[main.cpp] Restarting WiFi connection...");
      WiFi.disconnect();
      delay(100);
      WiFi.reconnect();
    }
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    // Wait a two seconds to get IP Address and other configurations from AP
    delay(1000);
    Serial.println("[main.cpp] Connected to wifi successfully");
    Serial.println("[main.cpp] WiFi information:");
    printNetworkInfo();
  }
  else
  {
    Serial.println("[main.cpp] Failed to connect to wifi within the timeout period");
    return false;
  }

  return true;
  Serial.println("-------------------");
}

void redBlink() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(redLED, LOW);
    delay(200);
    digitalWrite(redLED, HIGH);
    delay(200);
  }
}

void blueBlink() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(blueLED, LOW);
    delay(200);
    digitalWrite(blueLED, HIGH);
    delay(200);
  }
}

void redPulse() {
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 255; j++) {
      analogWrite(redLED, j);
      delay(5);
    }
    for (int j = 255; j > 0; j--) {
      analogWrite(redLED, j);
      delay(5);
    }
  }
}

void bluePulse() {
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 255; j++) {
      analogWrite(blueLED, j);
      delay(5);
    }
    for (int j = 255; j > 0; j--) {
      analogWrite(blueLED, j);
      delay(5);
    }
  }
}

void lightsOff() {
  digitalWrite(redLED, LOW);
  digitalWrite(blueLED, LOW);
}

void lightsOn() {
  digitalWrite(redLED, HIGH);
  digitalWrite(blueLED, HIGH);
}

/// @brief Detects button presses and holds
/// @details Detects single, double, and triple presses, as well as long presses.

const unsigned long longPressTime = 5000; // 5 seconds hold
const unsigned long debounceTime = 50;  // Debounce time in ms
const unsigned long pressTimeout = 400; // Max time between quick presses

bool buttonState = HIGH;   // Active LOW (INPUT_PULLUP)
unsigned long pressStartTime = 0;
unsigned long lastPressTime = 0;
int pressCount = 0;
bool longPressDetected = false;

void detectButtonPress() {
    bool currentState = digitalRead(STU);

    // Button Pressed
    if (currentState == LOW) {
        if (pressStartTime == 0) {
            pressStartTime = millis(); // Record when the press started
        }
        
        if ((millis() - pressStartTime > longPressTime) && !longPressDetected) {
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
    else {
        if (pressStartTime > 0 && !longPressDetected) {
            unsigned long pressDuration = millis() - pressStartTime;

            if (pressDuration < longPressTime) {
                pressCount++;  // Count quick presses
                lastPressTime = millis();
            }
        }
        
        pressStartTime = 0;
        longPressDetected = false;
    }

    // Detect quick presses
    if (pressCount > 0 && (millis() - lastPressTime > pressTimeout)) {
        if (pressCount == 1) {
            Serial.println("Single Press Detected");
            lightsOn();
            delay(1000);
            lightsOff();
        } else if (pressCount == 2) {
            Serial.println("Double Press Detected");
            redBlink();
            lightsOff();

        } else if (pressCount == 3) {
            Serial.println("Triple Press Detected");
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
        pressCount = 0; // Reset after detection
    }
}

void setup()
{
  // Begin
  Serial.begin(460800);
  Serial.println("Hello, I'm Homehub Mini!");

  pinMode(STU, INPUT_PULLUP);
  pinMode(redLED, OUTPUT);
  pinMode(blueLED, OUTPUT);

  /// webserver for captive portal!!
  Serial.println("Activating root for captive-portal");
  wm.setWebServerCallback(bindServerCallback);

  WiFi.mode(WIFI_AP_STA);
  Serial.print("Connecting to WiFi");

  // Trying to connect to the internet
  WiFi.begin();

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("[main.cpp] connected to wifi");
  }
  else
  {
    Serial.println("[main.cpp] NOT connected to wifi");
  }
 
  Serial.println("");
  Serial.println("WiFi connected successfully");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP()); // Show ESP32 IP on serial
  Serial.print("Mi MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  // Get weather and location.
  Serial.println("Getting Weather and Location");
  homehub::getSystemStats();
  String greeting = waterManager.dev_name;
  weather_location::get_complete_weather(weather_location::lat, weather_location::lon);

  // Initialize time server
  Serial.println("Initializing Time Server");
  //timeserver::gmtOffset_sec = timezone; // +-3600 per hour difference against GMT.
  Serial.println("Time client started");

  eventVariables.sending_climate = true;
  delay(200);
  server_send();
  Serial.println(greeting);
    
  Serial.println("Starting ESP NOW Communication");
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Disconnect from the internet
  disconnectWiFi();

  esp_now_register_recv_cb(OnDataRecv);

  // Sending pairing data struct
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.encrypt = false;

  if (!esp_now_is_peer_exist(broadcastAddress))
  {
    esp_now_add_peer(&peerInfo);
  }

  Serial.println("Setup is complete!");
}

// TESTING VARIABLES FOR TIME
long startTime = 0;
long intervalTime = 1000 * 10; // 10 seconds

void loop() { 
  
  detectButtonPress();

  eventVariables.current_time = millis();
  eventVariables.elapsed_time = eventVariables.current_time - eventVariables.sent_time;
  
  if (eventVariables.elapsed_time >= 28800000)
  { // Updates and Sends Climate Data every 8 hours

    // reconnecting to wifi

    if (!connectToSavedNetwork())
    {
      Serial.println("[main.cpp] Couldn't connect to the internet.");
      Serial.println("[main.cpp] Restarting Homehub...");
      ESP.restart();
    }
    else
    {
      eventVariables.sending_climate = true;
      server_send();
      Serial.println("Sent Climate Data To Server");
      disconnectWiFi();
    }
  }

  if (received_message)
  {
    // Reconnect to the internet to send data received

    if (!connectToSavedNetwork())
    {
      Serial.println("[main.cpp] Couldn't connect to the internet.");
      Serial.println("[main.cpp] Restarting Homehub...");
      ESP.restart();
    }
    else
    {
      server_send();
      received_message = false;

      // Disconnect from the internet
      disconnectWiFi();
    }
  }

  /*if ((WiFi.status() != WL_CONNECTED) && (eventVariables.current_time - previousMillis >= interval))
  {
    Serial.println("Reconnecting to WiFi!");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = eventVariables.current_time;
  }*/



  detectButtonPress();
  
}