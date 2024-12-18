#ifndef REGISTERPAGE_H
#define REGISTERPAGE_H

#include <wm_consts_en.h>
#include <wm_strings_en.h>
#include <wm_strings_es.h>

String registerPage = HTTP_HEAD_START + String(HTTP_STYLE) + "<style>" + "input{" + "   border: 1px #C1BDBD solid;" + "   line-height: 2em;" + "}" + ".textbox{" + "   display: flex;" + "   flex-direction: column;" + "   align-items: flex-start;" + "   gap: 0.5rem;" + "}" + "form{" + "   gap: 1.5rem;" + "}"

                      + "</style>" + "</head>" + "<body>" + "<h1>Register HomeHub</h1>" + "<form action='/api/register' method='post'>"

                      + "<div class='textbox'>" + "   <span>Username</span>" + "   <input type='text' name='username' />" + "</div>"

                      + "<div class='textbox'>" + "   <span>HomeHub Name</span>" + "   <input type='text' name='homehub_name' />" + "</div>"

                      + "<button type='submit'>Register</button>" + "</form>" + HTTP_END;
#endif // REGISTERPAGE_H