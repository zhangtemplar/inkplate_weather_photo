// Include Inkplate library to the sketch
#include "Weather.h"
// Including fonts used
#include "Fonts/FreeSerifBold18pt7b.h"
#include "Fonts/FreeSerifBold12pt7b.h"

/* RTC Data --Cannot be Class variable */
RTC_DATA_ATTR char currentTime[17] = "Www Mmm dd hh:mm";
RTC_DATA_ATTR char temperature_wind[27] = "Temp -xx.xxC Wind yyy.yyNW";
RTC_DATA_ATTR char humidity_cloud_uvi[31] = "Humid xxx% Cloud yyy% UVI zzz%";
/* RTC Data End ======================================== */

#define NUMBER_WEATHER_ABBRS 11
// wind direction
const char wind_direction[8][3] = {"N", "NE", "E", "SE", "S", "SW", "W", "WN"};
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
    const int top = 228;
    const int bottom = 433;
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

    const float step = 200.0f / (maxTemperature - minTemperature);
    Serial.print(F("max temperature "));
    Serial.print(maxTemperature, DEC);
    Serial.print(F(" min temperature "));
    Serial.print(minTemperature, DEC);
    Serial.print(F(" step "));
    Serial.println(step, DEC);
    display.setTextColor(BLACK, WHITE);
    // shift right for axis
    left += 100;
    display.drawRect(left, top - space, 576, 205, BLACK);
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
    display.println(currentTime);
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

    display.setCursor(x, 550);
    display.println(F("Cloud"));
    display.setCursor(x, 580);
    display.println(F("Humidity"));
    display.setCursor(x, 610);
    display.println(F("UVI"));
    display.setCursor(x, 640);
    display.println(F("Wind"));
    display.setCursor(x, 670);
    display.println(F("Direction"));
    display.setCursor(x, 700);
    display.println(F("Morning"));
    display.setCursor(x, 730);
    display.println(F("Day"));
    display.setCursor(x, 760);
    display.println(F("Evening"));
    display.setCursor(x, 790);
    display.println(F("Night"));

    for (int i = 0; i < NUMBER_DAILY; i++) {
        drawOneDay(weatherReport.daily[i], i);
    }
}

void Weather::drawOneDay(WeatherData &data, int index) {
    Serial.print(F("weather drawOneDay "));
    Serial.println(index, DEC);
    int x = 166 + index * 132;

    display.setCursor(x, 440);
    display.println(data.day);
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
    display.println(wind_direction[((data.wind.deg + 45) / 45) % 8]);
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
            display.drawBitmap(16, 82, logos[i], 100, 100, BLACK);
            break;
        }
    }

    // Temperature and wind
    display.setFont(&FreeSerifBold18pt7b);
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);
    display.setCursor(120, 128);
    sprintf(
        temperature_wind, 
        "Temp %.2fC Wind %.2f%s",
        weatherReport.current.temperature.day,
        weatherReport.current.wind.speed,
        wind_direction[((weatherReport.current.wind.deg + 45) / 45) % 8]
    );
    display.print(temperature_wind);

    // other information
    display.setFont(&FreeSerifBold18pt7b);
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);
    display.setCursor(120, 182);
    sprintf(
        humidity_cloud_uvi, 
        "Humid %d%% Cloud %d%% UVI %.2f%%",
        weatherReport.current.humidity,
        weatherReport.current.clouds,
        weatherReport.current.uvi
    );
    display.print(humidity_cloud_uvi);
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
