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
        
        http_response["message"] = response;
        http_response["code"] = responseCode;

        return http_response;
    }

    String sensorData(String &endpoint, String &requestBody)
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

        if (responseCode == 201)
        {
            Serial.println("Success. TANK DATA has been registered succesfully.");
        }
        else
        {
            Serial.println("Request failed. Code: " + String(responseCode));
        }

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