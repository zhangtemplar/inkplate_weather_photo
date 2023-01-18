/*
WeatherNetwork.h
Inkplate 6 Arduino library
David Zovko, Borna Biro, Denis Vajak, Zvonimir Haramustek @ e-radionica.com
September 24, 2020
https://github.com/e-radionicacom/Inkplate-6-Arduino-library

For support, please reach over forums: forum.e-radionica.com/en
For more info about the product, please check: www.inkplate.io

This code is released under the GNU Lesser General Public License v3.0: https://www.gnu.org/licenses/lgpl-3.0.en.html
Please review the LICENSE file included with this example.
If you have any questions about licensing, please contact techsupport@e-radionica.com
Distributed as-is; no warranty is given.
*/

#include "Arduino.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <ArduinoJson.h>

#ifndef WEATHER_NETWORK_H
#define WEATHER_NETWORK_H
// string length for weather abbreviation
#define WEATHER_ABBR_SIZE 3
#define NUMBER_HOURLY 48
#define NUMBER_DAILY 8

// All functions defined in WeatherNetwork.cpp

struct Temperature
{
  float morning;
  float evening;
  float day; // current temperature
  float night;
};

struct Wind
{
  float speed;
  int deg; // 0-360
};

struct WeatherData
{
  char day[4];
  char icon[WEATHER_ABBR_SIZE];
  char rain; // 0-100, rain probability
  char clouds; // 0-100
  char humidity; // 0-100
  float uvi;
  Wind wind;
  Temperature temperature;
  char hour;
};

struct WeatherReport
{
  WeatherData current;
  WeatherData hourly[NUMBER_HOURLY];
  WeatherData daily[NUMBER_DAILY];
};

class WeatherNetwork
{
  public:
    // Functions we can access in main file
    void begin();
    // weather
    void getData(WeatherReport &weather, char *timeStr);

  private:
    // Functions called from within our class
    void setTime();
    void getTime(char *timeStr, int timezone_offset);
    void parseWeather(JsonObject data, WeatherData &weather, bool has_pop, bool is_daily, int timezone_offset);
};

#endif
