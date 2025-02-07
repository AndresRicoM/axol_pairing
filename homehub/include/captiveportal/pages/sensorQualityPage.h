#ifndef SENSORQUALITYPAGE_H
#define SENSORQUALITYPAGE_H

#include <wm_consts_en.h>
#include <wm_strings_en.h>
#include <wm_strings_es.h>
#include <WString.h> //Library that works for strings

String sensorQualityPage = HTTP_HEAD_START + String(HTTP_STYLE) + "<style>"

+ "input{" + "   border: 1px #C1BDBD solid;" + "   line-height: 2em;" + "}" + ".textbox{" + "   display: flex;" + "   flex-direction: column;" + "   align-items: flex-start;" + "   gap: 0.5rem;" + "}" + "form{" + "   gap: 1.5rem;" + "}"

+ "</style>" + "</head>" + "<body>" + "<h1>Quality Sensor Registration</h1>" + "<form action='/api/sensor/quality' method='post'>"

+ "<div class='textbox'>" + "   <span>Use</span>" + "   <input type='text' name='quality_use' placeholder='kitchen, cleaning, bathroom...' />" + "</div>"

+ "<button type='submit'>Register</button>" + "</form><br>" 

+ "<form action='/exit' method='get'><button type='submit'>Exit</button></form>"

+ HTTP_END;

#endif // SENSORTANKPAGE_H