#ifndef SENSOR_CONTROLLER_H
#define SENSOR_CONTROLLER_H

#include <WiFiManager.h>
#include "globals/globals.h"
#include "globals/management/management.h"
#include "../pages/sensorBucketPage.h"
#include "../pages/sensorTankPage.h"
#include "../pages/sensorQualityPage.h"
// #include "../../requests/sensors/bucket/bucket.h"

void handleSensorsPages()
{
  switch (myData.type)
  {
  case 1:
  {
    wm.server->send(200, "text/html", sensorBucketPage);
  }
  break;

  case 2:
  {
    wm.server->send(200, "text/html", sensorTankPage);
  }
  break;

  case 4:
  {
    wm.server->send(200, "text/html", sensorQualityPage);
  }
  break;

  default:
  {
    String page = HTTP_HEAD_START + String(HTTP_STYLE) + "<h1>SENSOR NO ENCONTRADO</h1>" + HTTP_END;
    wm.server->send(200, "text/html", page);
  }
  break;
  }
}
#endif // SENSORS_CONTROLLER_H