/*
 * main.c
 *
 * STM32F407
 * RTC Example
 */

#include "stm32f407.h"
#include "uart.h"

volatile uint32_t iser1;

void RTC_SendTime(uint8_t hh, uint8_t mm, uint8_t ss)
{
    char buf[12];  // "HH:MM:SS\r\n" + null

    buf[0] = (hh / 10) + '0';
    buf[1] = (hh % 10) + '0';
    buf[2] = ':';
    buf[3] = (mm / 10) + '0';
    buf[4] = (mm % 10) + '0';
    buf[5] = ':';
    buf[6] = (ss / 10) + '0';
    buf[7] = (ss % 10) + '0';
    buf[8] = '\r';
    buf[9] = '\n';
    buf[10] = '\0';

    UART_SendString_DMA(buf);   // tera existing UART2 driver function
}

void simple_delay(volatile uint32_t count)
{
    for (volatile uint32_t i = 0; i < count; i++);
}
int main(void)
{
    /*-------------------------------------------------------
     * Reset USART2
     *------------------------------------------------------*/
    RCC_APB1RSTR |=  (1U << 17);
    RCC_APB1RSTR &= ~(1U << 17);

    /*-------------------------------------------------------
     * Enable Peripheral Clocks
     *------------------------------------------------------*/
    RCC_AHB1ENR |= (1U << 0);      /* GPIOA */
    RCC_AHB1ENR |= (1U << 3);      /* GPIOD */

    RCC_APB1ENR |= (1U << 17);     /* USART2 */

    RCC_APB2ENR |= (1U << 14);     /* SYSCFG */

    /* PWR clock enable + backup domain unlock */
    RCC_APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR     |= PWR_CR_DBP;

    /* LSI clock select */
    RCC_CSR |= RCC_CSR_LSION;
    while (!(RCC_CSR & RCC_CSR_LSIRDY));

    RCC_BDCR |= RCC_BDCR_RTCSEL_1;   // LSI selected
    RCC_BDCR |= RCC_BDCR_RTCEN;

    /* RTC write protection unlock */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    /* Enter init mode */
    RTC->ISR |= RTC_ISR_INIT;
    while (!(RTC->ISR & RTC_ISR_INITF));

    /* Prescaler */
    RTC->PRER = (127 << 16) | (255);

    /* Set time (BCD) - example 14:30:00 */
    RTC->TR = (0x1 << 20) | (0x4 << 16) | (0x3 << 12) | (0x0 << 8) | (0x0 << 4) | (0x0);

    /* Set date (BCD) - example 11-07-26, Saturday */
    RTC->DR = (0x2 << 20) | (0x6 << 16) | (0x6 << 13) | (0x0 << 12) | (0x7 << 8) | (0x1 << 4) | (0x1);

    /* Exit init mode */
    RTC->ISR &= ~RTC_ISR_INIT;

    /* Lock */
    RTC->WPR = 0xFF;

    /*-------------------------------------------------------
     * Configure UART
     *------------------------------------------------------*/
    UART_Init();

    /*-------------------------------------------------------
     * Configure DMA
     *------------------------------------------------------*/
    UART_DMA_Init();





    UART_DMA_RX_Init();

    /*-------------------------------------------------------
     * Enable DMA1 Stream6 Interrupt
     *------------------------------------------------------*/
    NVIC_ISER0 |= (1U << 17);

    iser1 = NVIC_ISER1;

    /*-------------------------------------------------------
     * Main Loop
     *------------------------------------------------------*/
    while(1)
    {
        /* Background Tasks */

    		uint32_t tr = RTC->TR;
    	    uint32_t dr = RTC->DR;   // TR ke baad hi DR read karna
    	    (void)dr;

    	    uint8_t hh = ((tr >> 20) & 0x3) * 10 + ((tr >> 16) & 0xF);
    	    uint8_t mm = ((tr >> 12) & 0x7) * 10 + ((tr >> 8) & 0xF);
    	    uint8_t ss = ((tr >> 4) & 0x7) * 10 + (tr & 0xF);

    	    RTC_SendTime(hh, mm, ss);
    	    simple_delay(2000000);


        /* CPU can sleep here */

        /* All UART communication handled
           by DMA + Interrupts */
    }
}
