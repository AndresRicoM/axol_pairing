#ifndef BUCKET_CONTROLLER_H
#define BUCKET_CONTROLLER_H

#include <WiFiManager.h>
#include "globals/globals.h"
#include "globals/management/management.h"
// #include "../pages/sensorBucketPage.h"
#include "../../requests/sensors/bucket.h"

void handleSensorBucketRequest()
{
    // Variables para almacenar datos enviados por el formulario
    String capacity = wm.server->arg("b_capacity");
    String use = wm.server->arg("b_use");

    // Obtener la dirección MAC del ESP32
    String macAddr = WiFi.macAddress();

    // Crear cuerpo JSON para la solicitud POST
    JsonDocument jsonDoc;

    jsonDoc["mac_add"] = myData.id;
    jsonDoc["paired_with"] = macAddr;
    jsonDoc["buck_capacity"] = capacity;
    jsonDoc["use"] = use;

    // Serializar JSON
    String jsonBody;
    serializeJson(jsonDoc, jsonBody);

    // Realizar solicitud POST
    JsonDocument jsonResponse = bucket::createSensor(jsonBody);

    // Responder al cliente web
    wm.server->send(200, "text/plain", jsonResponse["message"].as<String>());
    delay(3000);
    wm.server->send(200, "text/plain", setupPageNoHomehub);
}
#endif