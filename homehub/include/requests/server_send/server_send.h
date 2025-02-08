#ifndef SERVER_SEND_H
#define SERVER_SEND_H

#include "globals/weather_location/get_complete_weather.h"
#include "globals/management/management.h"
#include "globals/timeserver/timeserver.h"
#include "../../requests/sensors/bucket.h"
#include "../../requests/sensors/tank.h"
#include "../../requests/sensors/quality.h"
#include "../../requests/climate/climate.h"

void server_send()
{ // Sends data to php script on server.
  // The function checks to see if the command is for the regular homehub climate updates or if the command is comming from a known connected sensor.
  // Case Switch function creates different php command depending on the type of sensor that the data corresponds to.

  // String endpoint = "http://192.168.100.13:8000/api/homehub";
  // String requestBody;
  // serializeJson(doc, requestBody);

  // Serial.println("requestBody");
  // Serial.println(requestBody);
  // String response = utils::postData(endpoint, requestBody);
  // Serial.println("response");
  // Serial.println(response);

  String command;

  if (eventVariables.sending_climate)
  {
    timeserver::get_time();
    weather_location::get_complete_weather(lat, lon);
    command = {"type=0&id=" + WiFi.macAddress() + "&temp=" + main_temp + "&min_temp=" + main_temp_min +
               "&max_temp=" + main_temp_max + "&weather_main=" + weather_0_main + "&weather_description=" +
               weather_0_description + "&pressure=" + main_pressure + "&humidity=" + main_humidity +
               "&wind_speed=" + wind_speed + "&wind_direction=" + wind_deg + "&datetime=" + timeserver::formattedDate};
    climate::connect_send(command);
    eventVariables.sent_time = millis();
    eventVariables.sending_climate = false;
  }
  else
  {

    JsonDocument jsonDoc;

    switch (myData.type)
    {

    case 1:
    { // Bucket Sensor

      timeserver::get_time();
      jsonDoc["mac_add"] = myData.id; // mac_add instead of id
      jsonDoc["datetime"] = timeserver::formattedDate;

      // Serialize JSON
      String jsonBody;
      serializeJson(jsonDoc, jsonBody);

      // Deserialize JSON response
      JsonDocument jsonResponse;
      String response = bucket::post(jsonBody);
      deserializeJson(jsonResponse, response);

      Serial.println("Bucket Sensor Data Sent");
      Serial.println(jsonResponse["message"].as<String>());
    }
    break;

    case 2: // Tank Sensor
    {
      timeserver::get_time();
      // String send_id = myData.id;
      // command = "type=2&id=" + send_id + "&water_distance=" + myData.data1 + "&datetime=" + formattedDate;
      jsonDoc["mac_add"] = myData.id; // mac_add instead of id
      jsonDoc["water_distance"] = myData.data1;
      jsonDoc["datetime"] = timeserver::formattedDate;

      // Serialize JSON
      String jsonBody;
      serializeJson(jsonDoc, jsonBody);

      // Deserialize JSON response
      JsonDocument jsonResponse;
      String response = tank::postData(jsonBody);
      deserializeJson(jsonResponse, response);

      Serial.println(jsonResponse["message"].as<String>());
    }
    break;
    case 4: // Water Quality Sensor
    {
      timeserver::get_time();
      // String send_id = myData.id;
      // command = "type=2&id=" + send_id + "&water_distance=" + myData.data1 + "&datetime=" + formattedDate;
      jsonDoc["mac_add"] = myData.id; // mac_add instead of id
      jsonDoc["tds"] = myData.data1;
      jsonDoc["water_temp"] = myData.data2;
      jsonDoc["datetime"] = timeserver::formattedDate;

      // Serialize JSON
      String jsonBody;
      serializeJson(jsonDoc, jsonBody);

      // Deserialize JSON response
      JsonDocument jsonResponse;
      String response = quality::postData(jsonBody);
      deserializeJson(jsonResponse, response);

      Serial.println(jsonResponse["message"].as<String>());
      // timeserver::get_time();
      // String send_id = myData.id;
      // command = "type=4&id=" + send_id + "&tds=" + myData.data1 + "&water_temp=" + myData.data2 + "&datetime=" + formattedDate;
    }
    break;

      // case 3: // Temp Humidity Sensor
      // {
      //   timeserver::get_time();
      //   String send_id = myData.id;
      //   // command = "type=3&id=" + send_id + "&temp=" + myData.data1 + "&humidity=" + myData.data2 + "&datetime=" + formattedDate;
      // }

      // case 4: // Water Quality Sensor
      // {
      //   timeserver::get_time();
      //   String send_id = myData.id;
      //   // command = "type=4&id=" + send_id + "&tds=" + myData.data1 + "&water_temp=" + myData.data2 + "&datetime=" + formattedDate;
      // }
      // break;
    }
  }
}

// if (sending_activity)
// {

//   timeserver::get_time();
//   command = "type=5&id=" + WiFi.macAddress() + "&activity=" + activity + "&datetime=" + formattedDate;
//   Serial.println(command);
//   connect_send(command);
// }
// else
// {

#endif // SERVER_SEND_H