#ifndef REGISTER_CONTROLLER_H
#define REGISTER_CONTROLLER_H

#include "../pages/registerPage.h"
#include "globals/weather_location/weather_location.h"
#include "globals/globals.h"
#include <EEPROM.h>

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
    // JsonDocument jsonResponse;
    JsonDocument jsonResponse = homehub::post(jsonBody);
    int responseCode = jsonResponse["code"];

    EEPROM.begin(EEPROM_SIZE); // Initialize EEPROM with the specified size
    // If the response code is 201 (created successfully), mark the ESP32 as registered in EEPROM
    if (responseCode == 201)
    {
        EEPROM.begin(EEPROM_SIZE); // Initialize EEPROM with the defined size
        EEPROM.write(0, 1);        // Write 1 to position 0 to indicate that the device is registered
        EEPROM.commit();           // Save changes to EEPROM
        Serial.println("Success. Homehub has been marked as registered in EEPROM.");
    }
    else
    {
        Serial.println("Request failed. Code: " + String(responseCode));
    }
    
    // deserializeJson(jsonResponse, response);

    // Responder al cliente web
    wm.server->send(200, "text/plain", jsonResponse["message"].as<String>());
}

void handleRegister()
{

    // Getting the nested values from the message attribute of the location response
    JsonDocument location = retrieveLocation();
    String message = location["message"];

    JsonDocument nestedDoc;
    DeserializationError error = deserializeJson(nestedDoc, message);

    if (error)
    {
        Serial.print("Failed to parse nested JSON: ");
        Serial.println(error.c_str());
    }
    else
    {
        Serial.println("[registerController] setting lat and lon from location response:");
        lat = nestedDoc["lat"];
        lon = nestedDoc["lon"];
        Serial.println(lat);
        Serial.println(lon);
    }

    wm.server->send(200, "text/html", registerPage);
}

#endif // REGISTER_CONTROLLER_H