#ifndef QUALITY_H
#define QUALITY_H

#include "../utils/utils.h"
#include "globals/globals.h"

namespace quality
{
    /**
     * @brief The endpoint URL for the sensor API.
     */
    String api_server = ENV_API_SERVER;
    String debugDataEndpoint = "/api/debug/sensor";

    String createSensorEndpoint;
    String registerQualityData;

    /**
     * @brief Sends a POST request to the sensor endpoint with the provided JSON serialized data.
     *
     * @param data A JSON serialized string containing the data to be sent in the POST request.
     * @return The response from the sensor endpoint as a string.
     */
    JsonDocument post(String data)
    {
        registerQualityData = api_server + (isDebugMode ? debugDataEndpoint : "/api/sensor/quality");
        return utils::postData(registerQualityData, data);
    }

    JsonDocument create(String data)
    {
        createSensorEndpoint = api_server + (isDebugMode ? debugDataEndpoint : "/api/sensor/qualityData");
        return utils::postData(createSensorEndpoint, data);
    }

} // namespace sensor

#endif // QUALITY_H