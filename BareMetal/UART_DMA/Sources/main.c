/*
 * main.c
 *
 * STM32F407
 * UART DMA Circular RX Example
 */

#include "stm32f407.h"
#include "uart.h"

  volatile uint32_t iser1;

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

        /* CPU can sleep here */

        /* All UART communication handled
           by DMA + Interrupts */
    }
}
