#include <WiFiManager.h>
#include <strings_en.h>
#include <wm_consts_en.h>
#include <wm_strings_en.h>
#include <wm_strings_es.h>
#include <WiFi.h>

WiFiManager wm;
const int ERASE_WIFI_CREDENTIALS = -1;
const int CONNECT_HH = 5;
const int REGISTER_HH = 6;

void setup() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);  // 115200 badios
}

void loop() {
  // Getting value from Serial Monitor for debugging.
  // By default, debug_value will keep receiving zero.
  if (Serial.available() > 0) {
    int debug_value = Serial.parseInt();

    switch (debug_value) {
      case ERASE_WIFI_CREDENTIALS:
        wm.resetSettings();
        break;

      case CONNECT_HH:
        if (!establishWiFiConnection()) {
          Serial.println("Couldn't connect to the network");
        } else {
          Serial.println("Connected!");
          printNetworkInfo();
        }
        break;
    }
  }
}

bool establishWiFiConnection() {
  if (!wm.autoConnect("Axol")) {
    return false;
  }

  return true;
}

void printNetworkInfo() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());  // Nombre de la red WiFi

  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());  // Dirección IP del ESP32

  Serial.print("Intensidad de señal (RSSI): ");
  Serial.println(WiFi.RSSI());  // Intensidad de la señal en dBm

  Serial.print("Dirección MAC: ");
  Serial.println(WiFi.macAddress());  // Dirección MAC del ESP32

  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());  // Dirección IP del gateway

  Serial.print("Máscara de Subred: ");
  Serial.println(WiFi.subnetMask());  // Máscara de subred

  Serial.print("DNS Primario: ");
  Serial.println(WiFi.dnsIP());  // Dirección IP del DNS primario
}
