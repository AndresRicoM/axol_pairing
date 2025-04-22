/*
   █████╗ ██╗  ██╗ ██████╗ ██╗        ██████╗ ██╗   ██╗ ██████╗██╗  ██╗███████╗████████╗
  ██╔══██╗╚██╗██╔╝██╔═══██╗██║        ██╔══██╗██║   ██║██╔════╝██║ ██╔╝██╔════╝╚══██╔══╝
  ███████║ ╚███╔╝ ██║   ██║██║        ██████╔╝██║   ██║██║     █████╔╝ █████╗     ██║
  ██╔══██║ ██╔██╗ ██║   ██║██║        ██╔══██╗██║   ██║██║     ██╔═██╗ ██╔══╝     ██║
  ██║  ██║██╔╝ ██╗╚██████╔╝███████╗   ██████╔╝╚██████╔╝╚██████╗██║  ██╗███████╗   ██║
  ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝   ╚═════╝  ╚═════╝  ╚═════╝╚═╝  ╚═╝╚══════╝   ╚═╝


  ᓬ(• - •)ᕒ

*/

#include <Preferences.h> // Include the Preferences library for EEPROM-like functionality
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Arduino.h>
// CONSTANTS
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 1        /* Time ESP32 will go to sleep (in seconds) */
// CONSTRUCTORS
void send_espnow();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
void check_data();
void check_pairing_connection();

////FLAGS FOR SETUP WITH HOMEHUB AND NETWORK////////
bool received_message = false;
bool data_sent = false;

// Receiver address
uint8_t broadcastAddress[6] = {}; // MAC Address for receiving homehub.

int32_t wifi_channel = 13;

/////////////////////////////////////////////////////////////////////

typedef struct struct_message
{
  char id[50];
  int type;
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

  if (savedMAC.length() > 0 && savedMAC != "00:00:00:00:00:00")
  {
    stringToMacAddress(savedMAC, broadcastAddress);

    Serial.println("[check_pairing_connection] Data loaded from EEPROM:");
    Serial.print("[check_pairing_connection] BROADCAST MAC Address: ");
    printMacAddress(broadcastAddress);
    Serial.print("[check_pairing_connection] SavedMAC string: ");
    Serial.println(savedMAC);
    return;
  }

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

void setup()
{
  Serial.begin(115200);
  pinMode(15, INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, 0);

  address.toCharArray(mac_add, 50);
  Serial.println("[setup] MAC Address for this device:");
  Serial.println(mac_add);

  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  // get the status of Trasnmitted packet
  // Once ESPNow is successfully Init, we will register for Send CB to
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

  Serial.println("[setup] WiFi Info...");
  WiFi.printDiag(Serial); // Uncomment to verify channel change after

  Serial.println("Registering callbacks...");
  esp_now_register_send_cb(OnDataSent); // Se registra el callback para mandar datos
  esp_now_register_recv_cb(OnDataRecv); // Se registra el callback para recibir datos (emparejamiento) parece que guarda basura
  
  Serial.println("///////////////////////");
  Serial.println("[setup] BROADCAST ADDRESS FROM SETUP...");
  printMacAddress(broadcastAddress);
  Serial.println("///////////////////////");
  
  Serial.println("[setup] Checking pairing connection...");
  check_pairing_connection();
  delay(100);
  send_espnow();
  
  delay(500);

  esp_wifi_stop();


  Serial.println("Going to sleep now...");
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
  myData.type = 1;

  Serial.println("[send_espnow] Data copied to struct:");
  Serial.print("[send_espnow] ID: ");
  Serial.println(myData.id);
  Serial.print("[send_espnow] Type: ");
  Serial.println(myData.type);

  Serial.println("Registering peer...");
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6); // POSIBLE CAUSA DE ERROR 1
  peerInfo.channel = wifi_channel;                 // Set the channel to the same as the sender
  peerInfo.ifidx = WIFI_IF_STA;                    // Station Interface
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
    Serial.print("Failed to add peer, error code: ");
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
  delay(4000);
}

void loop()
{
}