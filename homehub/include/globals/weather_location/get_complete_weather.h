#ifndef GET_COMPLETE_WEATHER_H
#define GET_COMPLETE_WEATHER_H

#include <ArduinoJson.h>
#include "globals/weather_location/weather_location.h"
#include "requests/utils/utils.h"

namespace weather_location
{
    void get_complete_weather(double& lat, double& lon)
    {
        // gets weather and location information.

        const String endpoint_base = "https://api.openweathermap.org/data/2.5/weather?lat=" + String(lat, 7) + "&lon=" + String(lon, 7) + "&appid=";
        const String key = "api key";
        String endpoint = endpoint_base + key;

        String response = utils::getData(endpoint);

        Serial.println(response);

        DeserializationError error = deserializeJson(doc, response);

        if (error)
        {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            return;
        }

        coord_lon = doc["coord"]["lon"];
        coord_lat = doc["coord"]["lat"];

        weather_0 = doc["weather"][0];
        weather_0_main = weather_0["main"];
        weather_0_description = weather_0["description"];
        weather_0_icon = weather_0["icon"];

        main1 = doc["main"];
        main_temp = main1["temp"];
        main_temp_min = main1["temp_min"];
        main_temp_max = main1["temp_max"];
        main_pressure = main1["pressure"];
        main_humidity = main1["humidity"];

        wind = doc["wind"];
        wind_speed = wind["speed"];
        wind_deg = wind["deg"];

        timezone = doc["timezone"];
        id = doc["id"]; // 6692163
        city_name = doc["name"];
        cod = doc["cod"];
    }

}

#endif // GET_COMPLETE_WEATHER_H