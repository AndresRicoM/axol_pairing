#ifndef BUCKET_H
#define BUCKET_H

#include "../../utils/utils.h"

namespace bucket
{
    /**
     * @brief The endpoint URL for the sensor API.
     */
    String dataEndpoint = "http://192.168.1.59:8000/api/sensor/bucket";
    String createSensorEndpoint = "http://192.168.1.59:8000/api/sensor/register/bucket";

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

    /**
     * @brief Retrieves all data from the sensor endpoint.
     * 
     * @return The response from the sensor endpoint as a string.
     */
    String get() {
        return utils::getData(dataEndpoint);
    };

} // namespace sensor

#endif // BUCKET_H