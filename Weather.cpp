// Include Inkplate library to the sketch
#include "Weather.h"
#include "CJKRenderer.h"
// Including fonts used
#include "Fonts/FreeSerifBold18pt7b.h"
#include "Fonts/FreeSerifBold12pt7b.h"

/* RTC Data --Cannot be Class variable */
RTC_DATA_ATTR char currentTime[32] = "";
RTC_DATA_ATTR char temperature_wind[40] = "";
RTC_DATA_ATTR char humidity_cloud_uvi[40] = "";
/* RTC Data End ======================================== */

#define NUMBER_WEATHER_ABBRS 11
// wind direction
const char wind_direction[8][7] = {
    "\xe5\x8c\x97",                    // 北
    "\xe4\xb8\x9c\xe5\x8c\x97",        // 东北
    "\xe4\xb8\x9c",                    // 东
    "\xe4\xb8\x9c\xe5\x8d\x97",        // 东南
    "\xe5\x8d\x97",                    // 南
    "\xe8\xa5\xbf\xe5\x8d\x97",        // 西南
    "\xe8\xa5\xbf",                    // 西
    "\xe8\xa5\xbf\xe5\x8c\x97"         // 西北
};
// Contants used for drawing icons defined in https://openweathermap.org/weather-conditions
const char abbrs[NUMBER_WEATHER_ABBRS][WEATHER_ABBR_SIZE] = {"13", "50", "11", "09", "10", "09", "03", "04", "02", "01"};
const uint8_t *logos[NUMBER_WEATHER_ABBRS] = {icon_sn, icon_sl, icon_h, icon_t, icon_hr, icon_lr, icon_s, icon_hc, icon_hc, icon_lc, icon_c};
const uint8_t *s_logos[NUMBER_WEATHER_ABBRS] = {icon_s_sn, icon_s_sl, icon_s_h,  icon_s_t,  icon_s_hr,
                            icon_s_lr, icon_s_s,  icon_s_hc, icon_s_hc, icon_s_lc, icon_s_c};
// Function for drawing weather info
void Weather::drawHourly()
{
    Serial.println(F("weather drawHourly"));
    // Draw a line plot for hourly temperature
    // x=8, y=228, w=18*48, h=200
    int left = 8;
    const int top = 244;
    const int bottom = 400;
    const int space = 12;
    // find min and max temerature
    float minTemperature = 200, maxTemperature = -100;
    for (int i = 0; i < NUMBER_HOURLY; i++) {
        minTemperature = minTemperature < weatherReport.hourly[i].temperature.day ? minTemperature : weatherReport.hourly[i].temperature.day;
        maxTemperature = maxTemperature > weatherReport.hourly[i].temperature.day ? maxTemperature : weatherReport.hourly[i].temperature.day;
    }
    // print y axis
    display.setFont(&FreeSerifBold12pt7b);
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);
    display.setCursor(left, top);
    sprintf(weatherFormat, "%.2fC", maxTemperature);
    display.println(weatherFormat);
    display.setCursor(left, bottom);
    sprintf(weatherFormat, "%.2fC", minTemperature);
    display.println(weatherFormat);

    const float step = (float)(bottom - top) / (maxTemperature - minTemperature);
    Serial.print(F("max temperature "));
    Serial.print(maxTemperature, DEC);
    Serial.print(F(" min temperature "));
    Serial.print(minTemperature, DEC);
    Serial.print(F(" step "));
    Serial.println(step, DEC);
    display.setTextColor(BLACK, WHITE);
    // shift right for axis
    left += 100;
    display.drawRect(left, top - space, 576, bottom - top + 2 * space, BLACK);
    for (int i = 0; i < NUMBER_HOURLY; i++) {
        display.setCursor(left + space * i, top + (int) ((maxTemperature - weatherReport.hourly[i].temperature.day) * step));
        display.println(F("+"));
        // Serial.println(weatherReport.hourly[i].temperature.day, DEC);
    }
}

// Function for drawing current time
void Weather::drawTime()
{
    Serial.println(F("weather drawTime"));
    // Drawing current time
    display.setTextColor(BLACK, WHITE);
    display.setFont(&FreeSerifBold18pt7b);
    display.setTextSize(2);

    display.setCursor(8, 72);
    CJKRenderer::drawString(display, 8, 72, currentTime, BLACK);
}

// Function for drawing city name
void Weather::drawQRCode()
{
    if (!display.sdCardInit()) {
      display.setCursor(900, 0);
      display.println(F("SD Card error!"));
      Serial.println(F("SD Card error!"));
      display.partialUpdate();
      return;
    }
    Serial.println(F("weather drawQRCode"));
    // display.drawImage("wifi.png", 900, 0);
    display.drawImage("slogan.png", 750, 50);
}

