#ifndef CALENDAR_H
#define CALENDAR_H

#include <ArduinoJson.h>
#include "SolarTerms.h"

#define MAX_HOLIDAYS 40
#define MAX_CUSTOM_DATES 20

struct HolidayEntry {
    int month;
    int day;
    char name[40];
};

class Calendar {
public:
    void draw();
private:
    HolidayEntry holidays[MAX_HOLIDAYS];
    int holidayCount;
    HolidayEntry customDates[MAX_CUSTOM_DATES];
    int customDateCount;

    void syncTime();
    void fetchUSHolidays(int year);
    void fetchChineseHolidays(int year);
    void loadCustomDates();
    void loadSolarTerms(int year, int month);
    void drawGrid(int year, int month, int daysInMonth, int startDow);
    bool isHoliday(int month, int day, char *nameBuf, int bufSize);
    int daysInMonth(int year, int month);
    int dayOfWeek(int year, int month, int day);
};

#endif
