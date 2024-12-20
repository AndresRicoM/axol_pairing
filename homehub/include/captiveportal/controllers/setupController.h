#ifndef SETUP_CONTROLLER_H
#define SETUP_CONTROLLER_H

#include <WiFiManager.h>
#include <EEPROM.h>
#include "globals/globals.h"
#include "../pages/setupPage.h"


void handleSetupRoute()
{
  Serial.println("---CHECKING HOMEHUB FLAG---");
  Serial.println("EPROM FLAG: ");
  if (EEPROM.read(0) != 1) {
    Serial.println(EEPROM.read(0));
    wm.server->send(200, "text/html", setupPage);
  } else {
    Serial.println(EEPROM.read(0));
    wm.server->send(200, "text/html", setupPageNoHomehub);
  }

}
#endif // SETUP_CONTROLLER_H