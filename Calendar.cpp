#include "Calendar.h"
#include "Inkplate.h"
#include "WiFiUtil.h"
#include "SdFat.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <time.h>

#include "Fonts/FreeSerifBold18pt7b.h"
#include "Fonts/FreeSerifBold12pt7b.h"
#include "Fonts/FreeSans9pt7b.h"

#include "CJKRenderer.h"
#include "SolarTerms.h"

extern Inkplate display;
extern int SECRET_TIMEZONE;
extern char CALENDARIFIC_KEY[128];

// Month names
static const char *monthNames[] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

// Chinese month names (UTF-8)
static const char *chineseMonthNames[] = {
    "\xe4\xb8\x80\xe6\x9c\x88",   // 一月
    "\xe4\xba\x8c\xe6\x9c\x88",   // 二月
    "\xe4\xb8\x89\xe6\x9c\x88",   // 三月
    "\xe5\x9b\x9b\xe6\x9c\x88",   // 四月
    "\xe4\xba\x94\xe6\x9c\x88",   // 五月
    "\xe5\x85\xad\xe6\x9c\x88",   // 六月
    "\xe4\xb8\x83\xe6\x9c\x88",   // 七月
    "\xe5\x85\xab\xe6\x9c\x88",   // 八月
    "\xe4\xb9\x9d\xe6\x9c\x88",   // 九月
    "\xe5\x8d\x81\xe6\x9c\x88",   // 十月
    "\xe5\x8d\x81\xe4\xb8\x80\xe6\x9c\x88", // 十一月
    "\xe5\x8d\x81\xe4\xba\x8c\xe6\x9c\x88"  // 十二月
};

void Calendar::syncTime() {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    Serial.print(F("Waiting for NTP time sync: "));
    time_t nowSecs = time(nullptr);
    int attempts = 0;
    while (nowSecs < 8 * 3600 * 2 && attempts < 30) {
        delay(500);
        Serial.print(F("."));
        yield();
        nowSecs = time(nullptr);
        attempts++;
    }
    Serial.println();
}

void Calendar::fetchUSHolidays(int year) {
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();
    char url[128];
    sprintf(url, "https://date.nager.at/api/v3/publicholidays/%d/US", year);
    http.begin(client, url);
    int httpCode = http.GET();
    if (httpCode == 200) {
        String response = http.getString();
        DynamicJsonDocument doc(8192);
        DeserializationError error = deserializeJson(doc, response);
        if (!error) {
            JsonArray arr = doc.as<JsonArray>();
            for (JsonObject obj : arr) {
                if (holidayCount >= MAX_HOLIDAYS) break;
                const char *dateStr = obj["date"];
                // dateStr format: "YYYY-MM-DD"
                int m = 0, d = 0;
                sscanf(dateStr + 5, "%d-%d", &m, &d);
                holidays[holidayCount].month = m;
                holidays[holidayCount].day = d;
                strlcpy(holidays[holidayCount].name, obj["localName"] | obj["name"], sizeof(holidays[0].name));
                holidayCount++;
            }
        } else {
            Serial.print(F("US holidays JSON error: "));
            Serial.println(error.c_str());
        }
    } else {
        Serial.print(F("US holidays HTTP error: "));
        Serial.println(httpCode);
    }
    http.end();
}

void Calendar::fetchChineseHolidays(int year) {
    if (strlen(CALENDARIFIC_KEY) == 0) {
        Serial.println(F("No Calendarific API key, skipping Chinese holidays"));
        return;
    }
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();
    char url[256];
    sprintf(url, "https://calendarific.com/api/v2/holidays?api_key=%s&country=CN&year=%d&language=zh", CALENDARIFIC_KEY, year);
    http.begin(client, url);
    int httpCode = http.GET();
    if (httpCode == 200) {
        String response = http.getString();
        DynamicJsonDocument doc(16384);
        DeserializationError error = deserializeJson(doc, response);
        if (!error) {
            JsonArray arr = doc["response"]["holidays"];
            for (JsonObject obj : arr) {
                if (holidayCount >= MAX_HOLIDAYS) break;
                JsonObject dateObj = obj["date"]["datetime"];
                holidays[holidayCount].month = dateObj["month"];
                holidays[holidayCount].day = dateObj["day"];
                strlcpy(holidays[holidayCount].name, obj["name"], sizeof(holidays[0].name));
                holidayCount++;
            }
        } else {
            Serial.print(F("CN holidays JSON error: "));
            Serial.println(error.c_str());
        }
    } else {
        Serial.print(F("CN holidays HTTP error: "));
        Serial.println(httpCode);
    }
    http.end();
}

