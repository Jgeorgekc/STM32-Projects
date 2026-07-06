#include <stdint.h>

/* RCC */
#define RCC_AHB1ENR   (*(volatile uint32_t *)0x40023830)
#define RCC_APB1ENR   (*(volatile uint32_t *)0x40023840)

/* GPIOA */
#define GPIOA_MODER   (*(volatile uint32_t *)0x40020000)
#define GPIOA_PUPDR   (*(volatile uint32_t *)0x4002000C)
#define GPIOA_AFRL    (*(volatile uint32_t *)0x40020020)

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

void UART_SendChar(char ch);
void UART_SendString(char *str);
char UART_ReadChar(void);

void delay(volatile uint32_t count)
{
    while(count--);
}

volatile uint32_t cr2;
volatile uint32_t cr3;
volatile uint32_t apb1enr;

volatile char rx;
volatile int x = 0;

int main(void)
{


    RCC_APB1RSTR |=  (1 << 17);   // Reset USART2
    RCC_APB1RSTR &= ~(1 << 17);   // Release reset

    RCC_APB1ENR |= (1 << 17);     // Enable USART2 clock

    /* Enable Clocks */
    RCC_AHB1ENR |= (1 << 0);      // GPIOA
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


    while (1)
    {
        //UART_SendChar('A');

        delay(500000);
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

    if (USART2_SR & (1 << 5))
    {
        rx = USART2_DR;

        UART_SendChar(rx);

        if(rx == 'A')
        {
            x++;
        }
    }
}

void EXTI0_IRQHandler(void)
{
    if (EXTI_PR & (1<<0))
    {
        EXTI_PR |= (1<<0);

        UART_SendString("Button Pressed\r\n");
    }
}
