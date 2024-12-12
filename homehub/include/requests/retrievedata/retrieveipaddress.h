#ifndef RETRIEVEIPADDRESS_H
#define RETRIEVEIPADDRESS_H

#include <HTTPClient.h>

String retrievePublicIPAddress()
{
    String endpoint = "https://api.ipify.org/";
    HTTPClient http;
    String response;

    http.begin(endpoint);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int responseCode = http.GET();

    Serial.println("response code");
    Serial.println(responseCode);

    response = http.getString();

    return response;
}

#endif // RETRIEVEIPADDRESS_H