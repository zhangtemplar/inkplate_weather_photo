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

// Please create your own settings.h file
#include "settings.h"

// Include Inkplate library to the sketch
#include "Inkplate.h"

// for reading the configuration from file
#include <ArduinoJson.h>
#include "SdFat.h"

// For Weather
#include "Weather.h"

// for local photo
#include "LocalPhoto.h"

// Picsum web photo
#include "PicsumPhoto.h"

// Calendar
#include "Calendar.h"

// Captive portal for WiFi setup and image upload
#include "CaptivePortal.h"

// Delay between API calls
// wait for 4 hours before next photo update
#define PHOTO_DELAY_US 4ll * 60 * 60 * 1000 * 1000

// Inkplate object
// 3 Bit will fail weather display
Inkplate display(INKPLATE_1BIT);
#define MIN_VOLTAGE 3.4

/*
 * which page to show
 * 0: weather
 * 1: photo (Picsum or local)
 * 2: calendar
 * 3: setup (captive portal)
 */
#define PAGE_WEATHER 0
#define PAGE_PHOTO 1
#define PAGE_CALENDAR 2
#define PAGE_SETUP 3
RTC_DATA_ATTR char page = PAGE_WEATHER;
RTC_DATA_ATTR char previousPage = -1;
// Toggles between web (Picsum) and local SD card photo on each pad 2 press
RTC_DATA_ATTR bool useWebPhoto = true;

// Calendarific API key (for Chinese holidays)
char CALENDARIFIC_KEY[128] = "";

Weather weather;
LocalPhoto localPhoto;
PicsumPhoto picsumPhoto;
Calendar calendar;
CaptivePortal captivePortal;

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
// Start captive portal (called from WeatherNetwork on WiFi failure)
void startCaptivePortal();

void readTouchPad() {
    // According to the schema, touch pad are connected to port B 2, 3 and 4 accordingly
    // and from https://github.com/e-radionicacom/Inkplate-Arduino-library/blob/451f49eb752d37d49c9beebefa1eb2817d541c86/src/include/Mcp.cpp
    // we know this is matched to bit 10, 11 and 12 accordingly
    uint16_t key = display.getINTstateInternal(MCP23017_INT_ADDR, display.mcpRegsInt);

    bool pad1 = key & (1 << 10);
    bool pad2 = key & (1 << 11);
    bool pad3 = key & (1 << 12);

    // Touchpad 1+3 combo triggers setup portal
    if (pad1 && pad3) {
        page = PAGE_SETUP;
        Serial.println(F("key combo for setup"));
        return;
    }

    if (pad1)
    { // First pad: weather
        page = PAGE_WEATHER;
        Serial.println(F("key pressed for weather"));
    }

    if (pad2)
    { // Second pad: photo (toggles between web and local)
        page = PAGE_PHOTO;
        useWebPhoto = !useWebPhoto;
        Serial.print(F("key pressed for photo, web="));
        Serial.println(useWebPhoto);
    }

    if (pad3)
    { // Third pad: calendar
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
    display.setTextSize(8);
    display.println("Battery low ");
    display.print(voltage, 2);
    display.println(" V");
    display.display();
    return false;
  }
  Serial.print(voltage, 2);
  Serial.println("V");
  return true;
}

/**
 * @brief Read settings from a json file in local microsd card named as settings.json
 *
 * @return true if setting is read succesfully
 * @return false if setting failed to read, then hard coded setting will be used instead.
 */
bool readSettings() {
  if (!display.sdCardInit()) {
    display.println(F("SD Card error!"));
    Serial.println(F("SD Card error!"));
    display.partialUpdate();
    return false;
  }
  SdFile file2;
  Serial.println(F("open settings.json"));
  if (!file2.open("settings.json", O_RDONLY)) {
    Serial.println(F("failed to open settings.json"));
    return false;
  }
  Serial.println(F("parse settings.json"));
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file2);
  file2.close();
  if (error) {
    Serial.println(F("failed to read setttings from settings.json will use default value"));
    return false;
  }
  SECRET_TIMEZONE = doc["timezone"] | SECRET_TIMEZONE;
  strlcpy(SECRET_CITY, doc["city"] | SECRET_CITY, sizeof(SECRET_CITY));
  strlcpy(SECRET_SSID, doc["ssid"] | SECRET_SSID, sizeof(SECRET_SSID));
  strlcpy(SECRET_PASS, doc["wifi_password"] | SECRET_PASS, sizeof(SECRET_PASS));
  strlcpy(WEATHER_API_KEY, doc["weather_api_key"] | WEATHER_API_KEY, sizeof(WEATHER_API_KEY));
  strlcpy(CALENDARIFIC_KEY, doc["calendarific_key"] | CALENDARIFIC_KEY, sizeof(CALENDARIFIC_KEY));
  return true;
}

void startCaptivePortal() {
    page = PAGE_SETUP;
    captivePortal.start();
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

    readSettings();

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
        if (useWebPhoto) {
            picsumPhoto.draw();
        } else {
            localPhoto.draw();
        }
        break;
      case PAGE_CALENDAR:
        calendar.draw();
        break;
      case PAGE_SETUP:
        captivePortal.start();
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
