#include <Preferences.h> // Include the Preferences library for EEPROM-like functionality
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Arduino.h>

// CONSTRUCTORS
void send_espnow();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
int32_t getWiFiChannel(const char *ssid);

bool received_message = false;

// Receiver address
uint8_t broadcastAddress[] = {255, 255, 255, 255, 255, 255}; // MAC Address for receiving homehub.

// // // char WIFI_SSID[32] = ""; // Network name, no password required.

int32_t wifi_channel = 13;

// // // int32_t getWiFiChannel(const char *ssid)
// // // {
// // //   Serial.println("Scanning Networks...");
// // //   Serial.println(ssid);

// // //   // Ensure WiFi is in STA mode
// // //   WiFi.mode(WIFI_STA);

// // //   int32_t n = WiFi.scanNetworks();
// // //   Serial.println("Number of Networks found:");
// // //   Serial.println(n);

// // //   if (n > 0)
// // //   {
// // //     for (uint8_t i = 0; i < n; i++)
// // //     {
// // //       if (!strcmp(ssid, WiFi.SSID(i).c_str()))
// // //       {
// // //         return WiFi.channel(i);
// // //       }
// // //     }
// // //   }

// // //   return 0;
// // // }

typedef struct struct_message
{
  char id[50];
  int type;
} struct_message;

struct_message myData;

typedef struct pairing_data
{
  // // // char ssid[32];
  char mac_addr[18];
} pairing_data;

struct pairing_data pairingData = {};

String address = WiFi.macAddress();
char mac_add[50];

void handshake()
{
  esp_err_t result = esp_now_send((const uint8_t *)broadcastAddress, (uint8_t *)&pairingData, sizeof(pairingData));
  if (result != ESP_OK) {
    Serial.print("Error sending handshake: ");
    Serial.println(result);
  }
}

void check_data()
{
  Serial.println("Pairing Data!");
  Serial.print("[check_data]: Homehub MAC Address:");
  Serial.println(pairingData.mac_addr);
  // // // Serial.print("Router SSID:");
  // // // Serial.println(pairingData.ssid);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  // Función callback que registra el dato enviado desde el homehub para hacer el pairing
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
  // Formatear la mac address. No devuelve nada. Es nomás para imprimir.
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
  // // // String savedSSID = preferences.getString("ssid", "");
  String savedMAC = preferences.getString("mac", "");
  preferences.end();

  // // // if (savedSSID.length() > 0 && savedMAC.length() > 0)
  if (savedMAC.length() > 0)
  {
    // // // strcpy(WIFI_SSID, savedSSID.c_str());
    stringToMacAddress(savedMAC, broadcastAddress);

    Serial.println("Data loaded from EEPROM:");
    // // // Serial.print("SSID: ");
    // // // Serial.println(WIFI_SSID);
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

  /* Cuando no hay datos guardados en eeprom, entonces espera por el dato y lo escribe en eeprom */
  Serial.println("No saved data found. Waiting for SSID ...");

  while (!received_message)
  {
    delay(300);
  }

  // // // if (strlen(pairingData.ssid) > 0)
  if (strlen(pairingData.mac_addr) > 0)
  {
    // // // Serial.print("SSID Received: ");
    // // // Serial.println(pairingData.ssid);
    Serial.print("Homehub MAC Address: ");
    Serial.println(pairingData.mac_addr);

    // Para escribir
    preferences.begin("sensor-data", false);
    // // // preferences.putString("ssid", pairingData.ssid);
    preferences.putString("mac", String(pairingData.mac_addr));
    preferences.end();

    stringToMacAddress(pairingData.mac_addr, broadcastAddress);
    // // // strcpy(WIFI_SSID, pairingData.ssid);
  
  }
  else
  {
    // // // Serial.println("Invalid SSID. Check Homehub connection");
    Serial.println("Invalid MAC_ADD. Check Homehub MAC_ADDRESS");
    return;
  }

  // // // Serial.println("SSID assigned!");
}

void setup()
{
  Serial.begin(115200);

  Serial.println("Starting ESP32...");
  pinMode(15, INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, 0);

  Serial.println("Starting WiFi...");
  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  Serial.println("Registering callbacks...");
  esp_now_register_send_cb(OnDataSent); // Se registra el callback para mandar datos
  esp_now_register_recv_cb(OnDataRecv); // Se registra el callback para recibir datos (emparejamiento) parece que guarda basura

  Serial.println("Checking pairing connection...");
  check_pairing_connection();
  send_espnow();

  Serial.println("Stopping esp_wifi...");
  esp_wifi_stop();

  Serial.println("Debugging information before deep sleep:");
  // // // Serial.println(WIFI_SSID);
  Serial.print("broadcastAddress: ");
  for (int i = 0; i < 6; i++)
  {
    if (i > 0)
      Serial.print(":");
    Serial.print(broadcastAddress[i], HEX);
  }
  Serial.println();

  Serial.println("Going to sleep now...");
  esp_deep_sleep_start();
}

void send_espnow()
{
  address.toCharArray(mac_add, 50);
  Serial.println(mac_add);

  Serial.println("Changing WiFi Channel...");
  // // // Serial.println("Current wifi ssid: ");
  // // // Serial.println(WIFI_SSID);
  // // // wifi_channel = getWiFiChannel(WIFI_SSID);

  Serial.println("Wifi channel is:");
  Serial.println(wifi_channel);

  esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);

  Serial.println("Copying data to struct...");
  strcpy(myData.id, mac_add);
  myData.type = 1;

  Serial.println("Registering peer...");
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6); // POSIBLE CAUSA DE ERROR 1
  peerInfo.encrypt = false;

  esp_err_t addPeerResult = esp_now_add_peer(&peerInfo);
  if (addPeerResult != ESP_OK) {
    Serial.print("Failed to add peer, error code: ");
    Serial.println(addPeerResult);
  }

  Serial.println("Sending data via ESP-NOW...");
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  if (result != ESP_OK) {
    Serial.print("Failed to send data, error code: ");
    Serial.println(result);
  }
  delay(4000);
}

void loop()
{
}