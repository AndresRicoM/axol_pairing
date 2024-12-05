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

// int32_t getWiFiChannel(const char *ssid) {

//     if (int32_t n = WiFi.scanNetworks()) {
//         for (uint8_t i=0; i<n; i++) {
//             if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
//                 return WiFi.channel(i);
//             }
//         }
//     }

//     return 0;
// }

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

void check_pairing_connection()
{
  // Waiting for Homehub's pairing data packet
  Serial.println("Checking and waiting for SSID ...");
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
    strcpy(WIFI_SSID, pairingData.ssid);
  }
  else
  {
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