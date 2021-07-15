// Include Inkplate library to the sketch
#include "Weather.h"
// Including fonts used
#include "Fonts/Roboto_Light_120.h"
#include "Fonts/Roboto_Light_36.h"
#include "Fonts/Roboto_Light_48.h"

/* RTC Data --Cannot be Class variable */
RTC_DATA_ATTR char abbr1[16];
RTC_DATA_ATTR char abbr2[16];
RTC_DATA_ATTR char abbr3[16];
RTC_DATA_ATTR char abbr4[16];

// Variables for storing temperature
RTC_DATA_ATTR char temps[8][4] = {
    "0F",
    "0F",
    "0F",
    "0F",
};

// Variables for storing days of the week
RTC_DATA_ATTR char days[8][4] = {
    "",
    "",
    "",
    "",
};

// Variable for counting partial refreshes
RTC_DATA_ATTR unsigned refreshes = 0;

// Variables for storing current time and weather info
RTC_DATA_ATTR char currentTemp[16] = "0F";
RTC_DATA_ATTR char currentWind[16] = "0m/s";

RTC_DATA_ATTR char currentTime[16] = "9:41";

RTC_DATA_ATTR char currentWeather[32] = "-";
RTC_DATA_ATTR char currentWeatherAbbr[8] = "th";
/* RTC Data End ======================================== */

// Function for drawing weather info
void Weather::drawWeather(Inkplate display)
{
    // Searching for weather state abbreviation
    for (int i = 0; i < 10; ++i)
    {
        // If found draw specified icon
        if (strcmp(abbrs[i], currentWeatherAbbr) == 0)
            display.drawBitmap(50, 50, logos[i], 152, 152, BLACK);
    }

    // Draw weather state
    display.setTextColor(BLACK, WHITE);
    display.setFont(&Roboto_Light_36);
    display.setTextSize(1);
    display.setCursor(40, 270);
    display.println(currentWeather);
}

// Function for drawing current time
void Weather::drawTime(Inkplate display)
{
    Serial.println("weather drawTime");
    // Drawing current time
    display.setTextColor(BLACK, WHITE);
    display.setFont(&Roboto_Light_36);
    display.setTextSize(1);

    display.setCursor(1024 - 20 * strlen(currentTime), 35);
    display.println(currentTime);
}

// Function for drawing city name
void Weather::drawCity(Inkplate display)
{
    // Drawing city name
    display.setTextColor(BLACK, WHITE);
    display.setFont(&Roboto_Light_36);
    display.setTextSize(1);

    display.setCursor(600 - 9 * strlen(city), 790);
    display.println(city);
}

