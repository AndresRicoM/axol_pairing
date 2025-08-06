#ifndef WIFICONNECTION_H
#define WIFICONNECTION_H

#include <WiFi.h>

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

bool connectToSavedNetwork()
{
    // WiFi Reconnecting Variables
    const unsigned long timeout = 30000; // Tiempo de espera de 30 segundos
    unsigned long startAttemptTime = millis();
    int attemptCounter = 0;
    int totalReconnectAttempts = 0;

    Serial.println("[wificonnection.h] Connecting to saved wifi network...");

    Serial.println("[wificonnection.h] Initialize WiFi...");
    WiFi.begin();
    Serial.println("[wificonnection.h] Setting WIFI_STA...");
    WiFi.mode(WIFI_AP_STA);
    Serial.println("[wificonnection.h] WiFi information after initialization:");
    WiFi.printDiag(Serial);

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout && totalReconnectAttempts < 3)
    {
        delay(1000);
        yield(); // Yield to allow other tasks to run
        Serial.println("[wificonnection.h] Trying to connect to saved network...");

        attemptCounter++;
        if (attemptCounter % 5 == 0)
        { // every 5 attemps, restart wifi connection
            totalReconnectAttempts++;
            Serial.println("[wificonnection.h] Restarting WiFi connection...");
            WiFi.disconnect();
            delay(100);
            WiFi.reconnect();
        }
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        // Wait a two seconds to get IP Address and other configurations from AP
        delay(1000);
        Serial.println("[wificonnection.h] Connected to wifi successfully");
        Serial.println("[wificonnection.h] WiFi information:");
        printNetworkInfo();
    }
    else
    {
        Serial.println("[wificonnection.h] Failed to connect to wifi within the timeout period");
        return false;
    }

    return true;
    Serial.println("-------------------");
}

#endif // WIFICONNECTION_H