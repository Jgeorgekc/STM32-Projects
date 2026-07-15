/*
 * rtc.c
 *
 *  Created on: 15-Jul-2026
 *      Author: lenovo
 */


#include "stm32f407.h"
#include "rtc.h"

static uint8_t bcd2bin(uint8_t bcd) { return ((bcd >> 4) * 10) + (bcd & 0x0F); }
static uint8_t bin2bcd(uint8_t bin) { return ((bin / 10) << 4) | (bin % 10); }

/*
 * WHAT: RTC init using LSI clock (matches ninte header's RCC_CSR_LSION/LSIRDY macros)
 * WHY : No external crystal needed
 * HOW : Enable PWR clock -> DBP -> reset backup domain -> select LSI (RTCSEL=10) ->
 *       enable RTC -> unlock WPR -> init mode -> prescaler -> exit init mode
 */
void RTC_Init(void)
{
    RCC_APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_DBP;

    /* Enable LSI */
    RCC_CSR |= RCC_CSR_LSION;
    while (!(RCC_CSR & RCC_CSR_LSIRDY));

    /* Reset backup domain */
    RCC_BDCR |= (1U << 16);   // BDRST
    RCC_BDCR &= ~(1U << 16);

    /* RTCSEL = 10 (LSI): RTCSEL_1=1, RTCSEL_0=0 */
    RCC_BDCR &= ~(RCC_BDCR_RTCSEL_0 | RCC_BDCR_RTCSEL_1);
    RCC_BDCR |= RCC_BDCR_RTCSEL_1;

    RCC_BDCR |= RCC_BDCR_RTCEN;

    /* Unlock RTC write protection */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    RTC->ISR |= RTC_ISR_INIT;
    while (!(RTC->ISR & RTC_ISR_INITF));

    /* Prescaler: LSI ~32kHz -> async(127+1)*sync(249+1) = 32000 -> 1Hz
       (tune sync value if LSI on ninte chip runs off-spec) */
    RTC->PRER = (127U << 16) | (249U << 0);

    RTC->ISR &= ~RTC_ISR_INIT;
    RTC->WPR = 0xFF;
}

void RTC_SetTime(uint8_t hh, uint8_t mm, uint8_t ss)
{
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    RTC->ISR |= RTC_ISR_INIT;
    while (!(RTC->ISR & RTC_ISR_INITF));

    RTC->TR = (bin2bcd(hh) << 16) | (bin2bcd(mm) << 8) | bin2bcd(ss);

    RTC->ISR &= ~RTC_ISR_INIT;
    RTC->WPR = 0xFF;
}

void RTC_SetDate(uint8_t date, uint8_t month, uint8_t year)
{
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    RTC->ISR |= RTC_ISR_INIT;
    while (!(RTC->ISR & RTC_ISR_INITF));

    RTC->DR = (bin2bcd(year) << 16) | (bin2bcd(month) << 8) | bin2bcd(date);

    RTC->ISR &= ~RTC_ISR_INIT;
    RTC->WPR = 0xFF;
}

/*
 * WHAT: Read time+date, BCD->binary
 * WHY : RTC->DR must be read AFTER RTC->TR (shadow register hardware rule, RM0090)
 */
void RTC_GetTime(RTC_Time *t)
{
    uint32_t tr = RTC->TR;
    uint32_t dr = RTC->DR;   // read after TR — don't reorder

    t->hours   = bcd2bin((tr >> 16) & 0x3F);
    t->minutes = bcd2bin((tr >> 8) & 0x7F);
    t->seconds = bcd2bin(tr & 0x7F);

    t->date  = bcd2bin(dr & 0x3F);
    t->month = bcd2bin((dr >> 8) & 0x1F);
    t->year  = bcd2bin((dr >> 16) & 0xFF);
}
