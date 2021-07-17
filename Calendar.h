#ifndef CALENDAR_H
#define CALENDAR_H

// Include Inkplate library to the sketch
#include "Inkplate.h"

#include "CalendarNetwork.h"

// Struct for storing calender event info
#define MAX_STRING_LENGTH 128
struct entry
{
    char name[MAX_STRING_LENGTH];
    char time[MAX_STRING_LENGTH];
    char location[MAX_STRING_LENGTH];
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