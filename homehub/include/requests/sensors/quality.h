#ifndef QUALITY_H
#define QUALITY_H

#include "../utils/utils.h"

namespace quality
{
    /**
     * @brief The endpoint URL for the sensor API.
     */
    // String createSensorEndpoint = "http://blindspot.media.mit.edu:8000/api/sensor/quality";
    // String registerQualityData = "http://blindspot.media.mit.edu:8000/api/sensor/qualityData";
    String createSensorEndpoint = "http://192.168.1.59:8000/api/sensor/quality";
    String registerQualityData = "http://192.168.1.59:8000/api/sensor/qualityData";

    /**
     * @brief Sends a POST request to the sensor endpoint with the provided JSON serialized data.
     *
     * @param data A JSON serialized string containing the data to be sent in the POST request.
     * @return The response from the sensor endpoint as a string.
     */
    JsonDocument post(String data)
    {
        return utils::postData(registerQualityData, data);
    }

    JsonDocument create(String data)
    {
        return utils::postData(createSensorEndpoint, data);
    }

} // namespace sensor

#endif // QUALITY_H