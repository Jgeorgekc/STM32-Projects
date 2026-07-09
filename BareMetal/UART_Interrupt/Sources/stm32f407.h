/*
 * stm32f407.h
 *
 *  Created on: 09-Jul-2026
 *      Author: lenovo
 */


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