void Calendar::loadCustomDates() {
    customDateCount = 0;
    if (!display.sdCardInit()) {
        Serial.println(F("SD Card error for dates.json"));
        return;
    }
    SdFile file;
    if (!file.open("dates.json", O_RDONLY)) {
        Serial.println(F("No dates.json found"));
        return;
    }
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error) {
        Serial.print(F("dates.json parse error: "));
        Serial.println(error.c_str());
        return;
    }
    JsonArray arr = doc["dates"];
    for (JsonObject obj : arr) {
        if (customDateCount >= MAX_CUSTOM_DATES) break;
        customDates[customDateCount].month = obj["month"];
        customDates[customDateCount].day = obj["day"];
        strlcpy(customDates[customDateCount].name, obj["name"], sizeof(customDates[0].name));
        customDateCount++;
    }
    Serial.print(F("Loaded custom dates: "));
    Serial.println(customDateCount);
}

void Calendar::loadSolarTerms(int year, int month) {
    SolarTermEntry terms[SOLAR_TERM_COUNT];
    int count = getSolarTerms(year, terms);
    for (int i = 0; i < count; i++) {
        if (terms[i].month == month && holidayCount < MAX_HOLIDAYS) {
            holidays[holidayCount].month = terms[i].month;
            holidays[holidayCount].day = terms[i].day;
            strlcpy(holidays[holidayCount].name,
                    getSolarTermName(terms[i].termIndex),
                    sizeof(holidays[0].name));
            holidayCount++;
        }
    }
}

bool Calendar::isHoliday(int month, int day, char *nameBuf, int bufSize) {
    nameBuf[0] = '\0';
    for (int i = 0; i < holidayCount; i++) {
        if (holidays[i].month == month && holidays[i].day == day) {
            strlcpy(nameBuf, holidays[i].name, bufSize);
            return true;
        }
    }
    for (int i = 0; i < customDateCount; i++) {
        if (customDates[i].month == month && customDates[i].day == day) {
            strlcpy(nameBuf, customDates[i].name, bufSize);
            return true;
        }
    }
    return false;
}

int Calendar::daysInMonth(int year, int month) {
    const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0))
        return 29;
    return days[month - 1];
}

// Zeller-like: returns 0=Sun, 1=Mon, ..., 6=Sat
int Calendar::dayOfWeek(int year, int month, int day) {
    if (month < 3) {
        month += 12;
        year--;
    }
    int k = year % 100;
    int j = year / 100;
    int h = (day + (13 * (month + 1)) / 5 + k + k / 4 + j / 4 - 2 * j) % 7;
    // Zeller gives 0=Sat, 1=Sun, ...
    int dow = ((h + 6) % 7); // convert to 0=Sun
    return dow;
}

