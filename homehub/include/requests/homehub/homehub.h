#ifndef HOMEHUB_H
#define HOMEHUB_H

#include "../utils/utils.h"
#include "globals/weather_location/weather_location.h"
#include <ArduinoJson.h>
#include "globals/management/management.h"
#include "env/env.h"
#include "../../globals/management/management.h"

namespace homehub
{
    /**
     * @brief The endpoint URL for the homehub API.
     */
    String api_server = ENV_API_SERVER;

    String createEndpoint = api_server + "/api/homehub";
    String climateEndpoint = api_server + "/api/homehub/weather";
    String activityEndpoint = api_server + "/api/homehub/activity";

    // TODO: recreate this endpoint in Laravel
    String systemStatsEndpoint = "http://blindspot.media.mit.edu/homehubweb/hh_updates.php?id=" + WiFi.macAddress();

    /**
     * @brief Sends a POST request to the homehub endpoint with the provided JSON serialized data.
     *
     * @param data A JSON serialized string containing the data to be sent in the POST request.
     * @return The response from the homehub endpoint as a string.
     */
    JsonDocument create(String data)
    {
        return utils::postData(createEndpoint, data);
    }

    JsonDocument sendClimate(String data)
    {
        return utils::postData(climateEndpoint, data);
    }

    JsonDocument sendActivity(String data)
    {
        return utils::postData(activityEndpoint, data);
    }

    /**
     * @brief Retrieves all data from the homehub endpoint.
     *
     * @return The response from the homehub endpoint as a string.
     */
    void getSystemStats()
    {
        Serial.println("[homehub.h] getSystemStats()");
        Serial.println("[homehub.h] systemStatsEndpoint: ");
        Serial.println(systemStatsEndpoint);
        Serial.println("[homehub.h] WiFi.macAddress(): ");
        Serial.println(WiFi.macAddress());
        
        JsonDocument response = utils::getData(systemStatsEndpoint);
        String message = response["message"];

        Serial.println("[homehub.h] getSystemStats() response.message: ");
        Serial.println(message);

        DeserializationError error = deserializeJson(weather_location::doc, message);

        if (error)
        {
            Serial.print("[systemStats.h] deserializeJson() failed: ");
            Serial.println(error.c_str());
            return;
        }

        waterManager.fill_percentage = weather_location::doc["percentage"];
        waterManager.buckets = weather_location::doc["bucketNum"];
        waterManager.tanks = weather_location::doc["tankNum"];
        waterManager.avail_storage = weather_location::doc["availStorage"];
        waterManager.avail_liters = weather_location::doc["availLiters"];
        waterManager.quality = weather_location::doc["qualityNum"];
        waterManager.envs = weather_location::doc["envNum"];
        waterManager.dev_name = weather_location::doc["name"];
        weather_location::lat = weather_location::doc["lat"];
        weather_location::lon = weather_location::doc["lon"];
    }

} // namespace homehub

#endif // HOMEHUB_H