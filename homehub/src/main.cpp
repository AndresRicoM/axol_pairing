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
#include "globals/weather_location/weather_location.h"

#include "requests/server_send/server_send.h"
#include "captiveportal/routes/routes.h"
#include "globals/globals.h"
#include "globals/management/management.h"
#include "requests/system/systemStats.h"
#include "globals/timeserver/timeserver.h"

/* FUNCTION HEADERS */
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
void broadcast();
void connect_to_saved_wifi_network();

/* HEADERS FOR 2.0v */
bool establishWiFiConnection();
void printNetworkInfo();
void onDemandPortal();

/* GLOBAL VARIABLES FOR 2.0v*/
// WiFiManager wm;

// ESP-NOW Broadcast MAC Address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

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

void broadcast()
{
  char saved_ssid[32];

  /* SETTING UP SENSOR PAIRING  */

  // Connecting to saved wifi network to get ssid
  connect_to_saved_wifi_network();
  strcpy(saved_ssid, WiFi.SSID().c_str());

  Serial.print("Saved SSID: ");
  Serial.println(saved_ssid);

  // Disconnecting in order to establish communication between sensors without router intervention
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_STA);
  delay(100);
  // Setting wifi channel
  const int wifi_channel = 13;
  esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);

  WiFi.printDiag(Serial);

  // Formatting MAC Address to XX:XX:XX:XX:XX:XX
  strcpy(pairingData.ssid, saved_ssid);
  strcpy(pairingData.mac_addr, WiFi.macAddress().c_str());
  delay(100);

  esp_err_t result = esp_now_send(broadcastAddress, (const uint8_t *)&pairingData, sizeof(pairingData));

  delay(100);
  Serial.println(result == ESP_OK ? "Datos enviados por broadcast" : "Error al enviar datos");

  Serial.println("Returning WiFi Mode to WiFi_AP_STA");
  WiFi.mode(WIFI_AP_STA);
  delay(100);

  Serial.println("Broadcasting Complete");
  delay(3000);
}

// Control Variables
int bucket_count = 0;
int current_liters = 100;
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
{ // Fucntion is activated when ESP receives data on ESPNOW.
  // It copies the received message to memory and sets the received message variable to True to indicate that there is new data to be sent to the server.
  memcpy(&myData, incomingData, sizeof(myData));
  received_message = true;
  Serial.println("SE RECIBIO UN DATO NUEVO DE ALGUN SENSOR");

  showDataReceived(mac);
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
            bluePulse();
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
            redPulse();
            Serial.println("Triple Press Detected");
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

  //digitalWrite(redLED, HIGH);
  //digitalWrite(blueLED, HIGH);

  // webserver for captive portal!!
  Serial.println("Activating root for captive-portal");
  wm.setWebServerCallback(bindServerCallback);


  //display.println("Inicializando HomeHub"); //"Welcome to Home  Hub"


  WiFi.mode(WIFI_AP_STA);
  // WiFi.mode(WIFI_STA);
  //display.print("Conectando a:"); //"Connecting to Wifi"
  Serial.print("Connecting to WiFi");

  //display.println("Red abierta: ");
  //display.println("Axol");
  
  connect_to_saved_wifi_network();

  //display.print("Conectado a: "); //"Connected to: "
  //display.print("Mi IP "); //"My IP Address is "
 
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
  get_system_stats();
  String greeting = waterManager.dev_name;
  weather_location::get_complete_weather(lat, lon);

  // Initialize time server
  Serial.println("Initializing Time Server");
  timeserver::gmtOffset_sec = timezone; // +-3600 per hour difference against GMT.
  Serial.println("Time client started");

  eventVariables.sending_climate = true;
  server_send();
  Serial.println(greeting);
  
  //display.println("Hello, I'm the Pairing Home Hub 2.0!");
  
  Serial.println("Starting ESP NOW Communication");
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

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

void loop()
{ 
  detectButtonPress();
  eventVariables.current_time = millis();
  eventVariables.elapsed_time = eventVariables.current_time - eventVariables.sent_time;
  if (eventVariables.elapsed_time >= 28800000)
  { // Updates and Sends Climate Data every 8 hours
    eventVariables.sending_climate = true;
    server_send();
    Serial.println("Sent Climate Data To Server");
  }

  if ((WiFi.status() != WL_CONNECTED) && (eventVariables.current_time - previousMillis >= interval))
  {
    Serial.println("Reconnecting to WiFi!");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = eventVariables.current_time;
  }

  if (received_message)
  {
    server_send();
    received_message = false;
  }

  /*if (!digitalRead(up))
  { // Shows Clock Screen When Up Arrow is Pressed
    int touch_delay = 300;
    // Update weather and then draw the information
    timeserver::get_time();
    weather_location::get_complete_weather(lat, lon);
    draw.draw_clockdash(timeserver::timeStamp, timeserver::dayStamp, city_name, main_temp, main_temp_max, main_temp_min, weather_0_icon);

    server_send();
    eventVariables.sending_activity = false;
  }*/

  /*if (!digitalRead(down))
  { // Shows Water Dashboard
    int touch_delay = 300;
    display.clearDisplay();

    eventVariables.sending_activity = true;
    activity = 2;

    // Update system stats and then draw the information
    get_system_stats();
    draw.draw_waterdash(waterManager.fill_percentage, waterManager.avail_liters, waterManager.avail_storage);

    // server_send();
    eventVariables.sending_activity = false;
  }*/
  /*if (!digitalRead(right))
  { // Shows Virtual Axol
    int touch_delay = 300;
    display.clearDisplay();

    eventVariables.sending_activity = true;
    activity = 3;

    // Updating system stats and drawing draw_axol
    get_system_stats();
    draw.draw_axol(waterManager.fill_percentage);

    // server_send();
    eventVariables.sending_activity = false;
  }*/

  /*if (!digitalRead(left))
  { // Clear Display
    int touch_delay = 300;
    display.clearDisplay();

    eventVariables.sending_activity = true;
    activity = 4;

    draw.draw_system(waterManager.buckets, waterManager.tanks, waterManager.quality, waterManager.envs);

    // server_send();
    eventVariables.sending_activity = false;
  }*/
  /*if (!digitalRead(a))
  { // Clear Display
    int touch_delay = 300;
    display.clearDisplay();

    Serial.println("Abriendo portal en demanda");
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

    // eventVariables.sending_activity = true;
    // activity = 5;
    // display.clearDisplay();
    // server_send();
    // eventVariables.sending_activity = false;
  }*/
  /*if (!digitalRead(b))
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
  }*/
  
}