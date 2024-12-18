#ifndef SERVER_SEND_H
#define SERVER_SEND_H

#include "globals/weather_location/get_complete_weather.h"
#include "globals/management/management.h"

void server_send()
{ // Sends data to php script on server.
  // The function checks to see if the command is for the regular homehub climate updates or if the command is comming from a known connected sensor.
  // Case Switch function creates different php command depending on the type of sensor that the data corresponds to.

  String endpoint = "http://192.168.1.59:8000/api/homehub";
  String requestBody;
  serializeJson(doc, requestBody);

  Serial.println("requestBody");
  Serial.println(requestBody);
  String response = utils::postData(endpoint, requestBody);
  Serial.println("response");
  Serial.println(response);

  // String command;

  // if (sending_climate)
  // {
  //   get_time();
  //   weather_location::get_complete_weather(lat, lon);
  //   command = {"type=0&id=" + WiFi.macAddress() + "&temp=" + main_temp + "&min_temp=" + main_temp_min +
  //              "&max_temp=" + main_temp_max + "&weather_main=" + weather_0_main + "&weather_description=" +
  //              weather_0_description + "&pressure=" + main_pressure + "&humidity=" + main_humidity +
  //              "&wind_speed=" + wind_speed + "&wind_direction=" + wind_deg + "&datetime=" + formattedDate};
  //   connect_send(command);
  //   sent_time = millis();
  //   sending_climate = false;
  // }

  // if (sending_activity)
  // {

  //   get_time();
  //   command = "type=5&id=" + WiFi.macAddress() + "&activity=" + activity + "&datetime=" + formattedDate;
  //   Serial.println(command);
  //   connect_send(command);
  // }
  // else
  // {

  //   switch (myData.type)
  //   {

  //   case 1:
  //   { // Bucket Sensor
  //     get_time();
  //     String send_id = myData.id;
  //     command = "type=1&id=" + send_id + "&datetime=" + formattedDate;
  //     Serial.println(command);
  //     connect_send(command);
  //   }
  //   break;

  //   case 2: // Tank Sensor
  //   {
  //     get_time();
  //     String send_id = myData.id;
  //     command = "type=2&id=" + send_id + "&water_distance=" + myData.data1 + "&datetime=" + formattedDate;
  //     Serial.println(command);
  //     connect_send(command);
  //   }
  //   break;

  //   case 3: // Temp Humidity Sensor
  //   {
  //     get_time();
  //     String send_id = myData.id;
  //     command = "type=3&id=" + send_id + "&temp=" + myData.data1 + "&humidity=" + myData.data2 + "&datetime=" + formattedDate;
  //     Serial.println(command);
  //     connect_send(command);
  //   }

  //   case 4: // Water Quality Sensor
  //   {
  //     get_time();
  //     String send_id = myData.id;
  //     command = "type=4&id=" + send_id + "&tds=" + myData.data1 + "&water_temp=" + myData.data2 + "&datetime=" + formattedDate;
  //     Serial.println(command);
  //     connect_send(command);
  //   }
  //   break;
  //   }
  // }
}

#endif // SERVER_SEND_H