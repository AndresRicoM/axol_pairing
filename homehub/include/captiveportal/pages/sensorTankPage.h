#ifndef SENSORTANKPAGE_H
#define SENSORTANKPAGE_H

#include <wm_consts_en.h>
#include <wm_strings_en.h>
#include <wm_strings_es.h>
#include <WString.h> //Library that works for strings

String sensorTankPage = 
    HTTP_HEAD_START + 
    String(HTTP_STYLE) +
    "<style>"
        "input{border:1px #C1BDBD solid;line-height:2em;}"
        ".textbox{display:flex;flex-direction:column;align-items:flex-start;gap:0.5rem;}"
        "form{display:flex;flex-direction:column;gap:1.5rem;max-width:400px;}"
        "#rectangular_fields{display:flex;flex-direction:column;gap:0.5rem;}"
    "</style>"
    "<script>"
        "function updateTankFields(){"
            "const type=document.getElementById('tank_type').value;"
            "document.getElementById('cylindrical_fields').style.display=(type==='cylindrical')?'block':'none';"
            "document.getElementById('rectangular_fields').style.display=(type==='rectangular')?'block':'none';"
        "}"
    "</script>"
    "</head><body>"
    "<h1>Tank Sensor Registration</h1>"
    "<form action='/api/sensor/tank' method='post'>"
        "<div class='textbox'>"
            "<span>Tank Capacity (Liters)</span>"
            "<input type='text' name='tank_capacity' placeholder='0.00' />"
        "</div>"
        "<div class='textbox'>"
            "<span>Use</span>"
            "<input type='text' name='tank_use' placeholder='kitchen, cleaning, bathroom...' />"
        "</div>"
        "<div class='textbox'>"
            "<span>Tank Height (Meteres)</span>"
            "<input type='text' name='tank_height' placeholder='0.00' />"
        "</div>"
        "<div class='textbox'>"
            "<span>Tank Type</span>"
            "<select name='tank_type' id='tank_type' onchange='updateTankFields()'>"
                "<option value='cylindrical'>Cylindrical</option>"
                "<option value='rectangular'>Rectangular</option>"
            "</select>"
        "</div>"
        "<div id='cylindrical_fields' class='textbox'>"
            "<span>Diameter (Meteres)</span>"
            "<input type='text' name='diameter' placeholder='0.00' />"
        "</div>"
        "<div id='rectangular_fields' style='display:none;'>"
            "<div class='textbox'>"
                "<span>Width (Meteres)</span>"
                "<input type='text' name='width' placeholder='0.00' />"
            "</div>"
            "<div class='textbox'>"
                "<span>Depth (Meteres)</span>"
                "<input type='text' name='depth' placeholder='0.00' />"
            "</div>"
        "</div>"
        "<button type='submit'>Register</button>"
    "</form><br>"
    "<form action='/exit' method='get'><button type='submit'>Exit</button></form>"
    + HTTP_END;
#endif // SENSORTANKPAGE_H