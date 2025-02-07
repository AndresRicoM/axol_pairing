#ifndef QUALITY_CONTROLLER_H
#define QUALITY_CONTROLLER_H

#include <WiFiManager.h>
#include "globals/globals.h"
#include "globals/management/management.h"
#include "../pages/sensorQualityPage.h"
#include "../../requests/sensors/quality.h"

void handleSensorQualityRequest()
{
    // Variables para almacenar datos enviados por el formulario
    String use = wm.server->arg("quality_use");

    // Obtener la dirección MAC del ESP32
    String macAddr = WiFi.macAddress();

    // Crear cuerpo JSON para la solicitud POST
    JsonDocument jsonDoc;

    jsonDoc["mac_add"] = myData.id;
    jsonDoc["paired_with"] = macAddr;
    jsonDoc["use"] = use;

    // Serializar JSON
    String jsonBody;
    serializeJson(jsonDoc, jsonBody);

    // Realizar solicitud POST
    JsonDocument jsonResponse;
    String response = quality::createSensor(jsonBody);
    deserializeJson(jsonResponse, response);

    // Responder al cliente web
    wm.server->send(200, "text/plain", jsonResponse["message"].as<String>());
}

#endif