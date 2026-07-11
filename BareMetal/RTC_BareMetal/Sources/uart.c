/*
 * uart.c
 *
 *  Created on: 09-Jul-2026
 *      Author: lenovo
 */
/*
 * uart.c
 *
 * UART Driver
 * STM32F407 Bare Metal
 */

#include "uart.h"
#include "cli.h"
#include "stm32f407.h"

#include <string.h>

/*=========================================================
 * Private Variables
 *========================================================*/

/* TX Interrupt Buffer */
static volatile char tx_buffer[TX_BUFFER_SIZE];
static volatile uint8_t tx_head = 0;
static volatile uint8_t tx_tail = 0;

/* RX Interrupt Buffer (Temporary - remove after full DMA migration) */
static volatile char uart_buffer[UART_BUFFER_SIZE];
static volatile uint8_t head = 0;
static volatile uint8_t tail = 0;

/* DMA RX Circular Buffer */
static volatile uint8_t dma_rx_buffer[DMA_RX_BUFFER_SIZE];

/* DMA RX Processing Position */
static uint16_t old_pos = 0;

/* Driver Status */
static volatile uint8_t tx_busy = 0;
static volatile uint8_t dma_busy = 0;
static volatile uint8_t ore_count = 0;

/*=========================================================
 * Private Function Prototypes
 *========================================================*/

static void UART_ProcessReceivedData(void);
static void UART_ProcessByte(char ch);

volatile uint32_t dma_irq_count = 0;

void UART_Init(void)
{
    /* Reset USART2 */
    RCC_APB1RSTR |= (1U << 17);
    RCC_APB1RSTR &= ~(1U << 17);

    /* PA2 -> USART2_TX */
    GPIOA_MODER &= ~(3U << 4);
    GPIOA_MODER |=  (2U << 4);

    /* PA3 -> USART2_RX */
    GPIOA_MODER &= ~(3U << 6);
    GPIOA_MODER |=  (2U << 6);

    /* AF7 */
    GPIOA_AFRL &= ~(0xFU << 8);
    GPIOA_AFRL |=  (7U << 8);

    GPIOA_AFRL &= ~(0xFU << 12);
    GPIOA_AFRL |=  (7U << 12);

    /* Pull-up */
    GPIOA_PUPDR &= ~(3U << 6);
    GPIOA_PUPDR |=  (1U << 6);

    /* PD12 Output */
    GPIOD_MODER &= ~(3U << 24);
    GPIOD_MODER |=  (1U << 24);

    /* PD13 Output (Orange LED) */
    GPIOD_MODER &= ~(3U << 26);
    GPIOD_MODER |=  (1U << 26);

    /* 115200 @16MHz */
    USART2_BRR = 0x008B;

    USART2_CR1 =
        USART_CR1_UE |
        USART_CR1_TE |
        USART_CR1_RE |
        USART_CR1_IDLEIE;

    /* Enable USART2 IRQ */
    NVIC_ISER1 |= (1U << 6);
}

/*=========================================================
 * Public APIs
 *========================================================*/


uint8_t UART_GetORECount(void)
{
    return ore_count;
}

void UART_SendChar(char ch)
{
    while(!(USART2_SR & USART_SR_TXE));

    USART2_DR = ch;
}

char UART_ReadChar(void)
{
    while(!(USART2_SR & USART_SR_RXNE));

    return (char)(USART2_DR & 0xFF);
}

void UART_SendString(char *str)
{
    while(*str)
    {
        UART_SendChar(*str++);
    }
}

/*=========================================================
 * Interrupt TX APIs
 *========================================================*/

void UART_SendChar_IT(char ch)
{
    uint8_t next = (tx_head + 1) % TX_BUFFER_SIZE;

    /* Wait if software buffer is full */
    while(next == tx_tail);

    tx_buffer[tx_head] = ch;

    tx_head = next;

    if(tx_busy == 0)
    {
        tx_busy = 1;

        USART2_CR1 |= USART_CR1_TXEIE;
    }
}