// Function for drawing temperatures
void Weather::drawTemps(Inkplate display)
{
    // Drawing 4 black rectangles in which temperatures will be written
    int rectWidth = 150;
    int rectSpacing = (1200 - rectWidth * 4) / 5;

    display.fillRect(1 * rectSpacing + 0 * rectWidth, 450, rectWidth, 302, BLACK);
    display.fillRect(2 * rectSpacing + 1 * rectWidth, 450, rectWidth, 302, BLACK);
    display.fillRect(3 * rectSpacing + 2 * rectWidth, 450, rectWidth, 302, BLACK);
    display.fillRect(4 * rectSpacing + 3 * rectWidth, 450, rectWidth, 302, BLACK);

    int textMargin = 6;

    display.setFont(&Roboto_Light_48);
    display.setTextSize(1);
    display.setTextColor(WHITE, BLACK);

    display.setCursor(1 * rectSpacing + 0 * rectWidth + textMargin, 450 + textMargin + 40);
    display.println(days[0]);

    display.setCursor(2 * rectSpacing + 1 * rectWidth + textMargin, 450 + textMargin + 40);
    display.println(days[1]);

    display.setCursor(3 * rectSpacing + 2 * rectWidth + textMargin, 450 + textMargin + 40);
    display.println(days[2]);

    display.setCursor(4 * rectSpacing + 3 * rectWidth + textMargin, 450 + textMargin + 40);
    display.println(days[3]);

    // Drawing temperature values into black rectangles
    display.setFont(&Roboto_Light_48);
    display.setTextSize(1);
    display.setTextColor(WHITE, BLACK);

    display.setCursor(1 * rectSpacing + 0 * rectWidth + textMargin, 450 + textMargin + 120);
    display.print(temps[0]);
    display.println(F("C"));

    display.setCursor(2 * rectSpacing + 1 * rectWidth + textMargin, 450 + textMargin + 120);
    display.print(temps[1]);
    display.println(F("C"));

    display.setCursor(3 * rectSpacing + 2 * rectWidth + textMargin, 450 + textMargin + 120);
    display.print(temps[2]);
    display.println(F("C"));

    display.setCursor(4 * rectSpacing + 3 * rectWidth + textMargin, 450 + textMargin + 120);
    display.print(temps[3]);
    display.println(F("C"));

    for (int i = 0; i < 18; ++i)
    {
        // If found draw specified icon
        if (strcmp(abbr1, abbrs[i]) == 0)
            display.drawBitmap(1 * rectSpacing + 0 * rectWidth + textMargin, 450 + textMargin + 150, s_logos[i], 48, 48,
                               WHITE, BLACK);
    }

    for (int i = 0; i < 18; ++i)
    {
        // If found draw specified icon
        if (strcmp(abbr2, abbrs[i]) == 0)
            display.drawBitmap(2 * rectSpacing + 1 * rectWidth + textMargin, 450 + textMargin + 150, s_logos[i], 48, 48,
                               WHITE, BLACK);
    }

    for (int i = 0; i < 18; ++i)
    {
        // If found draw specified icon
        if (strcmp(abbr3, abbrs[i]) == 0)
            display.drawBitmap(3 * rectSpacing + 2 * rectWidth + textMargin, 450 + textMargin + 150, s_logos[i], 48, 48,
                               WHITE, BLACK);
    }

    for (int i = 0; i < 18; ++i)
    {
        // If found draw specified icon
        if (strcmp(abbr4, abbrs[i]) == 0)
            display.drawBitmap(4 * rectSpacing + 3 * rectWidth + textMargin, 450 + textMargin + 150, s_logos[i], 48, 48,
                               WHITE, BLACK);
    }
}

// Current weather drawing function
void Weather::drawCurrent(Inkplate display)
{
    // Drawing current information

    // Temperature:
    display.setFont(&Roboto_Light_120);
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);

    display.setCursor(380, 206);
    display.print(currentTemp);

    int x = display.getCursorX();
    int y = display.getCursorY();

    display.setFont(&Roboto_Light_48);
    display.setTextSize(1);

    display.setCursor(x, y);
    display.println(F("C"));

    // Wind:
    display.setFont(&Roboto_Light_120);
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);

    display.setCursor(720, 206);
    display.print(currentWind);

    x = display.getCursorX();
    y = display.getCursorY();

    display.setFont(&Roboto_Light_48);
    display.setTextSize(1);

    display.setCursor(x, y);
    display.println(F("m/s"));

    // Labels underneath
    display.setFont(&Roboto_Light_36);
    display.setTextSize(1);

    display.setCursor(322, 288);
    display.println(F("TEMPERATURE"));

    display.setCursor(750, 288);
    display.println(F("WIND SPEED"));
}

void Weather::draw(Inkplate display)
{
    if (refreshes % fullRefresh == 0)
    {
        Serial.println("weather full refresh");
        // Calling our begin from weatherNetwork.h file
        weatherNetwork.begin(SECRET_CITY);

        // If city not found, do nothing
        if (weatherNetwork.location == -1)
        {
            display.setCursor(50, 290);
            display.setTextSize(3);
            display.print(F("City not in Metaweather Database"));
            display.display();
            while (1)
                ;
        }

        // Get all relevant data, see WeatherNetwork.cpp for info
        weatherNetwork.getTime(currentTime);
        weatherNetwork.getTime(currentTime);
        weatherNetwork.getDays(days[0], days[1], days[2], days[3]);
        weatherNetwork.getData(city, temps[0], temps[1], temps[2], temps[3], currentTemp, currentWind, currentTime,
                        currentWeather, currentWeatherAbbr, abbr1, abbr2, abbr3, abbr4);

        // Draw data, see functions below for info
        drawTime(display);
        drawWeather(display);
        drawCurrent(display);
        drawTemps(display);
        drawCity(display);

        display.display();
    }
    else
    {
        Serial.println("weather partial refresh");
        // Refresh only the clock
        weatherNetwork.getTime(currentTime);

        drawTime(display);
        drawWeather(display);
        drawCurrent(display);
        drawTemps(display);
        drawCity(display);

        display.display();
    }

    ++refreshes;
}