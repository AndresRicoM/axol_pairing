#ifndef TANK_CONTROLLER_H
#define TANK_CONTROLLER_H

#include <WiFiManager.h>
#include "globals/globals.h"
#include "globals/management/management.h"
#include "../pages/sensorBucketPage.h"
#include "../../requests/sensors/tank.h"

void handleSensorTankRequest()
{
    // Variables para almacenar datos enviados por el formulario
    String capacity = wm.server->arg("tank_capacity");
    String use = wm.server->arg("tank_use");
    String area = wm.server->arg("tank_area");
    String height = wm.server->arg("tank_height");

    // Obtener la dirección MAC del ESP32
    String macAddr = WiFi.macAddress();

    // Crear cuerpo JSON para la solicitud POST
    JsonDocument jsonDoc;

    jsonDoc["mac_add"] = myData.id;
    jsonDoc["paired_with"] = macAddr;
    jsonDoc["tank_capacity"] = capacity;
    jsonDoc["use"] = use;
    jsonDoc["tank_area"] = area;
    jsonDoc["max_height"] = height;

    // Serializar JSON
    String jsonBody;
    serializeJson(jsonDoc, jsonBody);

    // Realizar solicitud POST
    JsonDocument jsonResponse;
    String response = tank::createSensor(jsonBody);
    deserializeJson(jsonResponse, response);

    // Responder al cliente web
    wm.server->send(200, "text/plain", jsonResponse["message"].as<String>());
    delay(3000);
    wm.server->send(200, "text/plain", setupPageNoHomehub);
}

#endif