#ifndef SYSTEM_STATS_H
#define SYSTEM_STATS_H

#include "globals/globals.h"
#include "../utils/utils.h"
#include "globals/weather_location/weather_location.h"
#include <ArduinoJson.h>
#include "globals/management/management.h"

// void get_system_stats()
// {

//     // String endpoint = "http://blindspot.media.mit.edu/homehubweb/hhdash.php?id=" + WiFi.macAddress();
//     String endpoint = "http://blindspot.media.mit.edu/homehubweb/hh_updates.php?id=" + WiFi.macAddress();
//     Serial.println(endpoint);
//     String response = utils::getData(endpoint);
//     Serial.println(response);

//     DeserializationError error = deserializeJson(doc, response);

//     if (error)
//     {
//         Serial.print("[systemStats.h] deserializeJson() failed: ");
//         Serial.println(error.c_str());
//         return;
//     }

//     waterManager.fill_percentage = doc["percentage"];
//     waterManager.buckets = doc["bucketNum"];
//     waterManager.tanks = doc["tankNum"];
//     waterManager.avail_storage = doc["availStorage"];
//     waterManager.avail_liters = doc["availLiters"];
//     waterManager.quality = doc["qualityNum"];
//     waterManager.envs = doc["envNum"];
//     waterManager.dev_name = doc["name"];
//     lat = doc["lat"];
//     lon = doc["lon"];

// }

#endif // SYSTEM_STATS_H