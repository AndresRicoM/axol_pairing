#ifndef RETRIEVELOCATION_H
#define RETRIEVELOCATION_H

#include <ArduinoJson.h>
#include "../utils/utils.h"
#include "retrieveipaddress.h"

JsonDocument retrieveLocation()
{
  JsonDocument doc1;
  JsonDocument json;

  doc1["homeMobileCountryCode"] = "334";
  doc1["homeMobileNetworkCode"] = "020";
  doc1["radioType"] = "lte";
  doc1["carrier"] = "Telcel";
  doc1["considerIp"] = "true";

  String requestBody;
  serializeJson(doc1, requestBody);
  String publicIP = retrievePublicIPAddress();
  String endpoint = "http://ip-api.com/json/" + publicIP + "?fields=status,message,lat,lon,query";

  String response = utils::postData(endpoint, requestBody);
  Serial.println("response");
  Serial.println(response);

  deserializeJson(json, response);

  return json;
}

#endif // RETRIEVELOCATION