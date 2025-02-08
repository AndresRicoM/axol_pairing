#ifndef QUALITY_H
#define QUALITY_H

#include "../utils/utils.h"

namespace quality
{
    /**
     * @brief The endpoint URL for the sensor API.
     */
    String dataEndpoint = "http://blindspot.media.mit.edu:8000/api/sensor/quality";
    //String createSensorEndpoint = "http://127.0.0.1:8000/api/sensor/register/tank";
    String createSensorEndpoint = "http://blindspot.media.mit.edu:8000/api/sensor/quality";
    String registerQualityData = "http://blindspot.media.mit.edu:8000/api/sensor/qualityData";

    /**
     * @brief Sends a POST request to the sensor endpoint with the provided JSON serialized data.
     *
     * @param data A JSON serialized string containing the data to be sent in the POST request.
     * @return The response from the sensor endpoint as a string.
     */
    String post(String data)
    {
        return utils::postData(dataEndpoint, data);
    }

    String createSensor(String data)
    {
        return utils::postData(createSensorEndpoint, data);
    }

    String postData(String data)
    {
        return utils::sensorData(registerQualityData, data);
    }


    /**
     * @brief Retrieves all data from the sensor endpoint.
     * 
     * @return The response from the sensor endpoint as a string.
     */
    String get() {
        return utils::getData(dataEndpoint);
    };

} // namespace sensor

#endif // QUALITY_H