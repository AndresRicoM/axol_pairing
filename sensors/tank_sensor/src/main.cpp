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

// CONSTANTS
#define DEV_I2C Wire
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 10       /* Time ESP32 will go to sleep (in seconds) */

// CONSTRUCTORS
void send_espnow();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
void check_data();
void check_pairing_connection();

////FLAGS FOR SETUP WITH HOMEHUB AND NETWORK////////
bool received_message = false;
bool data_sent = false;

/// PINS ////////////////////////////////////////////////
int sensorVoltage = 4;
int x_shut = 5;
int STU = 7;
bool setup_pressed = 0;

// Receiver address
uint8_t broadcastAddress[6] = {}; // MAC Address for receiving homehub.

//ESPnow Channel
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

// Components.
VL53L4CX sensor_vl53l4cx_sat(&DEV_I2C, x_shut);

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(460800);
  delay(100); //Comment for production. 

  pinMode(sensorVoltage, OUTPUT);
  digitalWrite(sensorVoltage, HIGH);
  pinMode(x_shut, OUTPUT);
  digitalWrite(x_shut, HIGH);
  pinMode(STU, INPUT_PULLUP);

  setup_pressed = digitalRead(STU);

  // Initialize I2C bus.
  DEV_I2C.setPins(0, 1);

  // Initialize I2C bus.
  if (DEV_I2C.begin()) {
    Serial.println("I2C bus started");
  } else {
    Serial.println("Failed to start I2C bus");
  }

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
  Serial.println("[setup] MAC Address for this device:");
  Serial.println(mac_add);
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  // get the status of Trasnmitted packet
  // Once ESPNow is successfully Init, we will register for Send CB to

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("[setup] Error init ESP-NOW");
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

  Serial.println("[setup] WiFi Info...");
  WiFi.printDiag(Serial); // Uncomment to verify channel change after

  Serial.println("Registering callbacks...");
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("///////////////////////");
  Serial.println("[setup] BROADCAST ADDRESS FROM SETUP...");
  printMacAddress(broadcastAddress);
  Serial.println("///////////////////////");
  
  Serial.println("[setup] Checking pairing connection...");
  check_pairing_connection();
  
  delay(100);
  send_espnow();

  delay(500);

  /////////////////Change value for higher or lower frequency of data collection. This is the time the ESP32 will sleep for.
  esp_sleep_enable_timer_wakeup(3600000000 * 12); // TIME_TO_SLEEP * uS_TO_S_FACTOR); //Twice per day. Value is in microseconds.

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

  Serial.println("Copying data to struct...");
  strcpy(myData.id, mac_add);
  myData.type = 2; // Id 2 = Tank Level sensor.

  Serial.println("[send_espnow] Data copied to struct:");
  Serial.print("[send_espnow] ID: ");
  Serial.println(myData.id);
  Serial.print("[send_espnow] Type: ");
  Serial.println(myData.type);
  Serial.print("[send_espnow] Height: ");
  Serial.println(myData.height);

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

void loop()
{
}