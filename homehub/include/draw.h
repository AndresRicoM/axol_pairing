#ifndef DRAW_H
#define DRAW_H

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#include "cs_logo_bmp.h"
#include "happy_axol_bmp.h"
#include "sad_axol_bmp.h"

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
    void draw_clockdash(String, String, const char *, float, float, float, const char *, const unsigned char[64], const unsigned char[120], const unsigned char[100], const unsigned char[120], const unsigned char[120]);
    void draw_waterdash(float, int, int);
    void draw_system(int, int, int, int);
    void draw_receiveddata();
};

/**
 * @brief Draws the CS logo on the display.
 */
void Draw::drawCS()
{
    this->display.clearDisplay();
    this->display.drawBitmap(
        (this->display.width() - LOGO_WIDTH) / 2,
        (this->display.height() - LOGO_HEIGHT) / 2,
        cs_logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
    this->display.display();
    delay(1000);
}

/**
 * @brief Draws a happy or sad axol.
 * @param fill_percentage a float data type updated from get_system_stats in main.c.
 */
void Draw::draw_axol(float fill_percentage)
{ // Draws Axol

    this->display.clearDisplay();
    if (fill_percentage >= .5)
    {
        this->display.drawBitmap(
            (this->display.width() - 108) / 2,
            (this->display.height() - 60) / 2,
            happy_axol_bmp, 108, 60, 1);
        this->display.display();
    }
    else
    {
        this->display.drawBitmap(
            (this->display.width() - 125) / 2,
            (this->display.height() - 60) / 2,
            sad_axol_bmp, 125, 60, 1);
        this->display.display();
    }

    delay(5000);
    this->draw_maindash();
}

void Draw::draw_clockdash(String timeStamp, String dayStamp, const char *city_name, float main_temp, float main_temp_max, float main_temp_min, const char *weather_0_icon, const unsigned char cloud_bmp[64], const unsigned char sun_bmp[120], const unsigned char rain_bmp[100], const unsigned char storm_bmp[120], const unsigned char snow_bmp[120])
{

    int16_t x1;
    int16_t y1;
    uint16_t width;
    uint16_t height;

    this->display.clearDisplay();
    this->display.setTextSize(2.5);
    this->display.setTextColor(WHITE);
    // Gents Center for Date
    this->display.getTextBounds(timeStamp, 0, 0, &x1, &y1, &width, &height);
    this->display.setCursor(0, 5);
    this->display.println(timeStamp);

    this->display.setTextSize(.9);
    this->display.getTextBounds(dayStamp, 0, 0, &x1, &y1, &width, &height);
    this->display.setCursor(0, 25);
    this->display.println(dayStamp);

    this->display.setTextSize(0);
    this->display.getTextBounds(city_name, 0, 0, &x1, &y1, &width, &height);
    this->display.setCursor(0, 55);
    this->display.println(city_name);

    int d_temp = round(main_temp - 272.15);
    int d_max = round(main_temp_max - 272.15);
    int d_min = round(main_temp_min - 272.15);

    this->display.setTextSize(2);
    String display_temp = String(d_temp) + "C";
    this->display.getTextBounds(display_temp, 0, 0, &x1, &y1, &width, &height);
    this->display.setCursor(SCREEN_WIDTH - width, 5);
    this->display.println(display_temp);

    this->display.setTextSize(.9);
    String display_range = String(d_min) + "C / " + String(d_max) + "C";
    this->display.getTextBounds(display_range, 0, 0, &x1, &y1, &width, &height);
    this->display.setCursor(SCREEN_WIDTH - width, 25);
    this->display.println(display_range);

    if ((String(weather_0_icon) == "04d") || (String(weather_0_icon) == "04n") || (String(weather_0_icon) == "03n") || (String(weather_0_icon) == "03d"))
    { // Clouds
        this->draw_weather_icon(cloud_bmp, 30, 16);
    }
    else if ((String(weather_0_icon) == "01d") || (String(weather_0_icon) == "01n") || (String(weather_0_icon) == "02n") || (String(weather_0_icon) == "02d"))
    { // Sun
        this->draw_weather_icon(sun_bmp, 30, 30);
    }
    else if ((String(weather_0_icon) == "09d") || (String(weather_0_icon) == "09n") || (String(weather_0_icon) == "10d") || (String(weather_0_icon) == "10n"))
    { // Rain
        this->draw_weather_icon(rain_bmp, 30, 25);
    }
    else if ((String(weather_0_icon) == "11d") || (String(weather_0_icon) == "11n"))
    { // Storm
        this->draw_weather_icon(storm_bmp, 30, 30);
    }
    else if ((String(weather_0_icon) == "13d") || (String(weather_0_icon) == "13n"))
    { // Snow
        this->draw_weather_icon(snow_bmp, 30, 30);
    }
    else
    { // Could be mist or other conditions.
    }

    this->display.display();
    delay(3500);

    this->draw_maindash();
}

void Draw::draw_waterdash(float fill_percentage, int avail_liters, int avail_storage)
{ // Function Draws Dashboard with loction, weather and time information.

    int16_t x1;
    int16_t y1;
    uint16_t width;
    uint16_t height;

    // Serial.println(remaining_average);
    this->display.clearDisplay();
    this->display.setTextSize(2);
    this->display.setTextColor(WHITE);
    this->display.setCursor(0, 0);
    this->display.print("Reservas: ");
    this->display.print(fill_percentage * 100);
    this->display.println(" %");
    this->display.setTextSize(1);
    this->display.println("");
    this->display.print(avail_liters);
    this->display.print("/");
    this->display.print(avail_storage);
    this->display.println(" L ");
    this->display.println("Displonibles");

    // water quality dashboard!!!!!!!!!!!!!!!!!

    /*if (remaining_average >= water_goal) {
      draw_water_icon(happy_drop_bmp, WWIDTH, WHEIGHT);
    }
    if (remaining_average <= water_goal) {
      draw_water_icon(sad_drop_bmp, WWIDTH, WHEIGHT);
    } */
    this->display.display();

    delay(5000);
    this->draw_maindash();
}

void Draw::draw_weather_icon(const unsigned char icon[], int width, int height)
{
    this->display.drawBitmap(
        (this->display.width() - width),
        (this->display.height() - height),
        icon, width, height, 1);
}

void Draw::draw_system(int buckets, int tanks, int quality, int envs)
{
    int16_t x1;
    int16_t y1;
    uint16_t width;
    uint16_t height;
    // Serial.println(remaining_average);
    this->display.clearDisplay();
    this->display.setTextSize(2);
    this->display.setTextColor(WHITE);
    this->display.setCursor(0, 0);
    this->display.println("Sensores");
    this->display.println("");
    this->display.setTextSize(1.5);
    this->display.print("Cubetas: ");
    this->display.println(buckets);
    this->display.print("Tanques: ");
    this->display.println(tanks);
    this->display.print("Calidad: ");
    this->display.println(quality);
    this->display.print("Ambiente: ");
    this->display.println(envs);

    this->display.display();

    delay(5000);
    this->draw_maindash();
}

void Draw::draw_receiveddata()
{

    this->display.clearDisplay();
    for (int16_t i = max(this->display.width(), this->display.height()) / 2; i > 0; i -= 3)
    {
        // The INVERSE color is used so circles alternate white/black
        this->display.fillCircle(this->display.width() / 2, this->display.height() / 2, i, SSD1306_INVERSE);
        this->display.display(); // Update screen with each newly-drawn circle
        delay(1);
    }

    this->draw_maindash();
}

/**
 * @brief Clears the contents of the current screen
 */
void Draw::draw_maindash()
{
    this->display.clearDisplay();
    this->display.display();
}

#endif // DRAW_H