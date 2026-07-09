#include <stdint.h>
#include <string.h>

/* RCC */
#define RCC_AHB1ENR   (*(volatile uint32_t *)0x40023830)
#define RCC_APB1ENR   (*(volatile uint32_t *)0x40023840)

/* GPIOA */
#define GPIOA_MODER   (*(volatile uint32_t *)0x40020000)
#define GPIOA_PUPDR   (*(volatile uint32_t *)0x4002000C)
#define GPIOA_AFRL    (*(volatile uint32_t *)0x40020020)

/* GPIOD */
#define GPIOD_MODER   (*(volatile uint32_t *)0x40020C00)
#define GPIOD_ODR     (*(volatile uint32_t *)0x40020C14)

/* USART2 */
#define USART2_SR     (*(volatile uint32_t *)0x40004400)
#define USART2_DR     (*(volatile uint32_t *)0x40004404)
#define USART2_BRR    (*(volatile uint32_t *)0x40004408)
#define USART2_CR1    (*(volatile uint32_t *)0x4000440C)

#define RCC_APB1RSTR (*(volatile uint32_t *)0x40023820)


#define USART2_CR2 (*(volatile uint32_t *)0x40004410)
#define USART2_CR3 (*(volatile uint32_t *)0x40004414)

#define NVIC_ISER1 (*(volatile uint32_t *)0xE000E104)

#define RCC_APB2ENR      (*(volatile uint32_t *)0x40023844)

#define SYSCFG_EXTICR1   (*(volatile uint32_t *)0x40013808)

#define EXTI_IMR         (*(volatile uint32_t *)0x40013C00)
#define EXTI_RTSR        (*(volatile uint32_t *)0x40013C08)
#define EXTI_PR          (*(volatile uint32_t *)0x40013C14)

#define NVIC_ISER0       (*(volatile uint32_t *)0xE000E100)

#define TX_BUFFER_SIZE 128

volatile char tx_buffer[TX_BUFFER_SIZE];

volatile uint8_t tx_head = 0;
volatile uint8_t tx_tail = 0;

#define UART_BUFFER_SIZE 64

#define CMD_BUFFER_SIZE 32

char cmd_buffer[CMD_BUFFER_SIZE];

uint8_t cmd_index = 0;


volatile char uart_buffer[UART_BUFFER_SIZE];

volatile uint8_t head = 0;
volatile uint8_t tail = 0;

void UART_SendChar(char ch);
void UART_SendString(char *str);
char UART_ReadChar(void);
char UART_BufferRead(void);
void UART_SendString_IT(char *str);
void UART_SendChar_IT(char ch);

void delay(volatile uint32_t count)
{
    while(count--);
}

volatile uint32_t cr2;
volatile uint32_t cr3;
volatile uint32_t apb1enr;

volatile char rx;
volatile int x = 0;
volatile uint32_t uart_sr = 0;
volatile uint8_t ore_count = 0;
volatile uint8_t tx_busy = 0;

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


    while(1)
    {
        if(head != tail)
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

                if(strcmp(cmd_buffer,"led on") == 0)
                {
                    GPIOD_ODR |= (1 << 12);

                    //UART_SendString("\r\nLED ON\r\n");
                    UART_SendString_IT("LED ON\r\n");
                    //UART_SendString_IT("123456789\r\n");

                }else if(strcmp(cmd_buffer,"led off") == 0)
                {
                    GPIOD_ODR &= ~(1 << 12);

                    UART_SendString_IT("\r\nLED OFF\r\n");
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
                	if(ore_count == 0)
                	{
                	    strcat(status_msg,"Overrun Error : NONE\r\n");
                	}
                	else
                	{
                	    strcat(status_msg,"Overrun Error : DETECTED\r\n");
                	}

                	strcat(status_msg,
                	       "===================================\r\n");

                    UART_SendString_IT(status_msg);
                    //UART_SendString_IT(
                    //    "===================================\r\n");
                }

                /* HELP */
                else if(strcmp(cmd_buffer, "help") == 0)
                {
                	UART_SendString_IT("\r\nAvailable Commands\r\n");
                	UART_SendString_IT("------------------\r\n");
                	UART_SendString_IT("help\r\n");
                	UART_SendString_IT("led on\r\n");
                	UART_SendString_IT("led off\r\n");
                	UART_SendString_IT("status\r\n");
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
