/*
   3-Google_calendar_example for e-radionica.com Inkplate 10
   For this example you will need only USB cable and Inkplate 10.
   Select "Inkplate 10(ESP32)" from Tools -> Board menu.
   Don't have "Inkplate 10(ESP32)" option? Follow our tutorial and add it:
   https://e-radionica.com/en/blog/add-inkplate-6-to-arduino-ide/

   This project shows you how Inkplate 10 can be used to display
   events in your Google Calendar using their provided API

   For this to work you need to change your timezone, wifi credentials and your private calendar url
   which you can find following these steps:

    1. Open your google calendar
    2. Click the 3 menu dots of the calendar you want to access at the bottom of left hand side
    3. Click 'Settings and sharing'
    4. Navigate to 'Integrate Calendar'
    5. Take the 'Secret address in iCal format'

   (https://support.google.com/calendar/thread/2408874?hl=en)

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: http://forum.e-radionica.com/en/
   11 February 2021 by e-radionica.com
*/

// Include Inkplate library to the sketch
#include "Calendar.h"

// Including fonts
#include "Fonts/FreeSans12pt7b.h"
#include "Fonts/FreeSans9pt7b.h"

// Includes
#include <algorithm>
#include <ctime>

// Grid size
#define NUMBER_ROWS 1
#define NUMBER_COLUMNS 7
#define WIDTH 1200
#define HEIGHT 825

const int refreshesToGet = 10;

// Variables for time and raw event info
char calendarDate[64];
char *calendarData;

// Here we store calendar calendarEntries
int entriesNum = 0;
entry calendarEntries[128];

void Calendar::draw()
{
    calendarData = (char *)ps_malloc(2000000LL);
    display.setTextWrap(false);
    display.setTextColor(0, 7);

    delay(5000);
    network.begin();

    // Keep trying to get calendarData if it fails the first time
    while (!network.getData(calendarData))
    {
        Serial.println("Failed getting calendarData, retrying");
        delay(1000);
    }

    // Drawing all calendarData, functions for that are above
    drawInfo();
    drawGrid();
    drawData();
    drawTime();

    // Can't do partial due to deepsleep
    display.display();
    // free(calendarData);
}

// Function for drawing calendar info
void Calendar::drawInfo()
{
    // Setting font and color
    display.setTextColor(0, 7);
    display.setFont(&FreeSans12pt7b);
    display.setTextSize(1);

    display.setCursor(20, 20);

    // Find email in raw calendarData
    char temp[64];
    char *start = strstr(calendarData, "X-WR-CALNAME:");

    // If not found return
    if (!start)
        return;

    // Find where it ends
    start += 13;
    char *end = strchr(start, '\n');

    strncpy(temp, start, end - start - 1);
    temp[end - start - 1] = 0;

    // Print it
    display.println(temp);
}

// Drawing what time it is
void Calendar::drawTime()
{
    // Initial text settings
    display.setTextColor(0, 7);
    display.setFont(&FreeSans12pt7b);
    display.setTextSize(1);

    display.setCursor(500, 20);

    // Our function to get time
    network.getTime(calendarDate);

    int t = calendarDate[16];
    calendarDate[16] = 0;
    display.println(calendarDate);
    calendarDate[16] = t;
}

// Draw lines in which to put events
void Calendar::drawGrid()
{
    // upper left and low right coordinates
    int x1 = 3, y1 = 30;
    int y2 = HEIGHT - 30, x2 = WIDTH;

    // header size, for day info
    int header = 30;

    // Line drawing
    display.drawThickLine(x1, y1 + header, x2, y1 + header, 0, 2.0);
    for (int i = 0; i < NUMBER_ROWS + 1; ++i)
    {
        int y = (int)((float)y1 + (float)i * (float)(y2 - y1) / (float)NUMBER_ROWS);
        display.drawThickLine(x1, y, x2, y, 0, 2.0);
    }
    for (int i = 0; i < NUMBER_COLUMNS + 1; ++i)
    {
        int x = (int)((float)x1 + (float)i * (float)(x2 - x1) / (float)NUMBER_COLUMNS);
        display.drawThickLine(x, y1, x, y2, 0, 2.0);
        display.setFont(&FreeSans9pt7b);

        // Display day info using time offset
        char temp[64];
        network.getTime(temp, i * 3600L * 24);
        temp[10] = 0;

        // calculate where to put text and print it
        display.setCursor(40 + x, y1 + header - 6);
        display.println(temp);
    }
}

/*
 * Format event times, example 13:00 to 14:00
 * from and to is the input
 */
