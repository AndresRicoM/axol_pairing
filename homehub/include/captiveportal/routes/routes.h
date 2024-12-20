#ifndef CAPTIVEPORTAL_ROUTES_H
#define CAPTIVEPORTAL_ROUTES_H

#include "../controllers/setupController.h"
#include "../controllers/registerController.h"
#include "../controllers/sensorsController.h"
#include "globals/globals.h"

void bindServerCallback()
{
    wm.server->on("/", handleSetupRoute);
    wm.server->on("/register", handleRegister);
    wm.server->on("/api/register", handleRegisterRequest);
    wm.server->on("/sensors", handleSensors);
     wm.server->on("/api/sensor/bucket", handleSensorsRequest);
}

#endif // CAPTIVEPORTAL_ROUTES_H