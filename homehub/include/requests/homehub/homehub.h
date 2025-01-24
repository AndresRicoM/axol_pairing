#ifndef HOMEHUB_H
#define HOMEHUB_H

#include "../utils/utils.h"

namespace homehub
{
    /**
     * @brief The endpoint URL for the homehub API.
     */
    String endpoint = "http://192.168.0.8:8000/api/homehub";

    /**
     * @brief Sends a POST request to the homehub endpoint with the provided JSON serialized data.
     *
     * @param data A JSON serialized string containing the data to be sent in the POST request.
     * @return The response from the homehub endpoint as a string.
     */
    String post(String data)
    {
        return utils::postData(endpoint, data);
    }

    /**
     * @brief Retrieves all data from the homehub endpoint.
     * 
     * @return The response from the homehub endpoint as a string.
     */
    String get() {
        return utils::getData(endpoint);
    };

} // namespace homehub

#endif // HOMEHUB_H