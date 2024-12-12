#ifndef POSTDATA_H
#define POSTDATA_H

#include <HTTPClient.h>

namespace utils
{
    String postData(String& endpoint, String& requestBody)
    {
        HTTPClient http;
        String response;

        http.begin(endpoint);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        int responseCode = http.POST(requestBody);

        Serial.println("response code");
        Serial.println(responseCode);

        response = http.getString();

        return response;
    }
}

#endif // POSTDATA_H