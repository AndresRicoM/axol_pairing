#ifndef DRAW_H
#define DRAW_H

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#include "cs_logo_bmp.h"
#include "happy_axol_bmp.h"
#include "sad_axol_bmp.h"
#include "cloud_bmp.h"
#include "snow_bmp.h"
#include "rain_bmp.h"
#include "sun_bmp.h"
#include "storm_bmp.h"

#define LOGO_HEIGHT 60 // CS logo Size in pixels
#define LOGO_WIDTH 60
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

/**
 * @class Draw
 * @brief A class to handle drawing operations on an Adafruit SSD1306 display.
 */
class Draw
{
private:
    Adafruit_SSD1306 &display;

public:
    /**
     * @brief Constructor for the Draw class.
     * @param _display An instance of Adafruit_SSD1306 display.
     */
    Draw(Adafruit_SSD1306 &_display) : display(_display) { }

    ~Draw()
    {
        // Destructor for the Draw class
        // Currently, no specific cleanup is required
    }

    /**
     * @brief Draw operations to Adafruit_SSD1306 display
     */
    void drawCS();
    void draw_axol(float);
    void draw_maindash();
    void draw_weather_icon(const unsigned char[], int, int);
    void draw_clockdash(String, String, const char *, float, float, float, const char *);
    void draw_waterdash(float, int, int);
    void draw_system(int, int, int, int);
    void draw_receiveddata();
};

#endif // DRAW_H