/*

   █████╗ ██╗  ██╗ ██████╗ ██╗
  ██╔══██╗╚██╗██╔╝██╔═══██╗██║
  ███████║ ╚███╔╝ ██║   ██║██║
  ██╔══██║ ██╔██╗ ██║   ██║██║
  ██║  ██║██╔╝ ██╗╚██████╔╝███████╗
  ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝

  ᓬ(• - •)ᕒ

  Axol sensing system.

   Code for tank quantity sensor. The seansor uses a vl53l4 optical sensor to detemine its distance from the water's surface.
   The data is then used to calculate the quanity and fill percentage in the database. Volume is calaculated in the database backend.
   You need to measure containers dimensions and capacity to register this device for the backend to calculate volume correctly.

   Andres Rico - aricom@mit.edu

 */

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <vl53l4cx_class.h>
#include <Preferences.h> // Include the Preferences library for EEPROM-like functionality
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#define DEV_I2C Wire
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 10       /* Time ESP32 will go to sleep (in seconds) */

// CONSTRUCTORS
void send_espnow();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
void check_data();
void check_pairing_connection();

bool received_message = false;

// Receiver address
uint8_t broadcastAddress[] = {255, 255, 255, 255, 255, 255}; // MAC Address for receiving homehub.

int32_t wifi_channel = 13;

/////////////////////////////////////////////////////////////////////

typedef struct struct_message
{
  char id[50];
  int type;
  float height;
} struct_message;

struct_message myData;

typedef struct pairing_data
{
  char mac_addr[18];
} pairing_data;

struct pairing_data pairingData = {};

String address = WiFi.macAddress();
char mac_add[50];

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
  String savedMAC = preferences.getString("mac", "");
  preferences.end();

  if (savedMAC.length() > 0)
  {
    stringToMacAddress(savedMAC, broadcastAddress);

    Serial.println("Data loaded from EEPROM:");
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

  if (strlen(pairingData.mac_addr) > 0)
  {
    Serial.print("Homehub MAC Address: ");
    Serial.println(pairingData.mac_addr);

    preferences.begin("sensor-data", false);
    preferences.putString("mac", String(pairingData.mac_addr));
    preferences.end();

    stringToMacAddress(pairingData.mac_addr, broadcastAddress);
  }
  else
  {
    Serial.println("Invalid MAC Address. Check Homehub connection");
    return;
  }

  Serial.println("MAC Address assigned!");
}

// Components.
VL53L4CX sensor_vl53l4cx_sat(&DEV_I2C, 16);

void setup()
{
  // put your setup code here, to run once:
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);

  Serial.begin(115200);

  // Initialize I2C bus.
  DEV_I2C.begin();

  // Configure VL53L4CX satellite component.
  sensor_vl53l4cx_sat.begin();

  // Switch off VL53L4CX satellite component.
  sensor_vl53l4cx_sat.VL53L4CX_Off();

  // Initialize VL53L4CX satellite component.
  sensor_vl53l4cx_sat.InitSensor(0x12);

  // Start Measurements
  sensor_vl53l4cx_sat.VL53L4CX_StartMeasurement();

  int i = 0;
  int final_reading;
  float sum = 0;

  while (i < 50)
  {

    VL53L4CX_MultiRangingData_t MultiRangingData;
    VL53L4CX_MultiRangingData_t *pMultiRangingData = &MultiRangingData;
    uint8_t NewDataReady = 0;
    int no_of_object_found = 0, j;
    char report[64];
    int status;

    do
    { // Loop until we get new data.
      status = sensor_vl53l4cx_sat.VL53L4CX_GetMeasurementDataReady(&NewDataReady);
    } while (!NewDataReady);

    if ((!status) && (NewDataReady != 0))
    {
      status = sensor_vl53l4cx_sat.VL53L4CX_GetMultiRangingData(pMultiRangingData);
      no_of_object_found = pMultiRangingData->NumberOfObjectsFound;
      if (no_of_object_found == 1)
      {
        i = i + 1;
        sum = sum + pMultiRangingData->RangeData[0].RangeMilliMeter;
        // Serial.println(pMultiRangingData->RangeData[0].RangeMilliMeter);
      }

      if (status == 0)
      {
        status = sensor_vl53l4cx_sat.VL53L4CX_ClearInterruptAndStartMeasurement();
      }
    }
  }

  // digitalWrite(18, LOW);
  myData.height = sum / 50; // Average 50 from 50 readings.
  Serial.println("Altura calculada:");
  Serial.println(myData.height);

  address.toCharArray(mac_add, 50);
  Serial.println(mac_add);
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  esp_now_init();

  // WiFi.printDiag(Serial); // Uncomment to verify channel number before
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  // WiFi.printDiag(Serial); // Uncomment to verify channel change after

  Serial.println("Registering callbacks...");
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Checking pairing connection...");
  check_pairing_connection();
  send_espnow();

  /////////////////Change value for higher or lower frequency of data collection. This is the time the ESP32 will sleep for.
  esp_sleep_enable_timer_wakeup(3600000000 * 12); // TIME_TO_SLEEP * uS_TO_S_FACTOR); //Twice per day. Value is in microseconds.

  esp_wifi_stop();

  esp_deep_sleep_start();
}

void send_espnow()
{
  address.toCharArray(mac_add, 50);
  Serial.println(mac_add);

  Serial.println("Wifi channel is:");
  Serial.println(wifi_channel);

  esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);

  Serial.println("Copying data to struct...");
  strcpy(myData.id, mac_add);
  myData.type = 2; // Id 2 = Tank Level sensor.

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

void loop()
{
}