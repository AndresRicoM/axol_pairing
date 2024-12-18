#ifndef UTILS_H
#define UTILS_H

#include <HTTPClient.h>

namespace utils
{
    String postData(String &endpoint, String &requestBody)
    {
        HTTPClient http;
        String response;

        http.begin(endpoint);
        http.addHeader("Content-Type", "application/json");

        Serial.println("Entrando a postdata.h...");
        Serial.println("requestBody: ");
        Serial.println(requestBody);
        Serial.println("endpoint: ");
        Serial.println(endpoint);
        int responseCode = http.POST(requestBody);

        Serial.println("response code");
        Serial.println(responseCode);

        response = http.getString();
        http.end();

        return response;
    }

    String getData(String &endpoint)
    {
        HTTPClient http;
        String response;

        http.begin(endpoint);
        int responseCode = http.GET();

        if (responseCode > 0)
        {
            response = http.getString();
        }
        else
        {
            response = "Error: " + String(responseCode);
        }
        
        http.end();

        return response;
    }
}

#endif // UTILS_H