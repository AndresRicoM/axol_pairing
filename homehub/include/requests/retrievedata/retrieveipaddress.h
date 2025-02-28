#ifndef RETRIEVEIPADDRESS_H
#define RETRIEVEIPADDRESS_H

#include "../utils/utils.h"

String retrievePublicIPAddress()
{
    String endpoint = "https://api.ipify.org/";
    JsonDocument response = utils::getData(endpoint);
    String publicIP = response["message"];
    return publicIP;
}

#endif // RETRIEVEIPADDRESS_H