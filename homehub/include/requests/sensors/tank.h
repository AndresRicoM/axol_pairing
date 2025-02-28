#ifndef TANK_H
#define TANK_H

#include "../utils/utils.h"

namespace tank
{
    /**
     * @brief The endpoint URL for the sensor API.
     */
    // String createSensorEndpoint = "http://blindspot.media.mit.edu:8000/api/sensor/tank";
    // String registerTankData = "http://blindspot.media.mit.edu:8000/api/sensor/tankData";
    String createSensorEndpoint = "http://192.168.1.59:8000/api/sensor/tank";
    String registerTankData = "http://192.168.1.59:8000/api/sensor/tankData";

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