#include "SolarTerms.h"

// Solar term names in Chinese (UTF-8)
const char* const solarTermNames[SOLAR_TERM_COUNT] = {
    "\xe5\xb0\x8f\xe5\xaf\x92",     // 小寒
    "\xe5\xa4\xa7\xe5\xaf\x92",     // 大寒
    "\xe7\xab\x8b\xe6\x98\xa5",     // 立春
    "\xe9\x9b\xa8\xe6\xb0\xb4",     // 雨水
    "\xe6\x83\x8a\xe8\x9b\xb0",     // 惊蛰
    "\xe6\x98\xa5\xe5\x88\x86",     // 春分
    "\xe6\xb8\x85\xe6\x98\x8e",     // 清明
    "\xe8\xb0\xb7\xe9\x9b\xa8",     // 谷雨
    "\xe7\xab\x8b\xe5\xa4\x8f",     // 立夏
    "\xe5\xb0\x8f\xe6\xbb\xa1",     // 小满
    "\xe8\x8a\x92\xe7\xa7\x8d",     // 芒种
    "\xe5\xa4\x8f\xe8\x87\xb3",     // 夏至
    "\xe5\xb0\x8f\xe6\x9a\x91",     // 小暑
    "\xe5\xa4\xa7\xe6\x9a\x91",     // 大暑
    "\xe7\xab\x8b\xe7\xa7\x8b",     // 立秋
    "\xe5\xa4\x84\xe6\x9a\x91",     // 处暑
    "\xe7\x99\xbd\xe9\x9c\xb2",     // 白露
    "\xe7\xa7\x8b\xe5\x88\x86",     // 秋分
    "\xe5\xaf\x92\xe9\x9c\xb2",     // 寒露
    "\xe9\x9c\x9c\xe9\x99\x8d",     // 霜降
    "\xe7\xab\x8b\xe5\x86\xac",     // 立冬
    "\xe5\xb0\x8f\xe9\x9b\xaa",     // 小雪
    "\xe5\xa4\xa7\xe9\x9b\xaa",     // 大雪
    "\xe5\x86\xac\xe8\x87\xb3"      // 冬至
};

// Each solar term index maps to a fixed month:
// 0,1 -> Jan(1); 2,3 -> Feb(2); 4,5 -> Mar(3); 6,7 -> Apr(4);
// 8,9 -> May(5); 10,11 -> Jun(6); 12,13 -> Jul(7); 14,15 -> Aug(8);
// 16,17 -> Sep(9); 18,19 -> Oct(10); 20,21 -> Nov(11); 22,23 -> Dec(12)
static const uint8_t termMonth[SOLAR_TERM_COUNT] = {
    1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
    7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12
};

