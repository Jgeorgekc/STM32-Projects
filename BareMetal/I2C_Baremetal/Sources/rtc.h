/*
 * rtc.h
 *
 *  Created on: 15-Jul-2026
 *      Author: lenovo
 */

#ifndef RTC_H
#define RTC_H

#include <stdint.h>

typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} RTC_Time;

void RTC_Init(void);
void RTC_GetTime(RTC_Time *t);
void RTC_SetTime(uint8_t hh, uint8_t mm, uint8_t ss);
void RTC_SetDate(uint8_t date, uint8_t month, uint8_t year);

#endif
