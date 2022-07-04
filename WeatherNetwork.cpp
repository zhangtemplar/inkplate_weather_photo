/*
WeatherNetwork.cpp
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

// WeatherNetwork.cpp contains various functions and classes that enable Weather station
// They have been declared in seperate file to increase readability
#include "WeatherNetwork.h"

#include <HTTPClient.h>
#include <WiFi.h>

#include <ArduinoJson.h>

// To get timeZone from main file
extern int SECRET_TIMEZONE;

// from settings.h
extern char SECRET_CITY[128];
// wifi ssid and password
extern char SECRET_SSID[128];
extern char SECRET_PASS[128];

// open weather api key
extern char WEATHER_API_KEY[128];

// Static Json from ArduinoJson library
// the response is around 16k bytes
StaticJsonDocument<20000> doc;

// Declared week days
char weekDays[8][8] = {
    "Mon", "Tue", "Wed", "Thr", "Fri", "Sat", "Sun",
};

void WeatherNetwork::begin()
{
    // Initiating wifi, like in BasicHttpClient example
    WiFi.mode(WIFI_STA);
    WiFi.begin(SECRET_SSID, SECRET_PASS);

    int cnt = 0;
    Serial.print(F("Waiting for WiFi to connect..."));
    while ((WiFi.status() != WL_CONNECTED))
    {
        Serial.print(F("."));
        delay(1000);
        ++cnt;

        if (cnt == 20)
        {
            Serial.println(F("Can't connect to WIFI, restarting"));
            delay(100);
            ESP.restart();
        }
    }
    Serial.println(F(" connected"));

    // Find internet time
    setTime();
}

// Gets time from ntp server
void WeatherNetwork::getTime(char *timeStr)
{
    // Get seconds since 1.1.1970.
    time_t nowSecs = time(nullptr);

    // Used to store time
    struct tm timeinfo;
    gmtime_r(&nowSecs, &timeinfo);

    // Copies time string into timeStr
    strncpy(timeStr, asctime(&timeinfo) + 11, 5);

    // Setting time string timezone
    int hr = 10 * timeStr[0] + timeStr[1] + SECRET_TIMEZONE;

    // Better defined modulo, in case timezone makes hours to go below 0
    hr = (hr % 24 + 24) % 24;

    // Adding time to '0' char makes it into whatever time char, for both digits
    timeStr[0] = hr / 10 + '0';
    timeStr[1] = hr % 10 + '0';
}

void formatTemp(char *str, float temp)
{
    // Built in function for float to char* conversion
    dtostrf(temp, 2, 0, str);
}

void formatWind(char *str, float wind)
{
    // Built in function for float to char* conversion
    dtostrf(wind, 2, 0, str);
}

void WeatherNetwork::getData(char *city, char *temp1, char *temp2, char *temp3, char *temp4, char *currentTemp,
                      char *currentWind, char *currentTime, char *currentWeather, char *currentWeatherAbbr, char *abbr1,
                      char *abbr2, char *abbr3, char *abbr4)
{
    // If not connected to wifi reconnect wifi
    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.reconnect();

        delay(5000);

        int cnt = 0;
        Serial.println(F("Waiting for WiFi to reconnect..."));
        while ((WiFi.status() != WL_CONNECTED))
        {
            // Prints a dot every second that wifi isn't connected
            Serial.print(F("."));
            delay(1000);
            ++cnt;

            if (cnt == 7)
            {
                Serial.println(F("Can't connect to WIFI, restart initiated."));
                delay(100);
                ESP.restart();
            }
        }
    }

    // Wake up if sleeping and save inital state
    bool sleep = WiFi.getSleep();
    WiFi.setSleep(false);

    // Http object used to make get request
    HTTPClient http;

    http.getStream().setNoDelay(true);
    http.getStream().setTimeout(1);

    // Add woeid to api call
    char url[256];
    sprintf(url, "https://api.openweathermap.org/data/2.5/forecast?%s&appid=%s&units=metric", SECRET_CITY, WEATHER_API_KEY);

    // Initiate http
    http.begin(url);

    // Actually do request
    int httpCode = http.GET();
    if (httpCode == 200)
    {
        int32_t len = http.getSize();

        if (len > 0)
        {
            // Try parsing JSON object
            DeserializationError error = deserializeJson(doc, http.getStream());

            // If an error happens print it to Serial monitor
            if (error)
            {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
            }
            else
            {
                strcpy(city, doc[F("city")][F("name")].as<char *>());
                Serial.print(F("found weather for city: "));
                Serial.println(city);
                // some rise and sun set
                // strcpy(sunrise, doc["city"]["sunrise"].as<char *>());
                // strcpy(sunset, doc["city"]["sunset"].as<char *>());

                // Set all data got from internet using formatTemp and formatWind defined above
                // This part relies heavily on ArduinoJson library
                formatTemp(currentTemp, doc[F("list")][0][F("main")][F("temp")].as<float>());
                formatWind(currentWind, doc[F("list")][0][F("wind")][F("speed")].as<float>());
                // Humidity
                // formatWind(humidity, doc[F("list")][0][F("main")][F("humidity")].as<float>());
                // Weather, use icon for short
                strcpy(currentWeather, doc[F("list")][0][F("weather")][0][F("main")].as<char *>());
                strncpy(currentWeatherAbbr, doc[F("list")][0][F("weather")][0][F("icon")].as<char *>(), WEATHER_ABBR_SIZE - 1);

                // Forecast, the API returns 5 day's weather in unit of 3 hours
                strncpy(abbr1, doc[F("list")][8][F("weather")][0][F("icon")].as<char *>(), WEATHER_ABBR_SIZE - 1);
                strncpy(abbr2, doc[F("list")][16][F("weather")][0][F("icon")].as<char *>(), WEATHER_ABBR_SIZE - 1);
                strncpy(abbr2, doc[F("list")][24][F("weather")][0][F("icon")].as<char *>(), WEATHER_ABBR_SIZE - 1);
                strncpy(abbr2, doc[F("list")][32][F("weather")][0][F("icon")].as<char *>(), WEATHER_ABBR_SIZE - 1);

                formatTemp(temp1, doc[F("list")][8][F("main")][F("temp")].as<float>());
                formatTemp(temp2, doc[F("list")][16][F("main")][F("temp")].as<float>());
                formatTemp(temp3, doc[F("list")][24][F("main")][F("temp")].as<float>());
                formatTemp(temp4, doc[F("list")][32][F("main")][F("temp")].as<float>());
            }
        }
    }

    // Clear document and end http
    doc.clear();
    http.end();

    // Return to initial state
    WiFi.setSleep(sleep);
}

void WeatherNetwork::setTime()
{
    // Used for setting correct time
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    Serial.print(F("Waiting for NTP time sync: "));
    time_t nowSecs = time(nullptr);
    while (nowSecs < 8 * 3600 * 2)
    {
        // Print a dot every half a second while time is not set
        delay(500);
        Serial.print(F("."));
        yield();
        nowSecs = time(nullptr);
    }

    Serial.println();

    // Used to store time info
    struct tm timeinfo;
    gmtime_r(&nowSecs, &timeinfo);

    Serial.print(F("Current time: "));
    Serial.print(asctime(&timeinfo));
}

void WeatherNetwork::getDays(char *day, char *day1, char *day2, char *day3)
{
    // Seconds since 1.1.1970.
    time_t nowSecs = time(nullptr);

    // Find weekday

    // We get seconds since 1970, add 3600 (1 hour) times the time zone and add 3 to
    // make monday the first day of the week, as 1.1.1970. was a thursday
    // finally do mod 7 to insure our day is within [0, 6]
    int dayWeek = ((long)((nowSecs + 3600L * SECRET_TIMEZONE) / 86400L) + 3) % 7;

    // Copy day data to globals in main file
    strncpy(day, weekDays[(dayWeek + 1) % 7], 3);
    strncpy(day1, weekDays[(dayWeek + 2) % 7], 3);
    strncpy(day2, weekDays[(dayWeek + 3) % 7], 3);
    strncpy(day3, weekDays[(dayWeek + 4) % 7], 3);
}
