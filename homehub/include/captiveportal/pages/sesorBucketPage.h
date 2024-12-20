#ifndef SENSORBUCKETPAGE_H
#define SENSORBUCKETPAGE_H

#include <wm_consts_en.h>
#include <wm_strings_en.h>
#include <wm_strings_es.h>
#include <WString.h> //Library that works for strings

String sensorBucketPage = HTTP_HEAD_START + String(HTTP_STYLE) + "<style>"

                + "input{" + "   border: 1px #C1BDBD solid;" + "   line-height: 2em;" + "}" + ".textbox{" + "   display: flex;" + "   flex-direction: column;" + "   align-items: flex-start;" + "   gap: 0.5rem;" + "}" + "form{" + "   gap: 1.5rem;" + "}"

                + "</style>" + "</head>" + "<body>" + "<h1>Bucket Sensor Registration</h1>" + "<form action='/api/sensor/bucket' method='post'>"

                + "<div class='textbox'>" + "   <span>Bucket Capacity</span>" + "   <input type='text' name='b_capacity' placeholder='0.00' />" + "</div>"

                + "<div class='textbox'>" + "   <span>Use</span>" + "   <input type='text' name='b_use' placeholder='kitchen, cleaning, bathroom...' />" + "</div>"

                + "<button type='submit'>Register</button>" + "</form><br>" 

                + "<form action='/exit' method='get'><button type='submit'>Exit</button></form>"
                
                + HTTP_END;

#endif // SENSORBUCKETPAGE_H