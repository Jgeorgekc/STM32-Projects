/*
 * uart.c
 *
 *  Created on: 09-Jul-2026
 *      Author: lenovo
 */
#include "uart.h"
#include "stm32f407.h"

static volatile char tx_buffer[TX_BUFFER_SIZE];
static volatile uint8_t tx_head = 0;
static volatile uint8_t tx_tail = 0;
static volatile char uart_buffer[UART_BUFFER_SIZE];
static volatile uint8_t head = 0;
static volatile uint8_t tail = 0;
static volatile char rx;
static volatile int x = 0;
static volatile uint32_t uart_sr = 0;
static volatile uint8_t tx_busy = 0;


void delay(volatile uint32_t count)
{
    while(count--);
}

static volatile uint8_t ore_count = 0;

uint8_t UART_GetORECount(void)
{
    return ore_count;
}

void UART_SendChar(char ch)
{
    while (!(USART2_SR & (1 << 7)));   // TXE

    USART2_DR = ch;
}

char UART_ReadChar(void)
{
    while (!(USART2_SR & (1 << 5)));   // RXNE

    return (char)(USART2_DR & 0xFF);
}

void UART_SendString(char *str)
{
    while (*str)
    {
        UART_SendChar(*str++);
    }
}
void USART2_IRQHandler(void)
{
	volatile uint32_t rx_count = 0;

    /* Capture status register */
    uart_sr = USART2_SR;

    /* Check Overrun Error */
    if (uart_sr & (1 << 3))
    {
        ore_count++;

        /* Clear ORE:
         * Read SR first (already done),
         * then read DR.
         */
        volatile uint32_t dummy = USART2_DR;
        (void)dummy;

        return;
    }


    if (USART2_SR & (1 << 5))
    {
    	uart_buffer[head]=USART2_DR;

    	head=(head+1)%UART_BUFFER_SIZE;

    	rx_count++;

    }
    /* TXE Interrupt */
    if ((USART2_SR & (1 << 7)) && (USART2_CR1 & (1 << 7)))
    {
        USART2_DR = tx_buffer[tx_tail];

        tx_tail = (tx_tail + 1) % TX_BUFFER_SIZE;

        /* Buffer Empty? */
        if (tx_head == tx_tail)
        {
            tx_busy = 0;
            USART2_CR1 &= ~(1 << 7);
        }
    }
}

void EXTI0_IRQHandler(void)
{
    if (EXTI_PR & (1<<0))
    {
        EXTI_PR |= (1<<0);

        UART_SendString_IT("Button Pressed\r\n");
    }
}
char UART_BufferRead(void)
{
    char data;

    /* Buffer empty? */
    while (head == tail);

    data = uart_buffer[tail];

    tail = (tail + 1) % UART_BUFFER_SIZE;

    return data;
}
void UART_SendString_IT(char *str)
{
    while(*str)
    {
        UART_SendChar_IT(*str++);
    }
}
void UART_SendChar_IT(char ch)
{
    uint8_t next = (tx_head + 1) % TX_BUFFER_SIZE;

    /* Wait if buffer is full */
    while(next == tx_tail);

    tx_buffer[tx_head] = ch;
    tx_head = next;

    /* Start transmission if idle */
    if(tx_busy == 0)
    {
        tx_busy = 1;
        USART2_CR1 |= (1 << 7);      // Enable TXE interrupt
    }
}

uint8_t UART_Available(void)
{
    return (head != tail);
}

void UART_SendString_DMA(char *str)
{
    /* Disable Stream */

    /* Wait until disabled */

    /* Clear DMA Flags */

    /* Memory Address */

    /* Peripheral Address */

    /* Transfer Length */

    /* Enable UART DMA */

    /* Enable DMA Stream */

    /*-------------------------------------------------------
     * Enable DMA1 Clock
     *
     * DMA1 belongs to AHB1 Bus.
     * Without enabling this clock, DMA registers
     * cannot be accessed.
     *------------------------------------------------------*/
    RCC_AHB1ENR |= (1 << 21);

    /*-------------------------------------------------------
     * Disable Stream6 before configuration.
     * DMA registers can be modified only when EN = 0.
     *------------------------------------------------------*/
    DMA1_S6CR &= ~(1 << 0);      // EN = 0

    /* Wait until hardware clears EN bit */
    while (DMA1_S6CR & (1 << 0));

    /*-------------------------------------------------------
     * Select Channel 4
     *
     * USART2_TX -> DMA1 Stream6 Channel4
     *------------------------------------------------------*/
    DMA1_S6CR &= ~(7 << 25);      // Clear CHSEL bits
    DMA1_S6CR |=  (4 << 25);      // CHSEL = 4

    /*-------------------------------------------------------
     * Memory -> Peripheral
     *
     * Data will move from RAM to USART2_DR.
     *------------------------------------------------------*/
    DMA1_S6CR &= ~(3 << 6);       // Clear DIR bits
    DMA1_S6CR |=  (1 << 6);       // DIR = 01

    /*-------------------------------------------------------
     * Enable Memory Increment
     *
     * Move to next byte after every transfer.
     *------------------------------------------------------*/
    DMA1_S6CR |= (1 << 10);

    /*-------------------------------------------------------
     * Disable Peripheral Increment
     *
     * USART_DR address is always fixed.
     *------------------------------------------------------*/
    DMA1_S6CR &= ~(1 << 9);

    /*-------------------------------------------------------
     * Disable Circular Mode
     *
     * One-shot transmission.
     *------------------------------------------------------*/
    DMA1_S6CR &= ~(1 << 8);

    /*-------------------------------------------------------
     * Medium Priority
     *------------------------------------------------------*/
    DMA1_S6CR &= ~(3 << 16);
    DMA1_S6CR |=  (1 << 16);

    /*-------------------------------------------------------
     * Peripheral Address Register
     *
     * Destination = USART2 Data Register
     * DMA writes every byte to USART2_DR.
     *------------------------------------------------------*/
    DMA1_S6PAR = (uint32_t)&USART2_DR;



    /*-------------------------------------------------------
     * Memory Address Register
     *
     * Source = msg buffer
     * DMA starts reading from this address.
     *------------------------------------------------------*/
    DMA1_S6M0AR = (uint32_t)str;

    /*-------------------------------------------------------
     * Number of Data Register
     *
     * Total bytes to transfer.
     *------------------------------------------------------*/
    DMA1_S6NDTR = strlen(str);

    /*-------------------------------------------------------
     * Enable USART2 DMA Transmitter
     *
     * When TXE occurs, USART2 will generate
     * DMA requests instead of CPU interrupts.
     *------------------------------------------------------*/
    USART2_CR3 |= (1 << 7);      // DMAT = 1

    /*-------------------------------------------------------
     * Enable DMA Stream6
     *
     * DMA starts transferring data
     * from Memory -> USART2_DR.
     *------------------------------------------------------*/
    DMA1_S6CR |= (1 << 0);      // EN = 1
}