void Calendar::getToFrom(char *dst, char *from, char *to, int *day, int *timeStamp)
{
    // ANSI C time struct
    struct tm ltm = {0}, ltm2 = {0};
    char temp[128], temp2[128];
    strncpy(temp, from, 16);
    temp[16] = 0;

    // https://github.com/esp8266/Arduino/issues/5141, quickfix
    memmove(temp + 5, temp + 4, 16);
    memmove(temp + 8, temp + 7, 16);
    memmove(temp + 14, temp + 13, 16);
    memmove(temp + 16, temp + 15, 16);
    temp[4] = temp[7] = temp[13] = temp[16] = '-';

    // time.h function
    strptime(temp, "%Y-%m-%dT%H-%M-%SZ", &ltm);

    // create start and end event structs
    struct tm event, event2;
    time_t epoch = mktime(&ltm) + (time_t)SECRET_TIMEZONE * 3600L;
    gmtime_r(&epoch, &event);
    strncpy(dst, asctime(&event) + 11, 5);

    dst[5] = '-';

    strncpy(temp2, to, 16);
    temp2[16] = 0;

    // Same as above

    // https://github.com/esp8266/Arduino/issues/5141, quickfix
    memmove(temp2 + 5, temp2 + 4, 16);
    memmove(temp2 + 8, temp2 + 7, 16);
    memmove(temp2 + 14, temp2 + 13, 16);
    memmove(temp2 + 16, temp2 + 15, 16);
    temp2[4] = temp2[7] = temp2[13] = temp2[16] = '-';

    strptime(temp2, "%Y-%m-%dT%H-%M-%SZ", &ltm2);

    time_t epoch2 = mktime(&ltm2) + (time_t)SECRET_TIMEZONE * 3600L;
    gmtime_r(&epoch2, &event2);
    strncpy(dst + 6, asctime(&event2) + 11, 5);
    // from: epoch/event and to: epoch2/event2

    dst[11] = 0;

    char dayName[64];
    char dayEvent[74];
    strcpy(dayEvent, asctime(&event));
    Serial.println(dayEvent);
    *timeStamp = epoch;
    // Getting the time from our function in Network.cpp
    *day = -1;
    network.getTime(temp);
    for (size_t i = 0; i < NUMBER_COLUMNS; i++) {
        network.getTime(dayName, i * 24 * 3600);
        Serial.println(dayName);
        if (strncmp(dayName, dayEvent, 10) == 0) {
            *day = i;
            break;
        }
    }
}

// Function to draw event
bool Calendar::drawEvent(entry *event, int day, int beginY, int maxHeigth, int *heigthNeeded)
{
    // Upper left coordintes
    int x1 = 3 + 4 + (WIDTH / NUMBER_COLUMNS) * day;
    int y1 = beginY + 3;

    // Setting text font
    display.setFont(&FreeSans12pt7b);

    // Some temporary variables
    int n = 0;
    char line[128];

    // Insert line brakes into setTextColor
    int lastSpace = -100;
    display.setCursor(x1 + 5, beginY + 26);
    for (int i = 0; i < min((size_t)64, strlen(event->name)); ++i)
    {
        // Copy name letter by letter and check if it overflows space given
        line[n] = event->name[i];
        if (line[n] == ' ')
            lastSpace = n;
        line[++n] = 0;

        int16_t xt1, yt1;
        uint16_t w, h;

        // Gets text bounds
        display.getTextBounds(line, 0, 0, &xt1, &yt1, &w, &h);

        // Char out of bounds, put in next line
        if (w > WIDTH / NUMBER_COLUMNS - 30)
        {
            // if there was a space 5 chars before, break line there
            if (n - lastSpace < 5)
            {
                i -= n - lastSpace - 1;
                line[lastSpace] = 0;
            }

            // Print text line
            display.setCursor(x1 + 5, display.getCursorY());
            display.println(line);

            // Clears line (null termination on first charachter)
            line[0] = 0;
            n = 0;
        }
    }

    // display last line
    display.setCursor(x1 + 5, display.getCursorY());
    display.println(line);

    // Set cursor on same y but change x
    display.setCursor(x1 + 3, display.getCursorY());
    display.setFont(&FreeSans9pt7b);

    // Print time
    // also, if theres a location print it
    if (strlen(event->location) != 1)
    {
        display.println(event->time);

        display.setCursor(x1 + 5, display.getCursorY());

        char line[128] = {0};

        for (int i = 0; i < strlen(event->location); ++i)
        {
            line[i] = event->location[i];
            line[i + 1] = 0;

            int16_t xt1, yt1;
            uint16_t w, h;

            // Gets text bounds
            display.getTextBounds(line, 0, 0, &xt1, &yt1, &w, &h);

            if (w > (WIDTH / NUMBER_COLUMNS))
            {
                for (int j = i - 1; j > max(-1, i - 4); --j)
                    line[j] = '.';
                line[i] = 0;
            }
        }

        display.print(line);
    }
    else
    {
        display.print(event->time);
    }

    int bx1 = x1 + 2;
    int by1 = y1;
    int bx2 = x1 + WIDTH / NUMBER_COLUMNS - 7;
    int by2 = display.getCursorY() + 7;

    // Draw event rect bounds
    display.drawThickLine(bx1, by1, bx1, by2, 0, 2.0);
    display.drawThickLine(bx1, by2, bx2, by2, 0, 2.0);
    display.drawThickLine(bx2, by2, bx2, by1, 0, 2.0);
    display.drawThickLine(bx2, by1, bx1, by1, 0, 2.0);

    // Set how high is the event
    *heigthNeeded = display.getCursorY() + 12 - y1;
    Serial.print("print event ");
    Serial.println(event -> name);
    Serial.println(x1);
    Serial.println(y1);

    // Return is it overflowing
    return display.getCursorY() < maxHeigth - 5;
}

