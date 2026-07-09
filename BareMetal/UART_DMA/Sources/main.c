#include <stdio.h>
#include <stdint.h>
#include "stm32f407.h"
#include "uart.h"



volatile uint32_t cr2;
volatile uint32_t cr3;
volatile uint32_t apb1enr;
char cmd_buffer[CMD_BUFFER_SIZE];
uint8_t cmd_index = 0;
char msg[] = "Hello DMA\r\n";
uint8_t dma_sent = 0;

int main(void)
{


    RCC_APB1RSTR |=  (1 << 17);   // Reset USART2
    RCC_APB1RSTR &= ~(1 << 17);   // Release reset

    RCC_APB1ENR |= (1 << 17);     // Enable USART2 clock

    /* Enable Clocks */
    RCC_AHB1ENR |= (1 << 0);   // GPIOA
    RCC_AHB1ENR |= (1 << 3);   // GPIOD

    RCC_APB1ENR |= (1 << 17);     // USART2

    /* PA2 -> USART2_TX */
    GPIOA_MODER &= ~(3 << 4);
    GPIOA_MODER |=  (2 << 4);

    /* PA3 -> USART2_RX */
    GPIOA_MODER &= ~(3 << 6);
    GPIOA_MODER |=  (2 << 6);

    /* AF7 for PA2 */
    GPIOA_AFRL &= ~(0xF << 8);
    GPIOA_AFRL |=  (7 << 8);

    /* AF7 for PA3 */
    GPIOA_AFRL &= ~(0xF << 12);
    GPIOA_AFRL |=  (7 << 12);

    /* Pull-up on RX pin */
    GPIOA_PUPDR &= ~(3 << 6);
    GPIOA_PUPDR |=  (1 << 6);

    /* 115200 baud @16 MHz */
    USART2_BRR = 0x008B;

    USART2_CR1 = (1 << 13) |   // UE
                 (1 << 3)  |   // TE
                 (1 << 2)  |   // RE
                 (1 << 5);     // RXNEIE
    NVIC_ISER1 |= (1 << 6);

    cr2 = USART2_CR2;
    cr3 = USART2_CR3;
    apb1enr = RCC_APB1ENR;

    /* PD12 Output */
    GPIOD_MODER &= ~(3 << 24);
    GPIOD_MODER |=  (1 << 24);

    /* Enable SYSCFG clock */
    RCC_APB2ENR |= (1<<14);

    /* PA0 -> EXTI0 */
    SYSCFG_EXTICR1 &= ~(0xF<<0);

    /* Unmask EXTI0 */
    EXTI_IMR |= (1<<0);

    /* Rising edge */
    EXTI_RTSR |= (1<<0);

    /* Enable EXTI0 IRQ */
    NVIC_ISER0 |= (1<<6);


    /*-------------------------------------------------------
     * Enable DMA1 Stream6 Interrupt
     *------------------------------------------------------*/
    NVIC_ISER0 |= (1 << 17);

    UART_DMA_Init();
    //ore_count = 0;
    while(1)
    {

    	while(UART_Available())
        {


            char ch = UART_BufferRead();

            //UART_SendChar(ch);      // Echo

            if(ch != '\r' && ch != '\n')
            {
                cmd_buffer[cmd_index++] = ch;
            }
            else
            {
                cmd_buffer[cmd_index] = '\0';

                if(cmd_index == 0)
                {
                    continue;
                }
                if(UART_DMA_IsBusy())
                {
                    return 0;
                }

                if(strcmp(cmd_buffer,"led on") == 0)
                {
                    GPIOD_ODR |= (1 << 12);

                    //UART_SendString("\r\nLED ON\r\n");
                    //UART_SendString_IT("LED ON\r\n");
                    UART_SendString_DMA("LED ON\r\n");
                    //UART_SendString_IT("123456789\r\n");

                }else if(strcmp(cmd_buffer,"led off") == 0)
                {
                    GPIOD_ODR &= ~(1 << 12);

                    //UART_SendString_IT("\r\nLED OFF\r\n");

                    UART_SendString_DMA("\r\nLED OFF\r\n");
                }/* STATUS */
                else if(strcmp(cmd_buffer, "status") == 0)
                {
                	char status_msg[256];

                	if (GPIOD_ODR & (1 << 12))
                	{
                	    strcpy(status_msg,
                	        "\r\n========== SYSTEM STATUS ==========\r\n"
                	        "LED           : ON\r\n"
                	        "UART          : OK\r\n"
                	        "RX Buffer     : OK\r\n"
                	        "TX Buffer     : OK\r\n"
                	        );
                	}
                	else
                	{
                	    strcpy(status_msg,
                	        "\r\n========== SYSTEM STATUS ==========\r\n"
                	        "LED           : OFF\r\n"
                	        "UART          : OK\r\n"
                	        "RX Buffer     : OK\r\n"
                	        "TX Buffer     : OK\r\n"
                	        );
                	}



                    /* ORE Count */
                	if(UART_GetORECount() == 0)
                	{
                	    strcat(status_msg,"Overrun Error : NONE\r\n");
                	}
                	else
                	{
                	    strcat(status_msg,"Overrun Error : DETECTED\r\n");
                	}
                	char temp[32];

                	sprintf(temp, "ORE Count     : %d\r\n", UART_GetORECount());

                	strcat(status_msg, temp);

                	strcat(status_msg,
                	       "===================================\r\n");

                    //UART_SendString_IT(status_msg);
                    UART_SendString_DMA(status_msg);
                    //UART_SendString_IT(
                    //    "===================================\r\n");
                }

                /* HELP */
                else if(strcmp(cmd_buffer, "help") == 0)
                {
					#if 0
                	UART_SendString_IT("\r\nAvailable Commands\r\n");
                	UART_SendString_IT("------------------\r\n");
                	UART_SendString_IT("help\r\n");
                	UART_SendString_IT("led on\r\n");
                	UART_SendString_IT("led off\r\n");
                	UART_SendString_IT("status\r\n");
					#endif

                	char help_msg[] =
                	"\r\nAvailable Commands\r\n"
                	"------------------\r\n"
                	"help\r\n"
                	"led on\r\n"
                	"led off\r\n"
                	"status\r\n";

                	UART_SendString_DMA(help_msg);
                }

                /* Unknown command */
                else
                {
                	UART_SendString_IT("\r\nUnknown Command\r\n");
                	UART_SendString_IT("Type 'help'\r\n");
                }

                cmd_index = 0;
            }
        }
    }
}

