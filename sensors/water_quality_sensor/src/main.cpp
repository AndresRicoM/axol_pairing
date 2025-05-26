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
#include "esp_adc_cal.h"

#define DEV_I2C Wire
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1        /* Time ESP32 will go to sleep (in seconds) */

//#define TdsSensorPin 4 //Output pin for giving power to the sensor. 
#define VREF 3      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point

#define ADC_PIN 4  // GPIO4
#define DEFAULT_VREF 1100  // Default VREF in mV for calibration fallback
#define SAMPLES 50         // Optional: number of samples to average

esp_adc_cal_characteristics_t adc_chars;

// CONSTRUCTORS
void send_espnow();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
void check_data();
void check_pairing_connection();

////FLAGS FOR SETUP WITH HOMEHUB AND NETWORK////////
bool received_message = false;
bool data_sent = false;


int sensorVoltage = 5; 
int sensorVoltage2 = 6;
int STU = 7;
int tdsSignal = 4;
bool setup_pressed = 0;


SensirionI2cSht4x sht4x;

int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0;

float temperature;
float humidity;

// Receiver address
uint8_t broadcastAddress[6] = {}; // MAC Address for receiving homehub.

//ESPnow Channel
int32_t wifi_channel = 13;

typedef struct struct_message {
  char id[50];
  int type;
  float tds;
  float temp;
  float hum;
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

void printMacAddress(const uint8_t *mac)
{
  Serial.print("[printMacAddress] MAC Address: ");
  for (int i = 0; i < 6; i++)
  {
    if (i > 0)
      Serial.print(":");
    if (mac[i] < 0x10) // Add leading zero for single-digit hex values
      Serial.print("0");
    Serial.print(mac[i], HEX);
  }
  Serial.println();
}

void handshake()
{
  esp_err_t result = esp_now_send((const uint8_t *)broadcastAddress, (uint8_t *)&pairingData, sizeof(pairingData));
  if (result != ESP_OK)
  {
    Serial.print("[handshake] Error sending handshake: ");
    Serial.println(result);
    return;
  }
  Serial.println("[handshake] Handshake sent!");
}

void check_data()
{
  Serial.println("[check_data] Pairing Data!");
  Serial.print("[check_data] Homehub Data received:");
  Serial.println(pairingData.mac_addr);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\n[OnDataSent] Last Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  data_sent = true;
}

String macToString(const uint8_t *mac)
{
  String macStr = "";
  for (int i = 0; i < 6; i++)
  {
    if (i > 0)
      macStr += ":";
    if (mac[i] < 0x10) // Add leading zero for single-digit hex values
      macStr += "0";
    macStr += String(mac[i], HEX);
  }
  macStr.toUpperCase(); // Ensure the MAC address is in uppercase

  return macStr;
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&pairingData, incomingData, sizeof(pairingData));
  memcpy(broadcastAddress, mac, sizeof(broadcastAddress));

  check_data();

  Serial.println("[OnDataRecv] THIS IS THE SENDER MAC ADDRESS!");
  printMacAddress(mac);
  printMacAddress(broadcastAddress);
  Serial.println(sizeof(broadcastAddress));

  delay(100);
  handshake();
  received_message = true;
}

