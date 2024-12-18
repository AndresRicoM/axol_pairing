#ifndef WEATHER_LOCATION_H
#define WEATHER_LOCATION_H

#include <ArduinoJson.h>

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

#endif // WEATHER_LOCATION_H