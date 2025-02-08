#ifndef CLIMATE_H
#define CLIMATE_H

#include "../utils/utils.h"

namespace climate
{
    /**
     * @brief The endpoint URL for the homehub API.
     */
    String endpoint = "http://blindspot.media.mit.edu:8000/api/homehub";

    /**
     * @brief Sends a POST request to the homehub endpoint with the provided JSON serialized data.
     *
     * @param data A JSON serialized string containing the data to be sent in the POST request.
     * @return The response from the homehub endpoint as a string.
     */
    void connect_send(String php_command) { //Send Data to PHP server
        //Function takes command as argument and sends a POST request to server.
      
        HTTPClient http;
        WiFiClient client;
      
        //String server_main = "http://blindspot.media.mit.edu/homehubweb/hh_updates.php?id=" + WiFi.macAddress();
        String server_main = "http://blindspot.media.mit.edu/homehub.php";
      
        http.begin(client, server_main);
      
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        String httpRequestData = php_command;
      
        int httpResponseCode = http.POST(httpRequestData);
      
        if (httpResponseCode > 0) {
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
          String payload = http.getString();
          Serial.println(payload);
        }
        else {
          Serial.print("Error code: ");
          Serial.println(httpResponseCode);
        }
        // Free resources
        http.end();
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