void Weather::drawDaily() {
    Serial.println(F("weather drawDaily"));
    display.setFont(&FreeSerifBold12pt7b);
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);

    const int x = 8;

    // Chinese labels: 降雨, 云量, 湿度, 紫外, 风速, 风向, 早晨, 白天, 傍晚, 夜晚
    CJKRenderer::drawString(display, x, 520, "\xe9\x99\x8d\xe9\x9b\xa8", BLACK);           // 降雨
    CJKRenderer::drawString(display, x, 550, "\xe4\xba\x91\xe9\x87\x8f", BLACK);           // 云量
    CJKRenderer::drawString(display, x, 580, "\xe6\xb9\xbf\xe5\xba\xa6", BLACK);           // 湿度
    CJKRenderer::drawString(display, x, 610, "\xe7\xb4\xab\xe5\xa4\x96", BLACK);           // 紫外
    CJKRenderer::drawString(display, x, 640, "\xe9\xa3\x8e\xe9\x80\x9f", BLACK);           // 风速
    CJKRenderer::drawString(display, x, 670, "\xe9\xa3\x8e\xe5\x90\x91", BLACK);           // 风向
    CJKRenderer::drawString(display, x, 700, "\xe6\x97\xa9\xe6\x99\xa8", BLACK);           // 早晨
    CJKRenderer::drawString(display, x, 730, "\xe7\x99\xbd\xe5\xa4\xa9", BLACK);           // 白天
    CJKRenderer::drawString(display, x, 760, "\xe5\x82\x8d\xe6\x99\x9a", BLACK);           // 傍晚
    CJKRenderer::drawString(display, x, 790, "\xe5\xa4\x9c\xe6\x99\x9a", BLACK);           // 夜晚

    for (int i = 0; i < NUMBER_DAILY; i++) {
        drawOneDay(weatherReport.daily[i], i);
    }
}

void Weather::drawOneDay(WeatherData &data, int index) {
    Serial.print(F("weather drawOneDay "));
    Serial.println(index, DEC);
    int x = 166 + index * 132;

    display.setCursor(x, 440);
    CJKRenderer::drawString(display, x, 440, data.day, BLACK);
    // Weather Logo
    for (int i = 0; i < NUMBER_WEATHER_ABBRS; ++i)
    {
        // If found draw specified icon
        if (strcmp(abbrs[i], data.icon) == 0)
            display.drawBitmap(x, 450, s_logos[i], 48, 48, BLACK);
    }
    display.setCursor(x, 520);
    sprintf(weatherFormat, "%d%%", data.rain);
    display.println(weatherFormat);
    display.setCursor(x, 550);
    sprintf(weatherFormat, "%d%%", data.clouds);
    display.println(weatherFormat);
    display.setCursor(x, 580);
    sprintf(weatherFormat, "%d%%", data.humidity);
    display.println(weatherFormat);
    display.setCursor(x, 610);
    sprintf(weatherFormat, "%.2f%%", data.uvi);
    display.println(weatherFormat);
    display.setCursor(x, 640);
    sprintf(weatherFormat, "%.2f", data.wind.speed);
    display.println(weatherFormat);
    display.setCursor(x, 670);
    CJKRenderer::drawString(display, x, 670, wind_direction[((data.wind.deg + 45) / 45) % 8], BLACK);
    display.setCursor(x, 700);
    sprintf(weatherFormat, "%.2fC", data.temperature.morning);
    display.println(weatherFormat);
    display.setCursor(x, 730);
    sprintf(weatherFormat, "%.2fC", data.temperature.day);
    display.println(weatherFormat);
    display.setCursor(x, 760);
    sprintf(weatherFormat, "%.2fC", data.temperature.evening);
    display.println(weatherFormat);
    display.setCursor(x, 790);
    sprintf(weatherFormat, "%.2fC", data.temperature.night);
    display.println(weatherFormat);
}

// Current weather drawing function
void Weather::drawCurrent() {
    Serial.println(F("weather drawCurrent"));
    // Weather Logo
    for (int i = 0; i < NUMBER_WEATHER_ABBRS; i++)
    {
        // If found draw specified icon
        if (strcmp(abbrs[i], weatherReport.current.icon) == 0) {
            Serial.println(i, DEC);
            display.drawBitmap(16, 82, logos[i], 152, 152, BLACK);
            break;
        }
    }

    // Temperature and wind
    display.setFont(&FreeSerifBold18pt7b);
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);
    // 温度=\xe6\xb8\xa9\xe5\xba\xa6  ℃=\xe2\x84\x83  风速=\xe9\xa3\x8e\xe9\x80\x9f
    sprintf(
        temperature_wind,
        "\xe6\xb8\xa9\xe5\xba\xa6 %.2f\xe2\x84\x83 \xe9\xa3\x8e\xe9\x80\x9f %.2f%s",
        weatherReport.current.temperature.day,
        weatherReport.current.wind.speed,
        wind_direction[((weatherReport.current.wind.deg + 45) / 45) % 8]
    );
    CJKRenderer::drawString(display, 176, 128, temperature_wind, BLACK);

    // other information
    display.setFont(&FreeSerifBold18pt7b);
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);
    // 湿度=\xe6\xb9\xbf\xe5\xba\xa6  云量=\xe4\xba\x91\xe9\x87\x8f  紫外=\xe7\xb4\xab\xe5\xa4\x96
    sprintf(
        humidity_cloud_uvi,
        "\xe6\xb9\xbf\xe5\xba\xa6 %d%% \xe4\xba\x91\xe9\x87\x8f %d%% \xe7\xb4\xab\xe5\xa4\x96 %.2f",
        weatherReport.current.humidity,
        weatherReport.current.clouds,
        weatherReport.current.uvi
    );
    CJKRenderer::drawString(display, 176, 182, humidity_cloud_uvi, BLACK);
}

void Weather::draw()
{
    // use 1 bit for weather
    display.setDisplayMode(INKPLATE_1BIT);
    Serial.println(F("weather full refresh"));
    // Calling our begin from weatherNetwork.h file
    weatherNetwork.begin();

    // Get all relevant data, see WeatherNetwork.cpp for info
    weatherNetwork.getData(weatherReport, currentTime);

    // Draw data, see functions below for info
    drawTime();
    drawQRCode();
    drawCurrent();
    drawHourly();
    drawDaily();

    display.display();
}