void UART_SendString_IT(char *str)
{
    while(*str)
    {
        UART_SendChar_IT(*str++);
    }
}

/*=========================================================
 * RX Interrupt Buffer APIs
 *
 * NOTE:
 * These APIs will be removed once
 * Circular DMA migration is complete.
 *========================================================*/

uint8_t UART_Available(void)
{
    return (head != tail);
}

char UART_BufferRead(void)
{
    char data;

    while(head == tail);

    data = uart_buffer[tail];

    tail = (tail + 1) % UART_BUFFER_SIZE;

    return data;
}

/*=========================================================
 * DMA TX APIs
 *========================================================*/

void UART_DMA_Init(void)
{
    /*-------------------------------------------------------
     * Enable DMA1 Clock
     *------------------------------------------------------*/
    RCC_AHB1ENR |= (1U << 21);

    /*-------------------------------------------------------
     * Disable Stream6 before configuration
     *------------------------------------------------------*/
    DMA1_S6CR &= ~(1U << 0);

    while(DMA1_S6CR & (1U << 0))
    {
        /* Wait */
    }

    /*-------------------------------------------------------
     * Select Channel4
     * USART2_TX -> DMA1 Stream6 Channel4
     *------------------------------------------------------*/
    DMA1_S6CR &= ~(7U << 25);
    DMA1_S6CR |=  (4U << 25);

    /*-------------------------------------------------------
     * Memory -> Peripheral
     *------------------------------------------------------*/
    DMA1_S6CR &= ~(3U << 6);
    DMA1_S6CR |=  (1U << 6);

    /*-------------------------------------------------------
     * Memory Increment Enable
     *------------------------------------------------------*/
    DMA1_S6CR |= (1U << 10);

    /*-------------------------------------------------------
     * Peripheral Increment Disable
     *------------------------------------------------------*/
    DMA1_S6CR &= ~(1U << 9);

    /*-------------------------------------------------------
     * Normal Mode
     *------------------------------------------------------*/
    DMA1_S6CR &= ~(1U << 8);

    /*-------------------------------------------------------
     * Medium Priority
     *------------------------------------------------------*/
    DMA1_S6CR &= ~(3U << 16);
    DMA1_S6CR |=  (1U << 16);

    /*-------------------------------------------------------
     * Peripheral Address = USART2_DR
     *------------------------------------------------------*/
    DMA1_S6PAR = (uint32_t)&USART2_DR;

    /*-------------------------------------------------------
     * Enable USART DMA TX
     *------------------------------------------------------*/
    USART2_CR3 |= (1U << 7);

    /*-------------------------------------------------------
     * Enable Transfer Complete Interrupt
     *------------------------------------------------------*/
     DMA1_S6CR |= (1U << 4);   // TCIE
}

void UART_SendString_DMA(char *str)
{
    /* Wait until previous DMA transfer completes */
   // while(dma_busy);

    dma_busy = 1;

    /* Disable Stream6 */
    DMA1_S6CR &= ~(1U << 0);

    while(DMA1_S6CR & (1U << 0))
    {
        /* Wait */
    }

    /* Clear Transfer Complete Flag */
    DMA1_HIFCR = DMA_CTCIF6;

    /* Source Address */
    DMA1_S6M0AR = (uint32_t)str;

    /* Number of Bytes */
    DMA1_S6NDTR = strlen(str);

    /* Enable Stream6 */
    DMA1_S6CR |= (1U << 0);

    /* Small delay */
    for(volatile int i = 0; i < 5000; i++);

    /* Save HISR value */
    dma_irq_count = DMA1_HISR;



    /* ---------- END DEBUG ---------- */
}

/*=========================================================
 * DMA RX Initialization
 * USART2_RX -> DMA1 Stream5 Channel4
 *========================================================*/

