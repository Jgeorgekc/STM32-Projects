/*
 * uart.h
 *
 *  Created on: 09-Jul-2026
 *      Author: lenovo
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <string.h>

#define TX_BUFFER_SIZE 128

#define UART_BUFFER_SIZE 64

#define CMD_BUFFER_SIZE 32

void UART_Init(void);

void UART_SendChar(char ch);

void UART_SendString(char *str);

void UART_SendChar_IT(char ch);

void UART_SendString_IT(char *str);

char UART_BufferRead(void);

void USART2_IRQHandler(void);

uint8_t UART_Available(void);

char UART_Read(void);

uint8_t UART_GetORECount(void);

void UART_SendString_DMA(char *str);

#endif
