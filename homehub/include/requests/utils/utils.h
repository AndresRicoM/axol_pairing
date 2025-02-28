#ifndef UTILS_H
#define UTILS_H
#define EEPROM_SIZE 1

#include <HTTPClient.h>

namespace utils
{
    JsonDocument postData(String &endpoint, String &requestBody)
    {
        HTTPClient http;
        String response;

        JsonDocument http_response;

        http.begin(endpoint);
        http.addHeader("Content-Type", "application/json");

        Serial.println("[utils.h] Entrando a postdata.h...");
        Serial.println("[utils.h] requestBody: ");
        Serial.println(requestBody);
        Serial.println("[utils.h] endpoint: ");
        Serial.println(endpoint);
        int responseCode = http.POST(requestBody);

        Serial.println("[utils.h] response code");
        Serial.println(responseCode);

        response = http.getString();
        http.end();
        
        http_response["message"] = response;
        http_response["code"] = responseCode;

        return http_response;
    }

    JsonDocument getData(String &endpoint)
    {
        HTTPClient http;
        String response;

        JsonDocument http_response;

        Serial.println("[utils.h] getData endpoint: ");
        Serial.println(endpoint);

        http.begin(endpoint);
        int responseCode = http.GET();
        Serial.println("[utils.h] getData response code: ");
        Serial.println(responseCode);

        if (responseCode > 0)
        {
            response = http.getString();
        }
        else
        {
            response = "[utils.h] getData error: " + String(responseCode);
        }
        
        http.end();

        http_response["message"] = response;
        http_response["code"] = responseCode;

        return http_response;
    }
         
}

#endif // UTILS_H