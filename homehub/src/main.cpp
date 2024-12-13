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
#include <EEPROM.h>
#include <Arduino.h>
#include <WiFiManager.h>
#include <strings_en.h>
#include <wm_consts_en.h>
#include <wm_strings_en.h>
#include <wm_strings_es.h>

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

#define EEPROM_SIZE 1
int touch_delay = 300;
/* FUNCTION HEADERS */
int get_buttons();
void get_time();
void get_complete_weather();
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
void server_send();
void get_system_stats();
void connect_send(String php_command);

/* HEADERS FOR 2.0v */
void bindServerCallback();
void handleSetupRoute();
void handleRegister();
void handleRegisterRequest();
bool establishWiFiConnection();
void printNetworkInfo();
void onDemandPortal();
void handleSensors();

typedef struct struct_message
{
  char id[50];
  int type = 0;
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
WiFiManager wm;

// Latitude & Longitude
double lat, lon;

// CAPTIVE-PORTAL FUNCTIONS
void bindServerCallback()
{
  wm.server->on("/", handleSetupRoute);
  wm.server->on("/register", handleRegister);
  wm.server->on("/api/register", handleRegisterRequest);
  wm.server->on("/sensor", handleSensors);
}

void handleRegisterRequest()
{
  // Variables to store data sent by the form
  String username = wm.server->arg("username");
  String homehubName = wm.server->arg("homehub_name");

  // Get the MAC address of the ESP32
  String macAddr = WiFi.macAddress();

  // Create JSON body for the POST request
  StaticJsonDocument<256> jsonDoc;

  jsonDoc["macAddr"] = macAddr;
  jsonDoc["latitude"] = lat;
  jsonDoc["longitude"] = lon;
  jsonDoc["username"] = username;
  jsonDoc["name"] = homehubName;

  // Serialize the JSON
  String jsonBody;
  serializeJson(jsonDoc, jsonBody);

  // Make the POST request
  HTTPClient http;
  http.begin("http://192.168.100.17:3000/homehub"); // API address
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(jsonBody);
  String response;

  // Process the response
  if (httpResponseCode > 0)
  {
    response = http.getString(); // Save the API response
    Serial.println("API response: " + response);
    if(httpResponseCode == 201){
      // Write to EEPROM if registration is successful
      EEPROM.begin(EEPROM_SIZE);  // Initialize EEPROM with the specified size
      EEPROM.write(0, 1);  // Write 1 at position 0 in EEPROM, indicating that the ESP32 is registered
      EEPROM.commit();  // Save changes to EEPROM
      Serial.println("Success. Homehub has been marked as registered in EEPROM.");
    }
  }
  else
  {
    response = "Request error: " + String(httpResponseCode) + "\n" +
               "Error details: " + http.errorToString(httpResponseCode);
    Serial.println(response);
  }

  http.end();

  // Respond to the web client with the API response or error message
  wm.server->send(200, "text/plain", response);
}


/*
 * Captive Portal routes for pages
 */
void handleSetupRoute()
{
  String page = HTTP_HEAD_START + String(HTTP_STYLE) + "</head>" + "<body>" + "<h1>Axol HomeHub Configuration</h1>" + "<form action='/wifi' method='get'><button type='submit'>Configure WiFi</button></form><br/>" + "<form action='/register' method='get'><button type='submit'>Register</button></form><br/>" + "<form action='/info' method='get'><button type='submit'>Info</button></form><br/>" + "<form action='/exit' method='get'><button type='submit'>Exit</button></form><br/>" + "<form action='/sensor' method='get'><button type='submit'>Registro Sensor</button></form>" + HTTP_END;
  wm.server->send(200, "text/html", page);
}

void handleRegister()
{
  JsonDocument location = retrieveLocation();
  lat = location["lat"];
  lon = location["lon"];

  String page = HTTP_HEAD_START + String(HTTP_STYLE) + "<style>"

                + "input{" + "   border: 1px #C1BDBD solid;" + "   line-height: 2em;" + "}" + ".textbox{" + "   display: flex;" + "   flex-direction: column;" + "   align-items: flex-start;" + "   gap: 0.5rem;" + "}" + "form{" + "   gap: 1.5rem;" + "}"

                + "</style>" + "</head>" + "<body>" + "<h1>Register HomeHub</h1>" + "<form action='/api/register' method='post'>"

                + "<div class='textbox'>" + "   <span>Username</span>" + "   <input type='text' name='username' />" + "</div>"

                + "<div class='textbox'>" + "   <span>HomeHub Name</span>" + "   <input type='text' name='homehub_name' />" + "</div>"

                + "<button type='submit'>Register</button>" + "</form>" + HTTP_END;

  /* To-do: write to EEPROM */

  wm.server->send(200, "text/html", page);

}

void handleSensors()
{
  //String page = "";

  switch (myData.type)
  {
  case 1:
  {
    String page = HTTP_HEAD_START + String(HTTP_STYLE) + "<style>"

                + "input{" + "   border: 1px #C1BDBD solid;" + "   line-height: 2em;" + "}" + ".textbox{" + "   display: flex;" + "   flex-direction: column;" + "   align-items: flex-start;" + "   gap: 0.5rem;" + "}" + "form{" + "   gap: 1.5rem;" + "}"

                + "</style>" + "</head>" + "<body>" + "<h1>Register HomeHub</h1>" + "<form action='/api/register' method='post'>"

                + "<div class='textbox'>" + "   <span>Tank Capacity</span>" + "   <input type='text' name='capacity' placeholder='0.00' />" + "</div>"

                + "<div class='textbox'>" + "   <span>Use</span>" + "   <input type='text' name='use' placeholder='kitchen, cleaning, bathroom...' />" + "</div>"

                + "<div class='textbox'>" + "   <span>Tank Area</span>" + "   <input type='text' name='area' placeholder='0.00' />" + "</div>"

                + "<div class='textbox'>" + "   <span>Tank Height</span>" + "   <input type='text' name='height' placeholder='0.00' />" + "</div>"

                + "<button type='submit'>Register</button>" + "</form><br>" 

                + "<form action='/exit' method='get'><button type='submit'>Exit</button></form>"
                
                + HTTP_END;

    wm.server->send(200, "text/html", page);
  }
    break;
  
  default:
    {
      String page = HTTP_HEAD_START + String(HTTP_STYLE) + "<h1>SENSOR NO ENCONTRADO</h1>" + HTTP_END;
      wm.server->send(200, "text/html", page);
    }
    break;
  }
  
    
  /* To-do: write to EEPROM */

}

void onDemandPortal()
{
  // Check if the ESP32 is already registered in EEPROM
  if (EEPROM.read(0) == 1) {
    Serial.println("ESP32 is already registered. Captive portal will not open.");
    return;  // If it's registered, don't open the captive portal
  }
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

// Weather/Location Server Variables
StaticJsonDocument<1024> doc;
float coord_lon = doc["coord"]["lon"];
float coord_lat = doc["coord"]["lat"];
JsonObject weather_0;
int weather_0_id = weather_0["id"];
const char *weather_0_main = weather_0["main"];
const char *weather_0_description = weather_0["description"];
const char *weather_0_icon = weather_0["icon"];
const char *base = doc["base"];
JsonObject main1;
float main_temp = main1["temp"];
float main_feels_like = main1["feels_like"];
float main_temp_min = main1["temp_min"];
float main_temp_max = main1["temp_max"];
int main_pressure = main1["pressure"];
int main_humidity = main1["humidity"];
int main_sea_level = main1["sea_level"];
int main_grnd_level = main1["grnd_level"];
int visibility = doc["visibility"];
JsonObject wind;
float wind_speed = wind["speed"];
int wind_deg = wind["deg"];
float wind_gust = wind["gust"];
int clouds_all = doc["clouds"]["all"];
long dt = doc["dt"];
JsonObject sys;
int sys_type = sys["type"];
long sys_id = sys["id"];
const char *sys_country = sys["country"];
long sys_sunrise = sys["sunrise"];
long sys_sunset = sys["sunset"];
int timezone = doc["timezone"];
long id = doc["id"];
const char *city_name = doc["name"];
int cod = doc["cod"];

// Control Variables
int bucket_count = 0;
int current_liters = 100;
bool received_message = false;

// ESP Now Communication Variables
// typedef struct struct_message
// {
//   char id[50];
//   int type;
//   float data1;
//   float data2;
//   float data3;
//   float data4;
//   float data6;
//   float data7;

//   char ssid[32];
//   char mac_addr[18];

// } struct_message;

// struct_message myData;

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

void get_complete_weather()
{ // gets weather and location information.

  const String endpoint = "https://api.openweathermap.org/data/2.5/weather?lat=" + String(lat, 7) + "&lon=" + String(lon, 7) + "&appid=";
  const String key = "api key";

  HTTPClient http;

  http.begin(endpoint + key); // construct the URL
  Serial.println(endpoint + key);
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

    coord_lon = doc["coord"]["lon"];
    coord_lat = doc["coord"]["lat"];

    weather_0 = doc["weather"][0];
    weather_0_id = weather_0["id"];
    weather_0_main = weather_0["main"];
    weather_0_description = weather_0["description"];
    weather_0_icon = weather_0["icon"];

    base = doc["base"];

    main1 = doc["main"];
    main_temp = main1["temp"];
    main_feels_like = main1["feels_like"];
    main_temp_min = main1["temp_min"];
    main_temp_max = main1["temp_max"];
    main_pressure = main1["pressure"];
    main_humidity = main1["humidity"];
    main_sea_level = main1["sea_level"];
    main_grnd_level = main1["grnd_level"];

    visibility = doc["visibility"];

    wind = doc["wind"];
    wind_speed = wind["speed"];
    wind_deg = wind["deg"];
    wind_gust = wind["gust"];

    clouds_all = doc["clouds"]["all"];

    dt = doc["dt"];

    sys = doc["sys"];
    sys_type = sys["type"];
    sys_id = sys["id"];
    sys_country = sys["country"];
    sys_sunrise = sys["sunrise"];
    sys_sunset = sys["sunset"];

    timezone = doc["timezone"];
    id = doc["id"]; // 6692163
    city_name = doc["name"];
    cod = doc["cod"];
  }

  else
  {
    Serial.println("Error fetching weather and location information. =( ");
  }

  http.end();
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{ // Fucntion is activated when ESP receives data on ESPNOW.
  // It copies the received message to memory and sets the received message variable to True to indicate that there is new data to be sent to the server.
  memcpy(&myData, incomingData, sizeof(myData));
  received_message = true;
  // Serial.println("SE RECIBIO UN DATO NUEVO DE ALGUN SENSOR");
  // Serial.println(myData.type);
}

void server_send()
{ // Sends data to php script on server.
  // The function checks to see if the command is for the regular homehub climate updates or if the command is comming from a known connected sensor.
  // Case Switch function creates different php command depending on the type of sensor that the data corresponds to.

  String command;

  if (sending_climate)
  {
    get_time();
    get_complete_weather();
    command = {"type=0&id=" + WiFi.macAddress() + "&temp=" + main_temp + "&min_temp=" + main_temp_min +
               "&max_temp=" + main_temp_max + "&weather_main=" + weather_0_main + "&weather_description=" +
               weather_0_description + "&pressure=" + main_pressure + "&humidity=" + main_humidity +
               "&wind_speed=" + wind_speed + "&wind_direction=" + wind_deg + "&datetime=" + formattedDate};
    connect_send(command);
    sent_time = millis();
    sending_climate = false;
  }

  if (sending_activity)
  {

    get_time();
    command = "type=5&id=" + WiFi.macAddress() + "&activity=" + activity + "&datetime=" + formattedDate;
    Serial.println(command);
    connect_send(command);
  }
  else
  {

    switch (myData.type)
    {

    case 1:
    { // Bucket Sensor
      get_time();
      String send_id = myData.id;
      command = "type=1&id=" + send_id + "&datetime=" + formattedDate;
      Serial.println(command);
      connect_send(command);
    }
    break;

    case 2: // Tank Sensor
    {
      get_time();
      String send_id = myData.id;
      command = "type=2&id=" + send_id + "&water_distance=" + myData.data1 + "&datetime=" + formattedDate;
      Serial.println(command);
      connect_send(command);
    }
    break;

    case 3: // Temp Humidity Sensor
    {
      get_time();
      String send_id = myData.id;
      command = "type=3&id=" + send_id + "&temp=" + myData.data1 + "&humidity=" + myData.data2 + "&datetime=" + formattedDate;
      Serial.println(command);
      connect_send(command);
    }

    case 4: // Water Quality Sensor
    {
      get_time();
      String send_id = myData.id;
      command = "type=4&id=" + send_id + "&tds=" + myData.data1 + "&water_temp=" + myData.data2 + "&datetime=" + formattedDate;
      Serial.println(command);
      connect_send(command);
    }
    break;
    }
  }
}

void connect_send(String php_command)
{ // Send Data to PHP server
  // Function takes command as argument and sends a POST request to server.

  HTTPClient http;
  WiFiClient client;

  String server_main = "http://blindspot.media.mit.edu/homehub.php";

  http.begin(client, server_main);

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String httpRequestData = php_command;

  int httpResponseCode = http.POST(httpRequestData);

  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
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
  EEPROM.begin(EEPROM_SIZE);  // Initialize EEPROM with the specified size

  pinMode(up, INPUT_PULLUP);
  pinMode(down, INPUT_PULLUP);
  pinMode(right, INPUT_PULLUP);
  pinMode(left, INPUT_PULLUP);
  pinMode(a, INPUT_PULLUP);
  pinMode(b, INPUT_PULLUP);
  // Function commented so I can try brodcast with button: 
  // char saved_ssid[32];

  /* SETTING UP SENSOR PAIRING  */

  // Connecting to saved wifi network to get ssid
  // connect_to_saved_wifi_network();
  // strcpy(saved_ssid, WiFi.SSID().c_str());

  // Serial.print("Saved SSID: ");
  // Serial.println(saved_ssid);

  // Disconnecting in order to establish communication between sensors without router intervention
  // WiFi.disconnect();

  // WiFi.mode(WIFI_STA);
  // // Setting wifi channel
  // const int wifi_channel = 13;
  // esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);

  // WiFi.printDiag(Serial);

  // Serial.println("Starting ESP NOW Communication");
  // if (esp_now_init() != ESP_OK)
  // {
  //   Serial.println("Error initializing ESP-NOW");
  //   return;
  // }

  // // ESP-NOW Broadcast MAC Address
  // uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  // // Sending pairing data struct
  // esp_now_peer_info_t peerInfo = {};
  // memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  // peerInfo.encrypt = false;

  // if (!esp_now_is_peer_exist(broadcastAddress))
  // {
  //   esp_now_add_peer(&peerInfo);
  // }

  // Serial.println("Canal wifi: ");
  // Serial.println(WiFi.channel());
  // Serial.println("------------------------------------------------");
  // Serial.println("------------------------------------------------");
  // Serial.println("------------------------------------------------");

  // // Formatting MAC Address to XX:XX:XX:XX:XX:XX
  // strcpy(pairingData.ssid, saved_ssid);
  // strcpy(pairingData.mac_addr, WiFi.macAddress().c_str());

  // esp_err_t result = esp_now_send(broadcastAddress, (const uint8_t *)&pairingData, sizeof(pairingData));
  // Serial.println(result == ESP_OK ? "Datos enviados por broadcast" : "Error al enviar datos");

  // Serial.println("------------------------------------------------");
  // Serial.println("------------------------------------------------");
  // Serial.println("------------------------------------------------");

  // //esp_now_register_recv_cb(OnDataRecv);
  // Serial.println("Debug: fin");

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
  delay(3000);
  display.invertDisplay(false);
  delay(3000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);

  // Display Welcome Text
  display.println("Inicializando HomeHub"); //"Welcome to Home  Hub"
  display.display();
  delay(2000);

  WiFi.mode(WIFI_AP_STA); // Optional
  // WiFi.mode(WIFI_STA);
  display.clearDisplay();
  Serial.print("Connecting to WiFi");
  display.display();
  delay(2000);

  WiFi.begin(ssid, password);

  connect_to_saved_wifi_network();

  // while (WiFi.status() != WL_CONNECTED)
  // { // Check wi-fi is connected to wi-fi network
  //   delay(1000);
  //   Serial.print(".");
  //   display.print(".");
  //   display.display();
  // }

  display.clearDisplay();
  display.print("Conectado a: "); //"Connected to: "
  display.println(WiFi.SSID());
  display.print("Mi IP:"); //"My IP Address is "
  display.println(WiFi.localIP());
  display.print("Mi MAC:"); //"My IP Address is "
  display.println(WiFi.macAddress());
  display.display();
  Serial.println("");
  Serial.println("WiFi connected successfully");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP()); // Show ESP32 IP on serial
  Serial.print("Mi MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

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
  get_complete_weather();

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

  delay(3000);
  draw.draw_maindash();

  display.print("Hello, I'm the Pairinng Home Hub 2.0!");
  Serial.println("Setup is complete!");
}
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
    /*
     1 - Up       // GPIO27 -> Up
     2 - Down     // GPIO15 -> Down
     3 - Right    // GPIO13 -> Right
     4 - Left     // GPIO14 -> Left
     5 - A        // GPIO4 -> B
     6 - B        // GPIO2 -> A
  */
  if (!digitalRead(up))
  { // Shows Clock Screen When Up Arrow is Pressed
    Serial.print("Presionaste: ");
    Serial.println(1);
    char saved_ssid[32];

    connect_to_saved_wifi_network();
    strcpy(saved_ssid, WiFi.SSID().c_str());

  Serial.print("Saved SSID: ");
  Serial.println(saved_ssid);

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

  Serial.println("Hiciste paired con: ");
  Serial.println(myData.type);

  //onDemandPortal();

    // display.clearDisplay();
    // display.print("Abriendo portal captivo...");
    // display.display();
    // delay(5000);
    // Serial.println("Borrando credenciales de Wi-Fi...");
    // wm.resetSettings(); // Borra las credenciales de Wi-Fi
    // ESP.restart();      // Reinicia el ESP32
    // sending_activity = true;
    // activity = 1;

    // // Update weather and then draw the information
    // get_time();
    // get_complete_weather();
    // draw.draw_clockdash(timeStamp, dayStamp, city_name, main_temp, main_temp_max, main_temp_min, weather_0_icon);

    // server_send();
    // sending_activity = false;
    delay(touch_delay);
  }
  if (!digitalRead(down))
  { // Shows Water Dashboard
    Serial.print("Presionaste: ");
    Serial.println(2);
    sending_activity = true;
    activity = 2;

    // Update system stats and then draw the information
    get_system_stats();
    draw.draw_waterdash(fill_percentage, avail_liters, avail_storage);

    server_send();
    sending_activity = false;
    delay(touch_delay);
  }
  if (!digitalRead(right))
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
    delay(touch_delay);
  }
  if (!digitalRead(left))
  { // Clear Display
    Serial.print("Presionaste: ");
    Serial.println(get_buttons());
    sending_activity = true;
    activity = 4;

    // display.clearDisplay();
    // display.print("Abriendo portal captivo...");
    // display.display();
    // delay(5000);

    // draw.draw_system(buckets, tanks, quality, envs);
    // server_send();
    // sending_activity = false;


    delay(touch_delay);
  }
  if (!digitalRead(a))
  { // Clear Display

    Serial.print("Presionaste: ");
    Serial.println(get_buttons());
    Serial.println("Abriendo portal en demanda");
    display.println("Abriendo portal en demanda");


    onDemandPortal();
    
    // sending_activity = true;
    // activity = 5;
    // display.clearDisplay();
    // server_send();
    // sending_activity = false;
    delay(touch_delay);
  }
  if(!digitalRead(b))
  {
    Serial.print("Presionaste: ");
    Serial.println(digitalRead(b));
    EEPROM.begin(EEPROM_SIZE);  // Initialize EEPROM with the specified size
    EEPROM.write(0, 0);  // Write 0 at position 0 in EEPROM, indicating that the ESP32 is NOT registered
    EEPROM.commit();  // Save changes to EEPROM

    // Serial.println("Borrando credenciales de Wi-Fi...");
    // wm.resetSettings(); // Borra las credenciales de Wi-Fi
    // ESP.restart();      // Reinicia el ESP32
    delay(touch_delay);
  }
}
