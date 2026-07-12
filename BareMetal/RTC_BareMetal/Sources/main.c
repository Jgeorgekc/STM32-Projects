/*
 * main.c
 *
 * STM32F407
 * RTC Example
 */

#include "stm32f407.h"
#include "uart.h"

volatile uint32_t iser1;
volatile uint32_t systick_counter = 0;
volatile uint8_t  led_on_flag = 0;
volatile uint32_t led_on_timestamp = 0;
volatile uint32_t print_timestamp = 0;   // global, mukalil declare cheyyu
/* ---------- SysTick ---------- */
void SysTick_Init(void)
{
    SysTick->LOAD = (16000000 / 1000) - 1;   // 16MHz HSI, 1ms tick
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE | SysTick_CTRL_TICKINT | SysTick_CTRL_CLKSOURCE;
}

void SysTick_Handler(void)
{
    systick_counter++;
}

/* ---------- LED (PD12) ---------- */
void LED_Init(void)
{
    RCC_AHB1ENR |= (1 << 3);           // GPIOD clock enable
    GPIOD_MODER &= ~(0x3 << (12 * 2));
    GPIOD_MODER |=  (0x1 << (12 * 2)); // PD12 output mode
}

/* ---------- Button (PA0) EXTI ---------- */
void Button_EXTI_Init(void)
{
    RCC_AHB1ENR |= (1 << 0);            // GPIOA clock enable
    GPIOA_MODER &= ~(0x3 << (0 * 2));   // PA0 input mode

    RCC_APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    SYSCFG_EXTICR1 &= ~(0xF << 0);      // PA0 -> EXTI0 (bits 3:0 = 0000)

    EXTI_RTSR |= (1 << 0);              // rising edge trigger
    EXTI_IMR  |= (1 << 0);              // unmask line 0

    NVIC_ISER0 |= (1 << EXTI0_IRQn);
}

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
void RTC_SetAlarm_1MinuteFromNow(void)
{
    uint32_t tr = RTC->TR;

    uint8_t min = ((tr >> 12) & 0x7) * 10 + ((tr >> 8) & 0xF);
    uint8_t hr  = ((tr >> 20) & 0x3) * 10 + ((tr >> 16) & 0xF);

    min = min + 1;
    if (min >= 60) {
        min = 0;
        hr = (hr + 1) % 24;
    }

    uint8_t min_bcd = ((min / 10) << 4) | (min % 10);
    uint8_t hr_bcd  = ((hr / 10) << 4) | (hr % 10);

    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    RTC->CR &= ~RTC_CR_ALRAE;
    while (!(RTC->ISR & RTC_ISR_ALRAWF));

    RTC->ALRMAR = (1U << 31)
                | (hr_bcd  << 16)
                | (min_bcd << 8)
                | (1U << 7);

    RTC->CR |= RTC_CR_ALRAE;
    RTC->CR |= RTC_CR_ALRAIE;

    RTC->WPR = 0xFF;

    EXTI_IMR  |= (1 << 17);   // RTC alarm internal EXTI line 17
    EXTI_RTSR |= (1 << 17);
    NVIC_ISER1 |= (1 << (RTC_Alarm_IRQn - 32));
}

void EXTI0_IRQHandler(void)
{
    if (EXTI_PR & (1 << 0)) {
        EXTI_PR |= (1 << 0);        // clear
        RTC_SetAlarm_1MinuteFromNow();
        UART_SendString_DMA("Button pressed - Alarm set for 1 min\r\n");
    }
}

void RTC_Alarm_IRQHandler(void)
{
    if (RTC->ISR & RTC_ISR_ALRAF) {
        RTC->ISR &= ~RTC_ISR_ALRAF;   // alarm flag clear
        EXTI_PR |= (1 << 17);         // EXTI pending clear

        /* --- Alarm-ne immediately disable cheyyuka (one-shot) --- */
        RTC->WPR = 0xCA;
        RTC->WPR = 0x53;

        RTC->CR &= ~RTC_CR_ALRAE;     // Alarm A disable
        RTC->CR &= ~RTC_CR_ALRAIE;    // Alarm A interrupt disable

        RTC->WPR = 0xFF;

        /* --- LED ON --- */
        GPIOD_ODR |= (1 << 12);
        led_on_flag = 1;
        led_on_timestamp = systick_counter;

        UART_SendString_DMA("Alarm fired - LED ON\r\n");
    }
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

    SysTick_Init();
       LED_Init();
       Button_EXTI_Init();


    /*-------------------------------------------------------
     * Main Loop
     *------------------------------------------------------*/
    while(1)
    {
        /* Background Tasks */

        /* Time print — oru second-il oru thavana mathram */
        if (systick_counter - print_timestamp >= 1000) {
            print_timestamp = systick_counter;

    		uint32_t tr = RTC->TR;
    	    uint32_t dr = RTC->DR;   // TR ke baad hi DR read karna
    	    (void)dr;

    	    uint8_t hh = ((tr >> 20) & 0x3) * 10 + ((tr >> 16) & 0xF);
    	    uint8_t mm = ((tr >> 12) & 0x7) * 10 + ((tr >> 8) & 0xF);
    	    uint8_t ss = ((tr >> 4) & 0x7) * 10 + (tr & 0xF);

    	    RTC_SendTime(hh, mm, ss);
    	    if (led_on_flag && (systick_counter - led_on_timestamp >= 5000)) {
    	           GPIOD_ODR &= ~(1 << 12);
    	           led_on_flag = 0;
    	           UART_SendString_DMA("LED OFF\r\n");
    	       }
        }



        /* CPU can sleep here */

        /* All UART communication handled
           by DMA + Interrupts */
    }
}
