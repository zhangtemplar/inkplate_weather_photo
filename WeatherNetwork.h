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

#ifndef WEATHER_NETWORK_H
#define WEATHER_NETWORK_H
// string length for weather abbreviation
#define WEATHER_ABBR_SIZE 3

// All functions defined in WeatherNetwork.cpp

class WeatherNetwork
{
  public:
    // Functions we can access in main file
    void begin();
    void getTime(char *timeStr);
    void getData(char *city, char *temp1, char *temp2, char *temp3, char *temp4, char *currentTemp, char *currentWind,
                 char *currentTime, char *currentWeather, char *currentWeatherAbbr, char *abbr1, char *abbr2,
                 char *abbr3, char *abbr4);
    void getDays(char *day, char *day1, char *day2, char *day3);

    // Used to store loaction woeid (world id), set in findCity()
    int location = -1;

  private:
    // Functions called from within our class
    void setTime();
};

#endif