// Day-of-month lookup table for each solar term.
// 24 entries per year (just the day), 21 years (2020-2040).
// Only the day varies year to year (by 1-2 days); the month is fixed.
//
// Sources: Hong Kong Observatory, Time and Date astronomical calculations
// Order per row: 小寒 大寒 立春 雨水 惊蛰 春分 清明 谷雨 立夏 小满 芒种 夏至
//                小暑 大暑 立秋 处暑 白露 秋分 寒露 霜降 立冬 小雪 大雪 冬至
static const uint8_t solarTermDays[SOLAR_TERM_YEAR_SPAN][SOLAR_TERM_COUNT] PROGMEM = {
    // 2020
    {  6, 20,  4, 19,  5, 20,  4, 19,  5, 20,  5, 21,
       6, 22,  7, 22,  7, 22,  8, 23,  7, 22,  7, 21 },
    // 2021
    {  5, 20,  3, 18,  5, 20,  4, 20,  5, 21,  5, 21,
       7, 22,  7, 23,  7, 23,  8, 23,  7, 22,  7, 21 },
    // 2022
    {  5, 20,  4, 19,  5, 20,  5, 20,  5, 21,  6, 21,
       7, 23,  7, 23,  7, 23,  8, 23,  7, 22,  7, 22 },
    // 2023
    {  5, 20,  4, 19,  6, 21,  5, 20,  6, 21,  6, 21,
       7, 23,  8, 23,  8, 23,  8, 24,  8, 22,  7, 22 },
    // 2024
    {  6, 20,  4, 19,  5, 20,  4, 19,  5, 20,  5, 21,
       6, 22,  7, 22,  7, 22,  8, 23,  7, 22,  6, 21 },
    // 2025
    {  5, 20,  3, 18,  5, 20,  4, 20,  5, 21,  5, 21,
       7, 22,  7, 23,  7, 23,  8, 23,  7, 22,  7, 21 },
    // 2026
    {  5, 20,  4, 18,  5, 20,  5, 20,  5, 21,  5, 21,
       7, 23,  7, 23,  7, 23,  8, 23,  7, 22,  7, 22 },
    // 2027
    {  5, 20,  4, 19,  6, 21,  5, 20,  6, 21,  6, 21,
       7, 23,  8, 23,  8, 23,  8, 23,  7, 22,  7, 22 },
    // 2028
    {  6, 20,  4, 19,  5, 20,  4, 19,  5, 20,  5, 21,
       6, 22,  7, 22,  7, 22,  8, 23,  7, 22,  6, 21 },
    // 2029
    {  5, 20,  3, 18,  5, 20,  4, 20,  5, 21,  5, 21,
       7, 22,  7, 23,  7, 23,  8, 23,  7, 22,  7, 21 },
    // 2030
    {  5, 20,  4, 18,  5, 20,  5, 20,  5, 21,  5, 21,
       7, 23,  7, 23,  7, 23,  8, 23,  7, 22,  7, 22 },
    // 2031
    {  5, 20,  4, 19,  6, 21,  5, 20,  6, 21,  6, 21,
       7, 23,  8, 23,  8, 23,  8, 23,  7, 22,  7, 22 },
    // 2032
    {  6, 20,  4, 19,  5, 20,  4, 19,  5, 20,  5, 21,
       6, 22,  7, 22,  7, 22,  8, 23,  7, 22,  6, 21 },
    // 2033
    {  5, 20,  3, 18,  5, 20,  4, 20,  5, 21,  5, 21,
       7, 22,  7, 23,  7, 23,  8, 23,  7, 22,  7, 21 },
    // 2034
    {  5, 20,  4, 18,  5, 20,  5, 20,  5, 21,  5, 21,
       7, 23,  7, 23,  7, 23,  8, 23,  7, 22,  7, 22 },
    // 2035
    {  5, 20,  4, 19,  6, 21,  5, 20,  5, 21,  6, 21,
       7, 23,  7, 23,  8, 23,  8, 23,  7, 22,  7, 22 },
    // 2036
    {  6, 20,  4, 19,  5, 20,  4, 19,  5, 20,  5, 21,
       6, 22,  7, 22,  7, 22,  8, 23,  7, 22,  6, 21 },
    // 2037
    {  5, 20,  3, 18,  5, 20,  4, 20,  5, 21,  5, 21,
       7, 22,  7, 23,  7, 23,  8, 23,  7, 22,  7, 21 },
    // 2038
    {  5, 20,  4, 18,  5, 20,  5, 20,  5, 21,  5, 21,
       7, 23,  7, 23,  7, 23,  8, 23,  7, 22,  7, 22 },
    // 2039
    {  5, 20,  4, 19,  6, 21,  5, 20,  5, 21,  6, 21,
       7, 23,  7, 23,  8, 23,  8, 23,  7, 22,  7, 22 },
    // 2040
    {  6, 20,  4, 19,  5, 20,  4, 19,  5, 20,  5, 21,
       6, 22,  7, 22,  7, 22,  8, 23,  7, 22,  6, 21 },
};

int getSolarTerms(int year, SolarTermEntry *terms) {
    if (year < SOLAR_TERM_YEAR_START || year > SOLAR_TERM_YEAR_END) {
        return 0;
    }
    int idx = year - SOLAR_TERM_YEAR_START;
    for (int i = 0; i < SOLAR_TERM_COUNT; i++) {
        terms[i].month = termMonth[i];
        terms[i].day = pgm_read_byte(&solarTermDays[idx][i]);
        terms[i].termIndex = i;
    }
    return SOLAR_TERM_COUNT;
}

int findSolarTerm(int year, int month, int day) {
    if (year < SOLAR_TERM_YEAR_START || year > SOLAR_TERM_YEAR_END) {
        return -1;
    }
    int idx = year - SOLAR_TERM_YEAR_START;
    for (int i = 0; i < SOLAR_TERM_COUNT; i++) {
        if (termMonth[i] == month &&
            pgm_read_byte(&solarTermDays[idx][i]) == day) {
            return i;
        }
    }
    return -1;
}

const char* getSolarTermName(int index) {
    if (index < 0 || index >= SOLAR_TERM_COUNT) {
        return "";
    }
    return solarTermNames[index];
}