void Calendar::drawGrid(int year, int month, int numDays, int startDow) {
    // Layout constants for 1200x825 display
    const int gridLeft = 40;
    const int gridTop = 120;
    const int cellW = 160;
    const int cellH = 90;
    const int headerH = 30;

    const char *dayHeaders[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    // Draw day headers
    display.setFont(&FreeSerifBold12pt7b);
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);
    for (int i = 0; i < 7; i++) {
        int x = gridLeft + i * cellW + cellW / 2 - 18;
        display.setCursor(x, gridTop);
        display.print(dayHeaders[i]);
    }

    // Draw grid lines
    for (int row = 0; row <= 6; row++) {
        display.drawFastHLine(gridLeft, gridTop + headerH + row * cellH, 7 * cellW, BLACK);
    }
    for (int col = 0; col <= 7; col++) {
        display.drawFastVLine(gridLeft + col * cellW, gridTop + headerH, 6 * cellH, BLACK);
    }

    // Fill in day numbers
    display.setFont(&FreeSerifBold18pt7b);
    display.setTextSize(1);
    char nameBuf[40];
    int eventListY = gridTop + headerH + 6 * cellH + 20;
    int eventsShown = 0;

    for (int day = 1; day <= numDays; day++) {
        int dow = (startDow + day - 1) % 7;
        int week = (startDow + day - 1) / 7;
        int cx = gridLeft + dow * cellW;
        int cy = gridTop + headerH + week * cellH;

        // Weekend shading (Sat=6, Sun=0) - draw diagonal lines pattern
        if (dow == 0 || dow == 6) {
            // In 1-bit mode, use a line pattern for shading
            for (int ly = cy + 2; ly < cy + cellH - 1; ly += 4) {
                display.drawFastHLine(cx + 1, ly, cellW - 2, BLACK);
            }
        }

        // Day number
        display.setTextColor(BLACK, WHITE);
        display.setCursor(cx + 8, cy + 32);
        display.print(day);

        // Solar term annotation inside cell
        int termIdx = findSolarTerm(year, month, day);
        if (termIdx >= 0) {
            const char *termName = getSolarTermName(termIdx);
            display.setFont(&FreeSans9pt7b);
            CJKRenderer::drawString(display, cx + 8, cy + 70, termName, BLACK);
            display.setFont(&FreeSerifBold18pt7b);
        }

        // Holiday marker
        if (isHoliday(month, day, nameBuf, sizeof(nameBuf))) {
            display.fillCircle(cx + cellW - 16, cy + 16, 6, BLACK);
            // Add to event list below the grid
            if (eventsShown < 8) {
                display.setFont(&FreeSerifBold12pt7b);
                int evX = gridLeft + (eventsShown % 2) * 560;
                int evY = eventListY + (eventsShown / 2) * 28;
                display.setCursor(evX, evY);
                char eventLine[16];
                sprintf(eventLine, "%d/%d: ", month, day);
                display.print(eventLine);

                // Draw holiday name (may be Chinese from Calendarific API)
                int16_t curX = display.getCursorX();
                int16_t curY = display.getCursorY();
                CJKRenderer::drawString(display, curX, curY, nameBuf, BLACK);

                display.setFont(&FreeSerifBold18pt7b);
                eventsShown++;
            }
        }
    }
}

void Calendar::draw() {
    display.setDisplayMode(INKPLATE_1BIT);

    holidayCount = 0;
    customDateCount = 0;

    // Connect WiFi and sync time
    if (!connectWiFi(20)) {
        Serial.println(F("WiFi failed for calendar"));
        display.setFont(&FreeSerifBold18pt7b);
        display.setTextSize(1);
        display.setCursor(100, 400);
        display.print(F("WiFi required for calendar"));
        display.display();
        return;
    }
    syncTime();

    // Get current date
    time_t now = time(nullptr) + SECRET_TIMEZONE;
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    int year = timeinfo.tm_year + 1900;
    int month = timeinfo.tm_mon + 1;

    Serial.print(F("Calendar for "));
    Serial.print(monthNames[month - 1]);
    Serial.print(F(" "));
    Serial.println(year);

    // Fetch holidays and solar terms
    fetchUSHolidays(year);
    fetchChineseHolidays(year);
    loadCustomDates();
    loadSolarTerms(year, month);

    // English title
    display.setFont(&FreeSerifBold18pt7b);
    display.setTextSize(2);
    display.setTextColor(BLACK, WHITE);
    char title[32];
    sprintf(title, "%s %d", monthNames[month - 1], year);
    // Center the title approximately
    int titleLen = strlen(title);
    int titleX = (1200 - titleLen * 28) / 2;
    if (titleX < 0) titleX = 40;
    display.setCursor(titleX, 60);
    display.print(title);

    // Chinese date subtitle (e.g. "2026年二月")
    display.setFont(&FreeSerifBold12pt7b);
    display.setTextSize(1);
    char cnTitle[32];
    sprintf(cnTitle, "%d\xe5\xb9\xb4", year);  // "2026年"
    int cnWidth = CJKRenderer::measureString(display, cnTitle);
    cnWidth += CJKRenderer::measureString(display, chineseMonthNames[month - 1]);
    int cnTitleX = (1200 - cnWidth) / 2;
    if (cnTitleX < 0) cnTitleX = 40;
    int endX = CJKRenderer::drawString(display, cnTitleX, 100, cnTitle, BLACK);
    CJKRenderer::drawString(display, endX, 100, chineseMonthNames[month - 1], BLACK);

    // Draw the calendar grid
    int numDays = daysInMonth(year, month);
    int startDow = dayOfWeek(year, month, 1);
    drawGrid(year, month, numDays, startDow);

    display.display();
}
