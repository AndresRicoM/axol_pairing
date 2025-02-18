/*

   █████╗ ██╗  ██╗ ██████╗ ██╗
  ██╔══██╗╚██╗██╔╝██╔═══██╗██║
  ███████║ ╚███╔╝ ██║   ██║██║
  ██╔══██║ ██╔██╗ ██║   ██║██║
  ██║  ██║██╔╝ ██╗╚██████╔╝███████╗
  ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝

  ᓬ(• - •)ᕒ 

  Axol sensing system.

   Code for water quality sensor. The sensor uses a TDS meter to measure water conductivity.
   The code is based on the example code provided by DFrobot for the TDS meter.

   Andres Rico - aricom@mit.edu

 */


#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <SensirionI2CSht4x.h>
#include <Preferences.h>
#include "driver/adc.h"
#include "esp_system.h"
#include <Wire.h>

#define DEV_I2C Wire
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1        /* Time ESP32 will go to sleep (in seconds) */

//#define TdsSensorPin 4 //Output pin for giving power to the sensor. 
#define VREF 3.3      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point


int sensorVoltage = 4;
int sensorVoltage2 = 6;
int STU = 7;
int tdsSignal = 5;

// CONSTRUCTORS
void send_espnow();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
int32_t getWiFiChannel(const char *ssid);
void check_data();
void check_pairing_connection();

SensirionI2cSht4x sht4x;

int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0;
//int sensPower = 17;
float temperature;
float humidity;

////CHANGE THESE VARIABLES FOR SETUP WITH HOMEHUB AND NETWORK////////

bool received_message = false; //change

//Receiver address
uint8_t broadcastAddress[] = {255, 255, 255, 255, 255, 255}; //MAC Address for receiving homehub.  

char WIFI_SSID[32] = ""; //Network name, no password required.

int32_t wifi_channel = 13;

 /////////////////////////////////////////////////////////////////////


int32_t getWiFiChannel(const char *ssid) {

  Serial.println("Scanning Networks...");
  Serial.println(ssid);

  // Ensure WiFi is in STA mode
  WiFi.mode(WIFI_STA);

  int32_t n = WiFi.scanNetworks();
  Serial.println("Number of Networks found:");
  Serial.println(n);

  if (n > 0)
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

typedef struct struct_message {
  char id[50];
  int type;
  float tds;
  float temp;
} struct_message;

struct_message myData;

typedef struct pairing_data
{
  char ssid[32];
  char mac_addr[18];
} pairing_data;

struct pairing_data pairingData = {};

String address = WiFi.macAddress();
char mac_add[50];

int attempts = 0; //change

void handshake()
{
  esp_err_t result = esp_now_send((const uint8_t *)broadcastAddress, (uint8_t *)&pairingData, sizeof(pairingData));
  if (result != ESP_OK)
  {
    Serial.print("Error sending handshake: ");
    Serial.println(result);
  }
}

void check_data()
{
  Serial.println("Pairing Data!");
  Serial.print("Homehub MAC Address:");
  Serial.println(pairingData.mac_addr);
  Serial.print("Router SSID:");
  Serial.println(pairingData.ssid);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&pairingData, incomingData, sizeof(pairingData));
  check_data();

  memcpy(broadcastAddress, mac, 6);

  Serial.println("THIS IS THE SENDER MAC ADDRESS!");
  Serial.println(*mac);

  handshake();
  received_message = true;
}

void stringToMacAddress(const String &macStr, uint8_t *macAddr)
{
  int byteIndex = 0;
  for (int i = 0; i < macStr.length(); i += 3)
  {
    String byteStr = macStr.substring(i, i + 2);
    macAddr[byteIndex++] = (uint8_t)strtol(byteStr.c_str(), NULL, 16);
  }
}

