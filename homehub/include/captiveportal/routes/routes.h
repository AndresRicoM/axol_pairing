#ifndef CAPTIVEPORTAL_ROUTES_H
#define CAPTIVEPORTAL_ROUTES_H

#include "../controllers/setupController.h"
#include "../controllers/registerController.h"
#include "globals/globals.h"

void bindServerCallback()
{
    wm.server->on("/", handleSetupRoute);
    wm.server->on("/register", handleRegister);
    wm.server->on("/api/register", handleRegisterRequest);
}

#endif // CAPTIVEPORTAL_ROUTES_H