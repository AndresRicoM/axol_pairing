#ifndef REGISTER_CONTROLLER_H
#define REGISTER_CONTROLLER_H

#include "../pages/registerPage.h"
#include "globals/weather_location/weather_location.h"
#include "globals/globals.h"

void handleRegisterRequest()
{
    // Variables para almacenar datos enviados por el formulario
    String username = wm.server->arg("username");
    String homehubName = wm.server->arg("homehub_name");

    // Obtener la dirección MAC del ESP32
    String macAddr = WiFi.macAddress();

    // Crear cuerpo JSON para la solicitud POST
    JsonDocument jsonDoc;

    jsonDoc["mac_add"] = macAddr;
    jsonDoc["lat"] = lat;
    jsonDoc["lon"] = lon;
    jsonDoc["username"] = username;
    jsonDoc["name"] = homehubName;

    // Serializar JSON
    String jsonBody;
    serializeJson(jsonDoc, jsonBody);

    // Realizar solicitud POST
    JsonDocument jsonResponse;
    String response = homehub::post(jsonBody);
    deserializeJson(jsonResponse, response);

    // Responder al cliente web
    wm.server->send(200, "text/plain", jsonResponse["message"].as<String>());
}

void handleRegister()
{

    JsonDocument location = retrieveLocation();
    lat = location["lat"];
    lon = location["lon"];

    wm.server->send(200, "text/html", registerPage);
}

#endif // REGISTER_CONTROLLER_H