/*
 * cli.c
 *
 *  Created on: 09-Jul-2026
 *      Author: lenovo
 */


#include "cli.h"
#include "uart.h"
#include "stm32f407.h"

#include <string.h>
#include <stdio.h>

/*=========================================================
 * Private Variables
 *========================================================*/

static char cmd_buffer[CMD_BUFFER_SIZE];
static uint8_t cmd_index = 0;

/*=========================================================
 * Private Function Prototypes
 *========================================================*/

static void CLI_ProcessCommand(char *cmd);
static void CMD_Help(void);
static void CMD_Status(void);
static void CMD_LedOn(void);
static void CMD_LedOff(void);
static void CMD_Unknown(void);

extern volatile uint32_t dma_irq_count;

void CLI_ProcessByte(char ch)
{
    if(ch != '\r' && ch != '\n')
    {
        if(cmd_index < CMD_BUFFER_SIZE-1)
        {
            cmd_buffer[cmd_index++] = ch;
        }
    }
    else
    {
        cmd_buffer[cmd_index] = '\0';

        if(cmd_index)
        {
            CLI_ProcessCommand(cmd_buffer);
        }

        cmd_index = 0;
    }
}

static void CLI_ProcessCommand(char *cmd)
{
    if(strcmp(cmd,"help")==0)
    {
        CMD_Help();
    }
    else if(strcmp(cmd,"status")==0)
    {
        CMD_Status();
    }
    else if(strcmp(cmd,"led on")==0)
    {
        CMD_LedOn();
    }
    else if(strcmp(cmd,"led off")==0)
    {
        CMD_LedOff();
    }
    else
    {
        CMD_Unknown();
    }
}

static void CMD_LedOn(void)
{


    GPIOD_ODR |= (1U << 12);

    UART_SendString_DMA("LED ON\r\n");
}

static void CMD_LedOff(void)
{
    GPIOD_ODR &= ~(1U<<12);

    UART_SendString_DMA("LED OFF\r\n");
}

static const char help_msg[] =
"\r\nAvailable Commands\r\n"
"------------------\r\n"
"help\r\n"
"led on\r\n"
"led off\r\n"
"status\r\n";

static void CMD_Help(void)
{
    UART_SendString_DMA((char *)help_msg);
}

static void CMD_Status(void)
{
    static char status_msg[256];

    sprintf(status_msg,
            "\r\n========== SYSTEM STATUS ==========\r\n"
            "LED           : %s\r\n"
            "UART          : OK\r\n"
            "RX Buffer     : OK\r\n"
            "TX Buffer     : OK\r\n"
            "ORE Count     : %d\r\n"
            "DMA HISR : 0x%08lX\r\n"
            "===================================\r\n",
            (GPIOD_ODR & (1U << 12)) ? "ON" : "OFF",
            UART_GetORECount(),
            dma_irq_count);



    UART_SendString_DMA(status_msg);
}
static void CMD_Unknown(void)
{
    UART_SendString_DMA("\r\nUnknown Command\r\n");
}
