#ifndef UART_H
#define UART_H

#include <stdint.h>

/*=========================================================
 * Buffer Sizes
 *========================================================*/
#define TX_BUFFER_SIZE        128
#define UART_BUFFER_SIZE       64
#define DMA_RX_BUFFER_SIZE     64
#define CMD_BUFFER_SIZE        32

/*=========================================================
 * UART Initialization
 *========================================================*/
void UART_Init(void);

/*=========================================================
 * UART Polling APIs
 *========================================================*/
void UART_SendChar(char ch);
void UART_SendString(char *str);
char UART_ReadChar(void);

/*=========================================================
 * UART Interrupt APIs
 *========================================================*/
void UART_SendChar_IT(char ch);
void UART_SendString_IT(char *str);

/*=========================================================
 * UART DMA APIs
 *========================================================*/
void UART_DMA_Init(void);
void UART_DMA_RX_Init(void);
void UART_SendString_DMA(char *str);

/*=========================================================
 * UART Receive APIs
 *========================================================*/
uint8_t UART_Available(void);
char UART_BufferRead(void);

/*=========================================================
 * UART Status APIs
 *========================================================*/
uint8_t UART_GetORECount(void);

#endif /* UART_H */
