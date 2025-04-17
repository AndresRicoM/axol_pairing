#ifndef WEATHER_LOCATION_H
#define WEATHER_LOCATION_H

#include <ArduinoJson.h>
#include "globals/weather_location/weather_location.h"
#include "requests/utils/utils.h"
#include "env/env.h"

namespace weather_location
{
    // Latitude & Longitude
    double lat, lon;

    // Weather/Location Server Variables
    JsonDocument doc;
    float coord_lon = doc["coord"]["lon"];
    float coord_lat = doc["coord"]["lat"];

    JsonObject weather_0;
    const char *weather_0_main = weather_0["main"];
    const char *weather_0_description = weather_0["description"];
    const char *weather_0_icon = weather_0["icon"];

    JsonObject main1;
    float main_temp = main1["temp"];
    float main_temp_min = main1["temp_min"];
    float main_temp_max = main1["temp_max"];

    int main_pressure = main1["pressure"];
    int main_humidity = main1["humidity"];

    JsonObject wind;
    float wind_speed = wind["speed"];
    int wind_deg = wind["deg"];

    JsonObject sys;
    int sys_type = sys["type"];
    long sys_id = sys["id"];
    const char *sys_country = sys["country"];
    long sys_sunrise = sys["sunrise"];
    long sys_sunset = sys["sunset"];
    int timezone = doc["timezone"];
    long id = doc["id"];
    const char *city_name = doc["name"];
    int cod = doc["cod"];

    void get_complete_weather(double &lat, double &lon)
    {
        // gets weather and location information.

        const String endpoint_base = "https://api.openweathermap.org/data/2.5/weather?lat=" + String(lat, 7) + "&lon=" + String(lon, 7) + "&appid=";
        const String key = WEATHER_KEY;
        String endpoint = endpoint_base + key;

        JsonDocument response = utils::getData(endpoint);
        String message = response["message"];

        Serial.println("[get_complete_weather.h] Response message: ");
        Serial.println(message);

        DeserializationError error = deserializeJson(doc, message);

        if (error)
        {
            Serial.print("[get_complete_weather.h] deserializeJson() failed: ");
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

#endif // WEATHER_LOCATION_H