#ifndef SETUPPAGE_H
#define SETUPPAGE_H

#include <wm_consts_en.h>
#include <wm_strings_en.h>
#include <wm_strings_es.h>

String setupPage = HTTP_HEAD_START + String(HTTP_STYLE) + "</head>" + "<body>" + "<h1>Axol HomeHub Configuration</h1>" + "<form action='/wifi' method='get'><button type='submit'>Configure WiFi</button></form><br/>" + "<form action='/register' method='get'><button type='submit'>Register</button></form><br/>" + "<form action='/info' method='get'><button type='submit'>Info</button></form><br/>" + "<form action='/exit' method='get'><button type='submit'>Exit</button></form><br/>" + HTTP_END;

#endif // SETUPPAGE_H