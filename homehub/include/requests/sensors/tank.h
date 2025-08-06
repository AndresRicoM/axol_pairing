#ifndef TANK_H
#define TANK_H

#include "../utils/utils.h"
#include "env/env.h"
namespace tank
{
    /**
     * @brief The endpoint URL for the sensor API.
     */

    String api_server = ENV_API_SERVER;
    String createSensorEndpoint = api_server + endpointManager.tankCreate;
    String registerTankData = api_server + endpointManager.tankData;

    /**
     * @brief Sends a POST request to the sensor endpoint with the provided JSON serialized data.
     *
     * @param data A JSON serialized string containing the data to be sent in the POST request.
     * @return The response from the sensor endpoint as a string.
     */
    JsonDocument post(String data)
    {
        return utils::postData(registerTankData, data);
    }

    JsonDocument create(String data)
    {
        return utils::postData(createSensorEndpoint, data);
    }

} // namespace sensor

#endif // BUCKET_H