void UART_DMA_RX_Init(void)
{
    /*-------------------------------------------------------
     * Disable Stream5
     *------------------------------------------------------*/
    DMA1_S5CR &= ~(1U << 0);

    while(DMA1_S5CR & (1U << 0))
    {
        /* Wait */
    }

    /*-------------------------------------------------------
     * Select Channel4
     *------------------------------------------------------*/
    DMA1_S5CR &= ~(7U << 25);
    DMA1_S5CR |=  (4U << 25);

    /*-------------------------------------------------------
     * Peripheral -> Memory
     *------------------------------------------------------*/
    DMA1_S5CR &= ~(3U << 6);

    /*-------------------------------------------------------
     * Memory Increment Enable
     *------------------------------------------------------*/
    DMA1_S5CR |= (1U << 10);

    /*-------------------------------------------------------
     * Peripheral Increment Disable
     *------------------------------------------------------*/
    DMA1_S5CR &= ~(1U << 9);

    /*-------------------------------------------------------
     * Circular Mode
     *------------------------------------------------------*/
    DMA1_S5CR |= (1U << 8);

    /*-------------------------------------------------------
     * Medium Priority
     *------------------------------------------------------*/
    DMA1_S5CR &= ~(3U << 16);
    DMA1_S5CR |=  (1U << 16);

    /*-------------------------------------------------------
     * Peripheral Address
     *------------------------------------------------------*/
    DMA1_S5PAR = (uint32_t)&USART2_DR;

    /*-------------------------------------------------------
     * Memory Address
     *------------------------------------------------------*/
    DMA1_S5M0AR = (uint32_t)dma_rx_buffer;

    /*-------------------------------------------------------
     * Buffer Size
     *------------------------------------------------------*/
    DMA1_S5NDTR = DMA_RX_BUFFER_SIZE;

    /*-------------------------------------------------------
     * Enable USART RX DMA
     *------------------------------------------------------*/
    USART2_CR3 |= (1U << 6);

    /*-------------------------------------------------------
     * Enable Stream5
     *------------------------------------------------------*/
    DMA1_S5CR |= (1U << 0);
}

/*=========================================================
 * USART2 Interrupt Handler
 *========================================================*/

void USART2_IRQHandler(void)
{
    /* IDLE Line Detection */
    if(USART2_SR & USART_SR_IDLE)
    {
        volatile uint32_t temp;

        /* Clear IDLE Flag */
        temp = USART2_SR;
        temp = USART2_DR;

        (void)temp;

        UART_ProcessReceivedData();
    }
}
/*=========================================================
 * Process Newly Received DMA Data
 *========================================================*/

static void UART_ProcessReceivedData(void)
{
    uint16_t new_pos;

    new_pos = DMA_RX_BUFFER_SIZE - DMA1_S5NDTR;

    /*-------------------------------------------------------
     * Normal Case
     *------------------------------------------------------*/
    if(new_pos >= old_pos)
    {
        for(uint16_t i = old_pos; i < new_pos; i++)
        {
            UART_ProcessByte(dma_rx_buffer[i]);
        }
    }
    /*-------------------------------------------------------
     * Buffer Wrapped Around
     *------------------------------------------------------*/
    else
    {
        /* old_pos -> Buffer End */
        for(uint16_t i = old_pos;
            i < DMA_RX_BUFFER_SIZE;
            i++)
        {
            UART_ProcessByte(dma_rx_buffer[i]);
        }

        /* Buffer Start -> new_pos */
        for(uint16_t i = 0;
            i < new_pos;
            i++)
        {
            UART_ProcessByte(dma_rx_buffer[i]);
        }
    }

    old_pos = new_pos;
}
static void UART_ProcessByte(char ch)
{
    CLI_ProcessByte(ch);
}


void DMA1_Stream6_IRQHandler(void)
{
    if(DMA1_HISR & DMA_TCIF6)
    {
        /* Clear Transfer Complete Flag */
        DMA1_HIFCR = DMA_CTCIF6;

        dma_busy = 0;

        GPIOD_ODR ^= (1U << 13);   // Toggle Orange LED

        dma_irq_count++;
    }
}
