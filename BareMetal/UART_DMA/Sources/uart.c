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
volatile uint8_t dma_busy = 0;

#define DMA_RX_BUFFER_SIZE    64

volatile uint8_t dma_rx_buffer[DMA_RX_BUFFER_SIZE];

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
void UART_DMA_Init(void)
{
    /* Disable Stream */

    /* Wait until disabled */

    /* Clear DMA Flags */

    /* Memory Address */

    /* Peripheral Address */

    /* Transfer Length */

    /* Enable UART DMA */

    /*-------------------------------------------------------
     * Enable DMA1 Clock
     *------------------------------------------------------*/
    RCC_AHB1ENR |= (1 << 21);

    /* Disable Stream6 */
    DMA1_S6CR &= ~(1 << 0);

    while(DMA1_S6CR & (1 << 0));

    /*-------------------------------------------------------
     * Channel 4
     *------------------------------------------------------*/
    DMA1_S6CR &= ~(7 << 25);
    DMA1_S6CR |=  (4 << 25);

    /*-------------------------------------------------------
     * Memory -> Peripheral
     *------------------------------------------------------*/
    DMA1_S6CR &= ~(3 << 6);
    DMA1_S6CR |=  (1 << 6);

    /* Memory Increment */
    DMA1_S6CR |= (1 << 10);

    /* Peripheral Increment Disable */
    DMA1_S6CR &= ~(1 << 9);

    /* Normal Mode */
    DMA1_S6CR &= ~(1 << 8);

    /* Medium Priority */
    DMA1_S6CR &= ~(3 << 16);
    DMA1_S6CR |=  (1 << 16);

    /* Peripheral Address */
    DMA1_S6PAR = (uint32_t)&USART2_DR;

    /* Enable USART DMA */
    USART2_CR3 |= (1 << 7);

    /*-------------------------------------------------------
     * Enable Transfer Complete Interrupt
     *------------------------------------------------------*/
    DMA1_S6CR |= (1 << 4);      // TCIE = 1
}
uint8_t UART_DMA_IsBusy(void)
{
    return dma_busy;
}

void UART_SendString_DMA(char *str)
{

		dma_busy = 1;

	 /* Disable Stream */
	    DMA1_S6CR &= ~(1 << 0);

	    while(DMA1_S6CR & (1 << 0)){};

	    /* Clear Transfer Complete Flag */
	    /* Clear previous Transfer Complete Flag */
	    DMA1_HIFCR = DMA_CTCIF6;

	     /* Source Address */
	      DMA1_S6M0AR = (uint32_t)str;

	     /* Transfer Length */
	      DMA1_S6NDTR = strlen(str);

    /*-------------------------------------------------------
     * Enable DMA Stream6
     *
     * DMA starts transferring data
     * from Memory -> USART2_DR.
     *------------------------------------------------------*/
    DMA1_S6CR |= (1 << 0);      // EN = 1
}
void DMA1_Stream6_IRQHandler(void)
{
    /* Transfer Complete? */

    if(DMA1_HISR & DMA_TCIF6)
    {
        /* Clear TC Flag */
        DMA1_HIFCR = DMA_TCIF6;

        dma_busy = 0;
    }
}

void UART_DMA_RX_Init(void)
{
    /* Disable Stream5 */
	/*-------------------------------------------------------
	 * Disable Stream5 before configuration.
	 * DMA registers can be modified only when EN = 0.
	 *------------------------------------------------------*/
	DMA1_S5CR &= ~(1 << 0);

	/* Wait until hardware clears EN bit */
	while (DMA1_S5CR & (1 << 0))
	{
	    /* Wait */
	}


    /* Wait until disabled */

    /* Select Channel4 */
	/*-------------------------------------------------------
	 * USART2_RX uses DMA1 Stream5 Channel4
	 *------------------------------------------------------*/
	DMA1_S5CR &= ~(7 << 25);
	DMA1_S5CR |=  (4 << 25);

    /* Peripheral -> Memory */
	/*-------------------------------------------------------
	 * Peripheral -> Memory
	 *------------------------------------------------------*/
	DMA1_S5CR &= ~(3 << 6);

    /* Memory Increment Enable */
	/*-------------------------------------------------------
	 * Enable Memory Increment
	 *------------------------------------------------------*/
	DMA1_S5CR |= (1 << 10);

    /* Peripheral Increment Disable */
	/*-------------------------------------------------------
	 * Disable Peripheral Increment
	 *------------------------------------------------------*/
	DMA1_S5CR &= ~(1 << 9);

    /* Normal Mode */
	/*-------------------------------------------------------
	 * Normal Mode
	 *------------------------------------------------------*/
	DMA1_S5CR &= ~(1 << 8);

    /* Medium Priority */
	/*-------------------------------------------------------
	 * Medium Priority
	 *------------------------------------------------------*/
	DMA1_S5CR &= ~(3 << 16);
	DMA1_S5CR |=  (1 << 16);

    /* Peripheral Address */
	/*-------------------------------------------------------
	 * Source = USART2 Data Register
	 *------------------------------------------------------*/
	DMA1_S5PAR = (uint32_t)&USART2_DR;

    /* Memory Address */
	/*-------------------------------------------------------
	 * Destination = RX DMA Buffer
	 *------------------------------------------------------*/
	DMA1_S5M0AR = (uint32_t)dma_rx_buffer;

    /* Transfer Length */
	/*-------------------------------------------------------
	 * Receive 64 Bytes
	 *------------------------------------------------------*/
	DMA1_S5NDTR = DMA_RX_BUFFER_SIZE;

    /* Enable USART RX DMA */
	/*-------------------------------------------------------
	 * Enable USART2 RX DMA Request
	 *------------------------------------------------------*/
	USART2_CR3 |= (1 << 6);
    /* Enable Stream5 */
	/*-------------------------------------------------------
	 * Start DMA Reception
	 *------------------------------------------------------*/
	DMA1_S5CR |= (1 << 0);
}
