#ifndef CALENDAR_H
#define CALENDAR_H

// Include Inkplate library to the sketch
#include "Inkplate.h"

#include "CalendarNetwork.h"

// Struct for storing calender event info
struct entry
{
    char name[128];
    char time[128];
    char location[128];
    int day = -1;
    int timeStamp;
};

// cannot be class variable or function argument, otherwise stack overflow
extern Inkplate display;
extern int SECRET_TIMEZONE;
// compare two event
int cmp(const void *a, const void *b);

class Calendar {
private:
    // Our networking functions, see Network.cpp for info
    CalendarNetwork network;
    const int refreshesToGet = 10;

    // Variables for time and raw event info
    char calendarDate[64];
    char *calendarData;

    // Here we store calendar calendarEntries
    int entriesNum = 0;
    entry calendarEntries[128];

    // draw title
    void drawInfo();
    // draw current time
    void drawTime();
    // draw the grid of calendar
    void drawGrid();
    // format the time information of event to string
    void getToFrom(char *dst, char *from, char *to, int *day, int *timeStamp);
    // draw a single event to calendar
    bool drawEvent(entry *event, int day, int beginY, int maxHeigth, int *heigthNeeded);
    // draw the whole calendar
    void drawData();
public:
    // main function
    void draw();
};
#endif