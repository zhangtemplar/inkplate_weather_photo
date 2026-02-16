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
#include "WiFiUtil.h"

// To get timeZone from main file
extern int SECRET_TIMEZONE;

// from settings.h
extern char SECRET_CITY[128];

// open weather api key
extern char WEATHER_API_KEY[128];

// Forward declaration for captive portal trigger
extern void startCaptivePortal();

// Declared week days
const char weekDays[8][8] = {
    "Mon", "Tue", "Wed", "Thr", "Fri", "Sat", "Sun",
};

void WeatherNetwork::begin()
{
    if (!connectWiFi(20)) {
        Serial.println(F("WiFi failed, starting captive portal"));
        startCaptivePortal();
        return;
    }
    // Find internet time
    setTime();
}

// Gets time from ntp server
void WeatherNetwork::getTime(char *timeStr, int timezone_offset)
{
    // Get seconds since 1.1.1970.
    time_t nowSecs = time(nullptr) + timezone_offset;

    // Used to store time
    struct tm timeinfo;
    gmtime_r(&nowSecs, &timeinfo);

    // Copies time string into timeStr: Www Mmm dd hh:mm
    strncpy(timeStr, asctime(&timeinfo), 16);
    Serial.println(timeStr);
}

void WeatherNetwork::setTime()
{
    Serial.println(F("setTime"));
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

void WeatherNetwork::parseWeather(JsonObject data, WeatherData &weather, bool has_pop, bool is_daily, int timezone_offset)
{
    // Serial.println(F("parseWeather"));
    // time
    int hours = (data[F("dt")].as<int>() + timezone_offset) / 3600;
    weather.hour = hours % 24;
    strncpy(weather.day, weekDays[(hours / 24 + 3) % 7], 3);

    // main
    // Serial.print(F("parseWeather/main: "));
    strncpy(weather.icon, data[F("weather")][0][F("icon")].as<char *>(), 2);
    // Serial.println(weather.icon);
    // Serial.println(F("parseWeather/clouds"));
    weather.clouds = data[F("clouds")].as<char>();
    // Serial.println(F("parseWeather/humidity"));
    weather.humidity = data[F("humidity")].as<char>();
    // Serial.println(F("parseWeather/uvi"));
    weather.uvi = data[F("uvi")].as<float>();
    if (has_pop) {
        // Serial.println(F("parseWeather/pop"));
        weather.rain = (int) (100 * data[F("pop")].as<float>());
    }
    // wind
    // Serial.println(F("parseWeather/wind"));
    weather.wind.speed = data[F("wind_speed")].as<int>();
    weather.wind.deg = data[F("wind_deg")].as<float>();
    // temperature
    // Serial.println(F("parseWeather/temperature"));
    if (is_daily) {
        weather.temperature.morning = data[F("temp")][F("morn")].as<float>();
        weather.temperature.day = data[F("temp")][F("day")].as<float>();
        weather.temperature.evening = data[F("temp")][F("eve")].as<float>();
        weather.temperature.night = data[F("temp")][F("night")].as<float>();
    } else {
        weather.temperature.day = data[F("temp")].as<float>();
    }
}

void WeatherNetwork::getData(WeatherReport &weather, char *timeStr)
{
    Serial.println(F("getData"));
    // If not connected to wifi reconnect wifi
    if (WiFi.status() != WL_CONNECTED)
    {
        if (!connectWiFi(20)) {
            Serial.println(F("Can't reconnect to WiFi"));
            return;
        }
    }

    // Wake up if sleeping and save inital state
    bool sleep = WiFi.getSleep();
    WiFi.setSleep(false);
    Serial.println(F("getData/Wifi is ready"));

    // Http object used to make get request
    HTTPClient http;

    http.getStream().setNoDelay(true);
    http.getStream().setTimeout(1);

    // Add woeid to api call
    char url[256];
    sprintf(url, "https://api.openweathermap.org/data/3.0/onecall?%s&appid=%s&exclude=minutely&units=metric", SECRET_CITY, WEATHER_API_KEY);

    // Initiate http
    http.begin(url);

    // Actually do request
    int httpCode = http.GET();
    if (httpCode == 200)
    {
        int32_t len = http.getSize();

        if (len > 0)
        {
            // Build a filter to parse only the fields we actually use.
            // This dramatically reduces memory: the full response is ~22KB
            // but the filtered parse only needs ~8KB of document memory.
            StaticJsonDocument<512> filter;
            filter["timezone_offset"] = true;

            // Current weather fields
            JsonObject curF = filter["current"].createNestedObject();
            curF["dt"] = true;
            curF["weather"][0]["icon"] = true;
            curF["clouds"] = true;
            curF["humidity"] = true;
            curF["uvi"] = true;
            curF["wind_speed"] = true;
            curF["wind_deg"] = true;
            curF["temp"] = true;

            // Hourly forecast fields
            JsonObject hourF = filter["hourly"][0].createNestedObject();
            hourF["dt"] = true;
            hourF["weather"][0]["icon"] = true;
            hourF["clouds"] = true;
            hourF["humidity"] = true;
            hourF["uvi"] = true;
            hourF["pop"] = true;
            hourF["wind_speed"] = true;
            hourF["wind_deg"] = true;
            hourF["temp"] = true;

            // Daily forecast fields
            JsonObject dayF = filter["daily"][0].createNestedObject();
            dayF["dt"] = true;
            dayF["weather"][0]["icon"] = true;
            dayF["clouds"] = true;
            dayF["humidity"] = true;
            dayF["uvi"] = true;
            dayF["pop"] = true;
            dayF["wind_speed"] = true;
            dayF["wind_deg"] = true;
            dayF["temp"]["morn"] = true;
            dayF["temp"]["day"] = true;
            dayF["temp"]["eve"] = true;
            dayF["temp"]["night"] = true;

            // 16KB is sufficient for the filtered response (down from 64KB)
            DynamicJsonDocument doc(16384);
            DeserializationError error = deserializeJson(
                doc, http.getStream(),
                DeserializationOption::Filter(filter)
            );

            // If an error happens print it to Serial monitor
            if (error)
            {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
            }
            else
            {
                const int timezone = doc[F("timezone_offset")].as<int>();
                Serial.println(F("parseWeather/getTime"));
                getTime(timeStr, timezone);
                Serial.println(F("parseWeather current"));
                parseWeather(doc[F("current")], weather.current, false, false, timezone);
                Serial.println(F("parseWeather hourly"));
                for (int i = 0; i < NUMBER_HOURLY; i++) {
                    Serial.println(i, DEC);
                    parseWeather(doc[F("hourly")][i], weather.hourly[i], true, false, timezone);
                }
                Serial.println(F(""));
                Serial.println(F("parseWeather daily"));
                for (int i = 0; i < NUMBER_DAILY; i++) {
                    Serial.println(i, DEC);
                    parseWeather(doc[F("daily")][i], weather.daily[i], true, true, timezone);
                }
                Serial.println(F("parseWeather done"));
            }
            doc.clear();
        }
    }

    // Clear document and end http
    http.end();

    // Return to initial state
    WiFi.setSleep(sleep);
}
