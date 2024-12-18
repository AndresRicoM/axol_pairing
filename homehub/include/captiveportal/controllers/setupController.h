#ifndef SETUP_CONTROLLER_H
#define SETUP_CONTROLLER_H

#include <WiFiManager.h>
#include "globals/globals.h"
#include "../pages/setupPage.h"

void handleSetupRoute()
{
  wm.server->send(200, "text/html", setupPage);
}
#endif // SETUP_CONTROLLER_H