void stringToMacAddress(const String &macStr, uint8_t *macAddr)
{
  Serial.println("[stringToMacAddress] Converting String to MAC Address array for broadcast...");
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
  Serial.println("[check_pairing_connection] Checking stored data in EEPROM...");

  preferences.begin("sensor-data", false);
  String savedMAC = preferences.getString("mac", "");
  preferences.end();

  if (savedMAC.length() > 0 && savedMAC != "00:00:00:00:00:00" && setup_pressed)
  {
    stringToMacAddress(savedMAC, broadcastAddress);

    Serial.println("[check_pairing_connection] Data loaded from EEPROM:");
    Serial.print("[check_pairing_connection] BROADCAST MAC Address: ");
    printMacAddress(broadcastAddress);
    Serial.print("[check_pairing_connection] SavedMAC string: ");
    Serial.println(savedMAC);
    return;
  } else if (!setup_pressed)
  {
    Serial.println("[check_pairing_connection] No saved data found. Waiting for Homehub MAC Address ...");

  while (!received_message)
  {
    delay(300);
  }

  delay(200);

  // Convert broadcastAddress to String
  String macStr = macToString(broadcastAddress);

  if (macStr.length() > 0)
  {
    Serial.println("[check_pairing_connection] Homehub MAC Address... ");
    printMacAddress(broadcastAddress);

    preferences.begin("sensor-data", false);
    preferences.putString("mac", macStr);
    preferences.end();
  }
  else
  {
    Serial.println("[check_pairing_connection] Invalid MAC Address. Check Homehub connection");
    return;
  }

  Serial.println("[check_pairing_connection] MAC Address assigned!");

  }
  
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
     Serial.print("[getHumTemp] Temperature:");
     Serial.print(temperature);
     Serial.println("[getHumTemp] Humidity:");
     Serial.print(humidity);
   }

 }

 void setup() {
   // put your setup code here, to run once:
  Serial.begin(460800);
  delay(100);

  analogReadResolution(12); // 12-bit resolution
  esp_adc_cal_characterize(
    ADC_UNIT_1,
    ADC_ATTEN_DB_11,   // Best for 0–3.3V range
    ADC_WIDTH_BIT_12,
    DEFAULT_VREF,
    &adc_chars
  );

  pinMode(sensorVoltage, OUTPUT);
  pinMode(sensorVoltage2, OUTPUT);
  pinMode(STU, INPUT_PULLUP);
  //pinMode(tdsSignal, INPUT);

  digitalWrite(sensorVoltage, HIGH);
  digitalWrite(sensorVoltage2, HIGH);   

  setup_pressed = digitalRead(STU);

  DEV_I2C.setPins(0, 1);
  Wire.begin();
  sht4x.begin(Wire,0x44);

  getHumTemp(); //Get temperature and humidity for temperature compensation.

  uint32_t adc_reading = 0;

   // Take multiple samples for stability
  for (int i = 0; i < SAMPLES; i++) {
    adc_reading += analogRead(ADC_PIN);
  }
  adc_reading /= SAMPLES;

  // Convert raw reading to voltage in mV using calibrated VREF
  uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars);

  //averageVoltage = analogVal * (float)VREF / 4096.0;                                                                                                                               // read the analog value more stable by the median filtering algorithm, and convert to voltage value
  float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);                                                                                                               // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  float compensationVolatge = voltage / compensationCoefficient;                                                                                                            // temperature compensation
  float tdsValue = (
    1.089e-6 * compensationVolatge * compensationVolatge * compensationVolatge
    - 0.001216 * compensationVolatge * compensationVolatge
    + 1.021 * compensationVolatge
    - 13.04
  ) *  .5;
  
  // Serial.println(read_efuse_vref(void));

  address.toCharArray(mac_add, 50);
  Serial.println("[setup] MAC Address for this device:");
  Serial.println(mac_add);
  
  WiFi.mode(WIFI_STA);
  
  
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("{setup] Error init ESP-NOW");
    return;
  }
  Serial.print("[setup] Wifi channel is:");
  Serial.println(wifi_channel);

  // WiFi.printDiag(Serial); // Uncomment to verify channel number before
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  // Forcing channel synchronization
  delay(100);

  Serial.println("[setup] Checking pairing connection...");
  delay(100);
  Serial.println("[setup] WiFi Info...");
  WiFi.printDiag(Serial); // Uncomment to verify channel change after
  
  // Init ESP-NOW
  // get the status of Trasnmitted packet
  // Once ESPNow is successfully Init, we will register for Send CB to
  
  Serial.println("[setup] Registering callbacks...");
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("///////////////////////");
  Serial.println("[setup] BROADCAST ADDRESS FROM SETUP...");
  printMacAddress(broadcastAddress);
  Serial.println("///////////////////////");
  
  //check_pairing_connection();
  send_espnow();

  delay(500);

  /////////////////Change value for higher or lower frequency of data collection. This is the time the ESP32 will sleep for.
  esp_sleep_enable_timer_wakeup(43200000000); // TIME_TO_SLEEP * uS_TO_S_FACTOR); //Twice per day. Value is in microseconds.
  // esp_sleep_enable_timer_wakeup(30000000) ; // 30 seconds for demo.

  // Cleaning before going to sleep
  esp_wifi_stop();

  esp_deep_sleep_start();

 }

 void send_espnow()
 {
   Serial.println("***************");
   Serial.println("[send_espnow] Broadcast address before peer...");
   printMacAddress(broadcastAddress);
   Serial.println("***************");
 
   Serial.println("[send_espnow] Copying data to struct...");
   strcpy(myData.id, mac_add);
   myData.type = 4; // Id 4 = Water Quality.
   myData.temp = temperature;
   myData.tds = tdsValue;
   myData.hum = humidity;

 
   Serial.println("[send_espnow] Data copied to struct:");
   Serial.print("[send_espnow] ID: ");
   Serial.println(myData.id);
   Serial.print("[send_espnow] Type: ");
   Serial.println(myData.type);
   Serial.print("[send_espnow] Temperature: ");
   Serial.println(myData.temp);
   Serial.print("[send_espnow] TDS: ");
   Serial.println(myData.tds);
   Serial.print("[send_espnow] Humidity: ");
   Serial.println(myData.hum);
 
   // Serial.println("[send_espnow] Deleting previous peer...");
   // esp_now_del_peer(broadcastAddress);
 
   Serial.println("[send_espnow] Registering peer...");
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
     Serial.print("[send_espnow] Failed to add peer, error code: ");
     Serial.println(addPeerResult);
   }
 
   // Send message via ESP-NOW
   Serial.println("[send_espnow] Sending data via ESP-NOW...");
   Serial.println("----------");
   Serial.println("[send_espnow] Sending to... ");
   printMacAddress(broadcastAddress);
   Serial.println("----------");

   Serial.println("[send_espnow] Data copied to struct:");
   Serial.print("[send_espnow] ID: ");
   Serial.println(myData.id);
   Serial.print("[send_espnow] Type: ");
   Serial.println(myData.type);
   Serial.print("[send_espnow] Temperature: ");
   Serial.println(myData.temp);
   Serial.print("[send_espnow] TDS: ");
   Serial.println(myData.tds);
   Serial.print("[send_espnow] Humidity: ");
   Serial.println(myData.hum);
 
   esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
 
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
 }

 void loop() {
   // put your main code here, to run repeatedly:
 }
