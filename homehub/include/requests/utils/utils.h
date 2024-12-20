#ifndef UTILS_H
#define UTILS_H
#define EEPROM_SIZE 1

#include <HTTPClient.h>
#include <EEPROM.h>

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
        EEPROM.begin(EEPROM_SIZE);  // Initialize EEPROM with the specified size
        // If the response code is 201 (created successfully), mark the ESP32 as registered in EEPROM
        if (responseCode == 201)
        {
            EEPROM.begin(EEPROM_SIZE);  // Initialize EEPROM with the defined size
            EEPROM.write(0, 1);         // Write 1 to position 0 to indicate that the device is registered
            EEPROM.commit();            // Save changes to EEPROM
            Serial.println("Success. Homehub has been marked as registered in EEPROM.");
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