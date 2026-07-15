#include "i2c.h"
#include "ssd1306.h"
#include "rtc.h"
#include <stdio.h>

int main(void)
{
    RTC_Time t;
    char line1[20], line2[20];

    I2C1_Init();
    SSD1306_Init();

    RTC_Init();
    //RTC_SetTime(07, 00, 0);        // set once — comment out after first flash
    //RTC_SetDate(15, 7, 26);        // 15-Jul-2026

    while(1)
    {
        RTC_GetTime(&t);

        sprintf(line1, "Time: %02d:%02d:%02d", t.hours, t.minutes, t.seconds);
        sprintf(line2, "Date: %02d/%02d/%02d", t.date, t.month, t.year);

        SSD1306_Clear();
        SSD1306_PrintString(0, 2, line1);
        SSD1306_PrintString(0, 4, line2);
        SSD1306_UpdateScreen();

        for (volatile int i = 0; i < 200000; i++);  // crude ~1s-ish delay, refine later
    }
}
