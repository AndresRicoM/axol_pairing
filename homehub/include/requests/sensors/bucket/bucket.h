#ifndef BUCKET_H
#define BUCKET_H

#include "../../utils/utils.h"

namespace bucket
{
    /**
     * @brief The endpoint URL for the sensor API.
     */
    String endpoint = "http://192.168.1.9:8000/api/sensors/bucket";

    /**
     * @brief Sends a POST request to the sensor endpoint with the provided JSON serialized data.
     *
     * @param data A JSON serialized string containing the data to be sent in the POST request.
     * @return The response from the sensor endpoint as a string.
     */
    String post(String data)
    {
        return utils::postData(endpoint, data);
    }

    /**
     * @brief Retrieves all data from the sensor endpoint.
     * 
     * @return The response from the sensor endpoint as a string.
     */
    String get() {
        return utils::getData(endpoint);
    };

} // namespace sensor

#endif // BUCKET_H