/*

   █████╗ ██╗  ██╗ ██████╗ ██╗
  ██╔══██╗╚██╗██╔╝██╔═══██╗██║
  ███████║ ╚███╔╝ ██║   ██║██║
  ██╔══██║ ██╔██╗ ██║   ██║██║
  ██║  ██║██╔╝ ██╗╚██████╔╝███████╗
  ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝

  ᓬ(• - •)ᕒ

  Axol sensing system.

   Code for bucket sensor. The sensor uses a tilt sensor to detect when a bucket is used.

   Andres Rico - aricom@mit.edu

 */
#include <Preferences.h>  // Include the Preferences library for EEPROM-like functionality
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Arduino.h>

// CONSTRUCTORS
void send_espnow();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
bool received_message = false;

// int32_t getWiFiChannel(const char *ssid);

////CHANGE THESE VARIABLES FOR SETUP WITH HOMEHUB AND NETWORK////////

// Receiver address
uint8_t broadcastAddress[] = {255, 255, 255, 255, 255, 255}; // MAC Address for receiving homehub.

char WIFI_SSID[] = ""; // Network name, no password required.

int32_t wifi_channel = 13;

/////////////////////////////////////////////////////////////////////

int32_t getWiFiChannel(const char *ssid) {

    if (int32_t n = WiFi.scanNetworks()) {
        for (uint8_t i=0; i<n; i++) {
            if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
                return WiFi.channel(i);
            }
        }
    }

    return 0;
}

typedef struct struct_message
{
  char id[50];
  int type;

} struct_message;

struct_message myData;

typedef struct pairing_data
{
  char ssid[32];
  char mac_addr[18];
} pairing_data;

struct pairing_data pairingData;

String address = WiFi.macAddress();
char mac_add[50];

int attempts = 6;

void handshake()
{
  // esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  esp_now_send((const uint8_t *)broadcastAddress, (uint8_t *)&pairingData, sizeof(pairingData));
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
{ // Fucntion is activated when ESP receives data on ESPNOW.
  // It copies the received message to memory and sets the received message variable to True to indicate that there is new data to be sent to the server.
  memcpy(&pairingData, incomingData, sizeof(pairingData));
  check_data();

  // Assign homehub's mac address to broadcastAddress
  memcpy(broadcastAddress, mac, 6);

  // Assigning Homehub MAC Address to pairingData.mac_addr
  Serial.println("THIS IS THE SENDER MAC ADDRESS!");
  Serial.println(*mac);

  handshake();
  received_message = true;
}


// Function to convert MAC address from string to byte array
void stringToMacAddress(const String &macStr, uint8_t *macAddr) {
  int byteIndex = 0;
  for (int i = 0; i < macStr.length(); i += 3) {
    String byteStr = macStr.substring(i, i + 2);
    macAddr[byteIndex++] = (uint8_t) strtol(byteStr.c_str(), NULL, 16);
  }
}

// Modified check_pairing_connection to correctly handle MAC address storage/retrieval
void check_pairing_connection() {
  Preferences preferences;
  Serial.println("Checking stored data in EEPROM...");

  // Attempt to retrieve SSID and MAC from EEPROM
  preferences.begin("sensor-data", false);
  String savedSSID = preferences.getString("ssid", "");
  String savedMAC = preferences.getString("mac", "");
  preferences.end();

  if (savedSSID.length() > 0 && savedMAC.length() > 0) {
    // If there is saved data, assign it to the global variables
    strcpy(WIFI_SSID, savedSSID.c_str());

    // Convert saved MAC address string to byte array and store it in broadcastAddress
    stringToMacAddress(savedMAC, broadcastAddress);

    Serial.println("Data loaded from EEPROM:");
    Serial.print("SSID: ");
    Serial.println(WIFI_SSID);
    Serial.print("BROADCAST MAC Address: ");
    for (int i = 0; i < 6; i++) {
      if (i > 0) Serial.print(":");
      Serial.print(broadcastAddress[i], HEX);
    }
    Serial.println();
    return;
  }

  // If no data is saved, wait for pairing data
  Serial.println("No saved data found. Waiting for SSID ...");

  // Waiting for Homehub's pairing data packet
  while (!received_message) {
    delay(300);
  }

  if (strlen(pairingData.ssid) > 0) {
    Serial.print("SSID Received: ");
    Serial.println(pairingData.ssid);
    Serial.print("Homehub MAC Address: ");
    Serial.println(pairingData.mac_addr);

    // Save the received data in EEPROM
    preferences.begin("sensor-data", false);
    preferences.putString("ssid", pairingData.ssid);  // Save SSID as a string
    preferences.putString("mac", String(pairingData.mac_addr));  // Save MAC Address as a string
    preferences.end();

    // Convert received MAC address string to byte array
    stringToMacAddress(pairingData.mac_addr, broadcastAddress);

    // Assign the received data to the sensor variables
    strcpy(WIFI_SSID, pairingData.ssid);
  } else {
    Serial.println("Invalid SSID. Check Homehub connection");
    return;
  }

  Serial.println("SSID assigned!");
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(5000);

  pinMode(15, INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, 0); // Set wake up pin to GPIO_NUM_15

  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  check_pairing_connection();
  send_espnow();

  Serial.println("Going to bed...");
  
  esp_wifi_stop();
  esp_deep_sleep_start();
}

void send_espnow()
{
  address.toCharArray(mac_add, 50);
  Serial.println(mac_add);

  strcpy(myData.id, mac_add);
  myData.type = 1; // Id1 = Bucket Sensor.

  // Register peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.encrypt = false;

  // Add peer
  esp_now_add_peer(&peerInfo);

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  delay(4000); // delay to allow ESP-NOW to work.
}

void loop()
{
}