// Struct event comparison function, by timestamp, used for qsort later on
int cmp(const void *a, const void *b)
{
    entry *entryA = (entry *)a;
    entry *entryB = (entry *)b;

    return (entryA->timeStamp - entryB->timeStamp);
}

// Main calendarData drawing calendarData
void Calendar::drawData()
{
    long i = 0;
    long n = strlen(calendarData);

    // reset count
    entriesNum = 0;

    // Search raw calendarData for events
    while (i < n)
    {
        // Find next event start and end
        i = strstr(calendarData + i, "BEGIN:VEVENT") - calendarData + 12;
        char *end = strstr(calendarData + i, "END:VEVENT");

        if (end == NULL)
            break;

        // Find all relevant event calendarData
        char *summary = strstr(calendarData + i, "SUMMARY:") + 8;
        char *location = strstr(calendarData + i, "LOCATION:") + 9;
        char *timeStart = strstr(calendarData + i, "DTSTART:") + 8;
        char *timeEnd = strstr(calendarData + i, "DTEND:") + 6;

        if (summary && summary < end)
        {
            strncpy(calendarEntries[entriesNum].name, summary, strchr(summary, '\n') - summary);
            calendarEntries[entriesNum].name[strchr(summary, '\n') - summary] = 0;
        }
        if (location && location < end)
        {
            strncpy(calendarEntries[entriesNum].location, location, strchr(location, '\n') - location);
            calendarEntries[entriesNum].location[strchr(location, '\n') - location] = 0;
        }
        if (timeStart && timeStart < end && timeEnd < end)
        {
            getToFrom(calendarEntries[entriesNum].time, timeStart, timeEnd, &calendarEntries[entriesNum].day,
                      &calendarEntries[entriesNum].timeStamp);
        }
        Serial.print("found event ");
        Serial.println(calendarEntries[entriesNum].name);
        Serial.println(calendarEntries[entriesNum].time);
        if (calendarEntries[entriesNum].day == -1) {
            Serial.println("unable to match day for");
        }
        ++entriesNum;
    }
    Serial.println("Found total events");
    Serial.println(entriesNum);
    // Sort calendarEntries by time
    qsort(calendarEntries, entriesNum, sizeof(entry), cmp);

    // Events displayed and overflown counters
    int columns[NUMBER_COLUMNS] = {0};
    bool clogged[NUMBER_COLUMNS] = {0};
    int cloggedCount[NUMBER_COLUMNS] = {0};

    // Displaying events one by one
    for (int i = 0; i < entriesNum; ++i)
    {
        // If column overflowed just add event to not shown
        if (calendarEntries[i].day != -1 && clogged[calendarEntries[i].day])
            ++cloggedCount[calendarEntries[i].day];
        if (calendarEntries[i].day == -1 || clogged[calendarEntries[i].day])
            continue;

        // We store how much height did one event take up
        int shift = 0;
        bool s = drawEvent(&calendarEntries[i], calendarEntries[i].day, columns[calendarEntries[i].day] + 64, HEIGHT - 4, &shift);

        columns[calendarEntries[i].day] += shift;

        // If it overflowed, set column to clogged and add one event as not shown
        if (!s)
        {
            Serial.print("overflowed at ");
            Serial.println(calendarEntries[i].name);
            ++cloggedCount[calendarEntries[i].day];
            clogged[calendarEntries[i].day] = 1;
        }
    }

    // Display not shown events info
    for (int i = 0; i < NUMBER_COLUMNS; ++i)
    {
        if (clogged[i])
        {
            // Draw notification showing that there are more events than drawn ones
            display.fillRoundRect(6 + i * (WIDTH / NUMBER_COLUMNS), HEIGHT - 24, (WIDTH / NUMBER_COLUMNS) - 5, 20, 10, 0);
            display.setCursor(10, HEIGHT - 6);
            display.setTextColor(7, 0);
            display.setFont(&FreeSans9pt7b);
            display.print(cloggedCount[i]);
            display.print(" more events");
        }
    }
}
