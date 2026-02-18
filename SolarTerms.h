#ifndef SOLAR_TERMS_H
#define SOLAR_TERMS_H

#include <Arduino.h>

#define SOLAR_TERM_COUNT 24
#define SOLAR_TERM_YEAR_START 2020
#define SOLAR_TERM_YEAR_END 2040
#define SOLAR_TERM_YEAR_SPAN (SOLAR_TERM_YEAR_END - SOLAR_TERM_YEAR_START + 1)

struct SolarTermEntry {
    int month;
    int day;
    int termIndex;  // 0-23
};

// Solar term names in Chinese (UTF-8), ordered starting from 小寒 (index 0)
// Following the traditional astronomical order within a year:
//  0: 小寒  1: 大寒  2: 立春  3: 雨水  4: 惊蛰  5: 春分
//  6: 清明  7: 谷雨  8: 立夏  9: 小满 10: 芒种 11: 夏至
// 12: 小暑 13: 大暑 14: 立秋 15: 处暑 16: 白露 17: 秋分
// 18: 寒露 19: 霜降 20: 立冬 21: 小雪 22: 大雪 23: 冬至
extern const char* const solarTermNames[SOLAR_TERM_COUNT];

// Fill an array with all 24 solar terms for a given year.
// Returns the number of terms filled (24 on success, 0 if year out of range).
int getSolarTerms(int year, SolarTermEntry *terms);

// Check if a specific date is a solar term.
// Returns the term index (0-23) if found, or -1 if not a solar term.
int findSolarTerm(int year, int month, int day);

// Get the name of a solar term by index (0-23).
const char* getSolarTermName(int index);

#endif // SOLAR_TERMS_H
