#ifndef RETRIEVEIPADDRESS_H
#define RETRIEVEIPADDRESS_H

#include "../utils/utils.h"

String retrievePublicIPAddress()
{
    String endpoint = "https://api.ipify.org/";
    return utils::getData(endpoint);
}

#endif // RETRIEVEIPADDRESS_H