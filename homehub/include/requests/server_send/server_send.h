#ifndef SERVER_SEND_H
#define SERVER_SEND_H

#include "globals/management/management.h"
#include "globals/timeserver/timeserver.h"
#include "../../requests/sensors/bucket.h"
#include "../../requests/sensors/tank.h"
#include "../../requests/sensors/quality.h"
#include "../../requests/homehub/homehub.h"
#include "../../requests/system/wificonnection.h"

void server_send()

{ // Sends data to Laravel server.
  // The function checks to see if the command is for the regular homehub climate updates or if the command is comming from a known connected sensor.
  // Case Switch function creates different Laravel endpoints depending on the type of sensor that the data corresponds to.

  Serial.println("*** [server_send.h] Sending data to server ***");
  // connect to the internet
  if (!connectToSavedNetwork())
  {
    Serial.println("[server_send.h] Couldn't connect to the internet.");
    Serial.println("[server_send.h] Restarting Homehub...");
    ESP.restart();
    return;
  }

  if (eventVariables.sending_activity)
  {
    JsonDocument jsonDoc;
    jsonDoc["mac_add"] = WiFi.macAddress();
    jsonDoc["activity"] = eventVariables.activity;
    Serial.println("[server_send.h] Activity body data:");
    Serial.println(jsonDoc.as<String>());

    Serial.println("[server_send.h] Sending Activity Data");
    String jsonBody;
    serializeJson(jsonDoc, jsonBody);
    JsonDocument response = homehub::sendActivity(jsonBody);
    String message = response["message"];
    Serial.println("[server_send.h] Activity Data Sent");
    Serial.println("[server_send.h] Activity response message: ");
    Serial.println(message);

    eventVariables.sending_activity = false;
    return;
  }

  if (eventVariables.sending_climate)
  {
    JsonDocument jsonDoc;

    weather_location::get_complete_weather(weather_location::lat, weather_location::lon);

    jsonDoc["mac_add"] = WiFi.macAddress();
    jsonDoc["temp"] = weather_location::main_temp;
    jsonDoc["min_temp"] = weather_location::main_temp_min;
    jsonDoc["max_temp"] = weather_location::main_temp_max;
    jsonDoc["weather_main"] = weather_location::weather_0_main;
    jsonDoc["weather_description"] = weather_location::weather_0_description;
    jsonDoc["pressure"] = weather_location::main_pressure;
    jsonDoc["humidity"] = weather_location::main_humidity;
    jsonDoc["wind_speed"] = weather_location::wind_speed;
    jsonDoc["wind_direction"] = weather_location::wind_deg;

    // Serialize JSON
    String jsonBody;
    serializeJson(jsonDoc, jsonBody);

    // Deserialize JSON response
    JsonDocument jsonResponse = homehub::sendClimate(jsonBody);
    String message = jsonResponse["message"];
    Serial.println("[server_send.h] Weather Data Sent");
    Serial.println(message);

    eventVariables.sent_time = millis();
    eventVariables.sending_climate = false;

    return;
  }

  switch (myData.type)
  {

  case 1:
  { // Bucket Sensor
    JsonDocument jsonDoc;

    jsonDoc["mac_add"] = myData.id; // mac_add instead of id

    // Serialize JSON
    String jsonBody;
    serializeJson(jsonDoc, jsonBody);

    // Deserialize JSON response
    JsonDocument jsonResponse = bucket::post(jsonBody);

    Serial.println("Bucket Sensor Data Sent");
    Serial.println(jsonResponse["message"].as<String>());
  }
  break;

  case 2: // Tank Sensor
  {
    JsonDocument jsonDoc;

    // String send_id = myData.id;
    // command = "type=2&id=" + send_id + "&water_distance=" + myData.data1 + "&datetime=" + formattedDate;
    jsonDoc["mac_add"] = myData.id; // mac_add instead of id
    jsonDoc["water_distance"] = myData.data1;

    // Serialize JSON
    String jsonBody;
    serializeJson(jsonDoc, jsonBody);

    // Deserialize JSON response
    JsonDocument jsonResponse = tank::post(jsonBody);

    Serial.println(jsonResponse["message"].as<String>());
  }
  break;
  case 4: // Water Quality Sensor
  {
    JsonDocument jsonDoc;

    jsonDoc["mac_add"] = myData.id; // mac_add instead of id
    jsonDoc["tds"] = myData.data1;
    jsonDoc["water_temp"] = myData.data2;
    jsonDoc["humidity"] = myData.data3;

    // Serialize JSON
    String jsonBody;
    serializeJson(jsonDoc, jsonBody);

    // Deserialize JSON response
    JsonDocument jsonResponse = quality::post(jsonBody);

    Serial.println(jsonResponse["message"].as<String>());
  }
  break;
  }

  return;
}

#endif // SERVER_SEND_H