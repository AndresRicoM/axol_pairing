#ifndef BUCKET_H
#define BUCKET_H

#include "../utils/utils.h"

namespace bucket
{
    /**
     * @brief The endpoint URL for the sensor API.
     */
    String dataEndpoint = "http://blindspot.media.mit.edu:8000/api/sensor/bucket";
    String createSensorEndpoint = "http://blindspot.media.mit.edu:8000/api/sensor/bucket";
    String registerBucketData = "http://blindspot.media.mit.edu:8000/api/sensor/bucketData";


    // Route::post('/sensor/bucket', [BucketController::class, 'registerBucket']);
    // Route::get('/sensor/bucket', [BucketController::class, 'getBucket']);

    /**
     * @brief Sends a POST request to the sensor endpoint with the provided JSON serialized data.
     *
     * @param data A JSON serialized string containing the data to be sent in the POST request.
     * @return The response from the sensor endpoint as a string.
     */
    String post(String data)
    {
        return utils::sensorData(dataEndpoint, data);
    }

    String postData(String data)
    {
        return utils::sensorData(registerBucketData, data);
    }

    JsonDocument createSensor(String data)
    {
        return utils::postData(createSensorEndpoint, data);
    }

    /**
     * @brief Retrieves all data from the sensor endpoint.
     *
     * @return The response from the sensor endpoint as a string.
     */
    String get()
    {
        return utils::getData(dataEndpoint);
    };


} // namespace sensor

#endif // BUCKET_H