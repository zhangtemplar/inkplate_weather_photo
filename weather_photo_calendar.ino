/*
 * Weather + photo frame for InkPlate 10 and 6.
 * 
 * It is based on https://github.com/e-radionicacom/Inkplate-Arduino-library/tree/master/examples/Inkplate10/Projects
 * 
 * Author: zhangtemplar@gmail.com
 */
// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE10
#error "Wrong board selection for this example, please select Inkplate 10 in the boards menu."
#endif

// ----------------------------------
/*
 * Please create your own settings.h file with the following variables
 * // time zone as int
 * int SECRET_TIMEZONE = -7;
 * // City name to de displayed on the bottom
 * char SECRET_CITY[128] = "latitude,longitude";
 * // Change to your wifi ssid and password
 * char *SECRET_SSID = "";
 * char *SECRET_PASS = "";
 * // url to fetch images, for example apiflash.com
 * char *SECRET_PHOTO_URL = ""
 */
#include "settings.h"

// Include Inkplate library to the sketch
#include "Inkplate.h"

// For Weather
#include "Weather.h"

// for local photo
#include "LocalPhoto.h"

// Flickr web photo
#include "Flickr.h"

// Delay between API calls
// wait for 4 hours before next photo update
#define PHOTO_DELAY_US 4 * 60 * 60 * 1000 * 1000
// wait for 1 hour before next photo update
#define CALENDAR_DELAY_US 60 * 60 * 1000 * 1000
// for weather, delay 1 minute before each call
#define WEATHER_DELAY_US 60 * 1000 * 1000

// Inkplate object
// 3 Bit will fail weather display
Inkplate display(INKPLATE_1BIT);
#define MIN_VOLTAGE 3.4
/*
 * which page to show
 * 0: weather
 * 1: photo
 * 2: calendar
 * others are not supported
 * 
 * TODO: move to a dedicated class
 */
#define PAGE_WEATHER 0
#define PAGE_PHOTO 1
#define PAGE_CALENDAR 2
RTC_DATA_ATTR char page = PAGE_PHOTO;
RTC_DATA_ATTR char previousPage = -1;

Weather weather;
LocalPhoto localPhoto;
// Flickr flickr;
/*
 * Refresh display when needed.
 * 
 * The display will be cleared if any of the conditions met:
 * - page is switched
 * - forceClear is requested
 */
void refreshDisplay(bool forceClear);
// Read the latest touch pad event (via interrupt register)
void readTouchPad();

void imageUrl(char *a) {
    String url;
    HTTPClient http;
    if (http.begin("https://source.unsplash.com/random/1200x800") && http.GET() > 0)
    {
        url = http.getString();

        int urlStart = url.indexOf("href=\"") + 6;
        int urlEnd = url.indexOf("\">", urlStart);

        url = url.substring(urlStart, urlEnd);
        url = url.substring(0, url.indexOf("?")) + "?crop=entropy&fit=crop&fm=png&h=800&w=1200";

        Serial.println(url);
        strcpy(a, url.c_str());
    }
    else
    {
        display.println("HTTP error");
        display.display();
    }
    http.end();
}

void webPhoto() {
    display.setDisplayMode(INKPLATE_3BIT);
  // Join wifi
    display.joinAP(SECRET_SSID, SECRET_PASS);

    char url[256];
    imageUrl(url);
    Serial.print(F("to display image from "));
    Serial.println(url);

    Serial.println(display.drawImage(url, display.PNG, 0, 0, true));
    display.display();
}

void readTouchPad() {
    // According to the schema, touch pad are connected to port B 2, 3 and 4 accordingly
    // and from https://github.com/e-radionicacom/Inkplate-Arduino-library/blob/451f49eb752d37d49c9beebefa1eb2817d541c86/src/include/Mcp.cpp
    // we know this is matched to bit 10, 11 and 12 accordingly
    uint16_t key = display.getINTstateInternal(MCP23017_INT_ADDR, display.mcpRegsInt);
    if (key & (1 << 10))
    { // Check if first pad has been touched. If it is, decrement the number and refresh the screen.
        page = PAGE_WEATHER;
        Serial.println(F("key pressed for weather"));
    }

    if (key & (1 << 11))
    { // If you touched second touchpad, set number to zero and refresh screen by calling our displayNumber() function
        page = PAGE_PHOTO;
        Serial.println(F("key pressed for photo"));
    }

    if (key & (1 << 12))
    { // If you touched third touchpad, incerement the number and refresh the screen.
        page = PAGE_CALENDAR;
        Serial.println(F("key pressed for calendar"));
    }
}

void refreshDisplay(bool forceClear)
{
    bool shallClear = forceClear;
    if (previousPage != page) {
      shallClear = true;
      previousPage = page;
    }
    // Initial cleaning of buffer and physical screen
    if (shallClear) {
      display.clearDisplay();
      display.display();
    }
}

/**
 * @brief Check battery, if low show a warning
 * 
 * @return true if battery is ok
 * @return false if battery is low
 */
bool checkBattery() {
  float voltage = display.readBattery();
  if (voltage < MIN_VOLTAGE) {
    display.println("Battery low");
    display.print(voltage, 2);
    display.println("V");
    display.display();
    return false;
  }
  Serial.print(voltage, 2);
  Serial.println("V");
  return true;
}

// Main function
void setup()
{
    // common set up
    Serial.begin(115200);
    display.begin();

    // check battery
    if (!checkBattery()) {
      return;
    }

    // Setup mcp interrupts
    for (int touchPadPin = 10; touchPadPin <=12; touchPadPin++) {
      display.pinModeInternal(MCP23017_INT_ADDR, display.mcpRegsInt, touchPadPin, INPUT);
      display.setIntOutputInternal(MCP23017_INT_ADDR, display.mcpRegsInt, 1, false, false, HIGH);
      display.setIntPinInternal(MCP23017_INT_ADDR, display.mcpRegsInt, touchPadPin, RISING);
    }
    readTouchPad();

    refreshDisplay(false);
    switch (page) {
      case PAGE_WEATHER:
        weather.draw();
        break;
      case PAGE_PHOTO:
        // flickr.draw();
        webPhoto();
        break;
      default:
        localPhoto.draw();
    }

    // Go to sleep
    Serial.println(F("Going to sleep"));
    esp_sleep_enable_timer_wakeup(PHOTO_DELAY_US);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_34, 1);
    (void)esp_deep_sleep_start();
}

void loop()
{
}