void check_pairing_connection()
{
  Preferences preferences;
  Serial.println("Checking stored data in EEPROM...");

  preferences.begin("sensor-data", false);
  String savedSSID = preferences.getString("ssid", "");
  String savedMAC = preferences.getString("mac", "");
  preferences.end();

  if (savedSSID.length() > 0 && savedMAC.length() > 0)
  {
    strcpy(WIFI_SSID, savedSSID.c_str());
    stringToMacAddress(savedMAC, broadcastAddress);

    Serial.println("Data loaded from EEPROM:");
    Serial.print("SSID: ");
    Serial.println(WIFI_SSID);
    Serial.print("BROADCAST MAC Address: ");
    for (int i = 0; i < 6; i++)
    {
      if (i > 0)
        Serial.print(":");
      Serial.print(broadcastAddress[i], HEX);
    }
    Serial.println();
    return;
  }

  Serial.println("No saved data found. Waiting for SSID ...");

  while (!received_message)
  {
    delay(300);
  }

  if (strlen(pairingData.ssid) > 0)
  {
    Serial.print("SSID Received: ");
    Serial.println(pairingData.ssid);
    Serial.print("Homehub MAC Address: ");
    Serial.println(pairingData.mac_addr);

    preferences.begin("sensor-data", false);
    preferences.putString("ssid", pairingData.ssid);
    preferences.putString("mac", String(pairingData.mac_addr));
    preferences.end();

    stringToMacAddress(pairingData.mac_addr, broadcastAddress);
    strcpy(WIFI_SSID, pairingData.ssid);
  }
  else
  {
    Serial.println("Invalid SSID. Check Homehub connection");
    return;
  }

  Serial.println("SSID assigned!");
}

 void getHumTemp() {
   uint16_t error;
   char errorMessage[256];

   delay(20);

   error = sht4x.measureHighPrecision(temperature, humidity);
   if (error) {
     Serial.print("Error trying to execute measureHighPrecision(): ");
     errorToString(error, errorMessage, 256);
     Serial.println(errorMessage);
   } else {
     Serial.print("Temperature:");
     Serial.print(temperature);
     Serial.print("\t");
   }

 }

 void setup() {
   // put your setup code here, to run once:
   Serial.begin(460800);

   pinMode(sensorVoltage, OUTPUT);
   pinMode(sensorVoltage2, OUTPUT);
   pinMode(STU, INPUT_PULLUP);
   pinMode(tdsSignal, INPUT);

   digitalWrite(sensorVoltage, HIGH);
   digitalWrite(sensorVoltage2, HIGH);

  //CONFIGURE ADC
   // Initialize I2C bus. Temp Hum sensor
   /*

   DEV_I2C.setPins(0, 1);
   Wire.begin();
   sht4x.begin(Wire,0x44);
   getHumTemp(); //Get temperature and humidity for temperature compensation.
   
   */

  temperature = 25.0; //Default temperature value for testing.
   
  float analogSum = 0;
    for (int i = 0 ; i < 50 ; i++) {
    int rawValue = 0;
      analogSum = analogSum + adc2_get_raw(ADC2_CHANNEL_0, ADC_WIDTH_BIT_12, &rawValue);

    }

  float analogVal = analogSum / 50;

  Serial.print("Analog Value:");
  Serial.println(analogVal);
  delay(1000);
  
  averageVoltage = analogVal * (float)VREF / 4096.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
  float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
  tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value

  Serial.print("TDS Value:");
  Serial.print(tdsValue,0);
  Serial.println("ppm");
  Serial.print("Temperature:");
  Serial.print(temperature);
  Serial.println("C");
   


   //Serial.println(read_efuse_vref(void));

   address.toCharArray(mac_add, 50);
   Serial.println(mac_add);
   WiFi.mode(WIFI_STA);
   esp_now_init();

  //  int32_t wifi_channel = getWiFiChannel(WIFI_SSID);
  //  strcpy(myData.id, mac_add);
  //  myData.type = 4; //Id 4 = Water Quality.
  //  myData.temp = temperature;
  //  myData.tds = tdsValue;

  //  Serial.println(myData.id);

   //WiFi.printDiag(Serial); // Uncomment to verify channel number before
   esp_wifi_set_promiscuous(true);
   esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
   esp_wifi_set_promiscuous(false);
   //WiFi.printDiag(Serial); // Uncomment to verify channel change after

   // Init ESP-NOW
  //  esp_now_init();

   // Once ESPNow is successfully Init, we will register for Send CB to
   // get the status of Trasnmitted packet
   Serial.println("Registering callbacks...");
   esp_now_register_send_cb(OnDataSent);
   esp_now_register_recv_cb(OnDataRecv);

   Serial.println("Checking pairing connection...");
   check_pairing_connection();
   send_espnow();

   // Register peer
  //  esp_now_peer_info_t peerInfo = {};
  //  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  //  peerInfo.encrypt = false;

  //  // Add peer
  //  esp_now_add_peer(&peerInfo);

  //  // Send message via ESP-NOW
  //  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

   /////////////////Change value for higher or lower frequency of data collection. This is the time the ESP32 will sleep for.
   esp_sleep_enable_timer_wakeup(43200000000); //TIME_TO_SLEEP * uS_TO_S_FACTOR); //Twice per day. Value is in microseconds.
   //esp_sleep_enable_timer_wakeup(30000000) ; // 30 seconds for demo.


   esp_wifi_stop();

   esp_deep_sleep_start();

 }

 void send_espnow()
{
  address.toCharArray(mac_add, 50);
  Serial.println(mac_add);

  Serial.println("Changing WiFi Channel...");
  Serial.println("Current wifi ssid: ");
  Serial.println(WIFI_SSID);

  wifi_channel = getWiFiChannel(WIFI_SSID);

  Serial.println("Wifi channel is:");
  Serial.println(wifi_channel);

  esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);

  Serial.println("Copying data to struct...");
  strcpy(myData.id, mac_add);
  myData.type = 4; // Id 4 = Water Quality.
  myData.temp = temperature;
  myData.tds = tdsValue;
  Serial.println("Registering peer...");
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.encrypt = false;

  esp_err_t addPeerResult = esp_now_add_peer(&peerInfo);
  if (addPeerResult != ESP_OK)
  {
    Serial.print("Failed to add peer, error code: ");
    Serial.println(addPeerResult);
  }

  Serial.println("Sending data via ESP-NOW...");
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  if (result != ESP_OK)
  {
    Serial.print("Failed to send data, error code: ");
    Serial.println(result);
  }
}

 void loop() {
   // put your main code here, to run repeatedly:
 }
