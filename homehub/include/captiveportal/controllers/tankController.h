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
    String offset = wm.server->arg("offset");
    // String area = wm.server->arg("tank_area");
    String height = wm.server->arg("tank_height");

    String type = wm.server->arg("tank_type");


    // Variables condicionales según el tipo de tanque
    String diameter = "";
    String width = "";
    String depth = "";

    // Obtener la dirección MAC del ESP32
    String macAddr = WiFi.macAddress();

    // Crear cuerpo JSON para la solicitud POST
    JsonDocument jsonDoc;

    jsonDoc["mac_add"] = myData.id;
    jsonDoc["paired_with"] = macAddr;
    jsonDoc["tank_capacity"] = capacity;
    jsonDoc["use"] = use;
    // jsonDoc["tank_area"] = area;
    jsonDoc["tank_area"] = 0;
    jsonDoc["max_height"] = height;
    jsonDoc["height"] = height;
    jsonDoc["offset"] = offset;

    if (type == "cylindrical") {
            diameter = wm.server->arg("diameter");
            jsonDoc["diameter"] = diameter;
        } else if (type == "rectangular") {
            width = wm.server->arg("width");
            depth = wm.server->arg("depth");
            jsonDoc["width"] = width;
            jsonDoc["depth"] = depth;
        }
    // Serializar JSON
    String jsonBody;
    serializeJson(jsonDoc, jsonBody);
    // Imprimir el JSON en el monitor serie
    Serial.println(jsonBody);
    // Realizar solicitud POST
    JsonDocument jsonResponse = tank::create(jsonBody);

    // Responder al cliente web
    wm.server->send(200, "text/plain", jsonResponse["message"].as<String>());
    delay(3000);
    wm.server->send(200, "text/plain", setupPageNoHomehub);
}

#endif