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

#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    delay(5000); // Retraso para estabilizar el puerto
    for (int i = 0; i < 10; i++) {
        Serial.println("ESP32-C3 funcionando correctamente.");
        delay(500);
    }
}

void loop() {
    delay(1000);
}