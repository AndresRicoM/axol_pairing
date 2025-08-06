#ifndef RETRIEVELOCATION_H
#define RETRIEVELOCATION_H

#include <ArduinoJson.h>
#include "../utils/utils.h"
#include "retrieveipaddress.h"

JsonDocument retrieveLocation()
{
  JsonDocument doc1;
  JsonDocument response;

  doc1["homeMobileCountryCode"] = "334";
  doc1["homeMobileNetworkCode"] = "020";
  doc1["radioType"] = "lte";
  doc1["carrier"] = "Telcel";
  doc1["considerIp"] = "true";

  String requestBody;
  serializeJson(doc1, requestBody);
  String publicIP = retrievePublicIPAddress();
  String endpoint = "http://ip-api.com/json/" + publicIP + "?fields=status,message,lat,lon,query";

  response = utils::postData(endpoint, requestBody);
  String str_response = response["message"];
  
  Serial.println("[retrievelocation.h] response message:");
  Serial.println(str_response);

  // deserializeJson(json, response);

  return response;
}

#endif // RETRIEVELOCATION