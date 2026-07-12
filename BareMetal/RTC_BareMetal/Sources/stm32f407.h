#ifndef STM32F407_H
#define STM32F407_H

#include <stdint.h>

/*=========================================================
 * Base Addresses
 *========================================================*/
#define RCC_BASE        0x40023800U
#define GPIOA_BASE      0x40020000U
#define GPIOD_BASE      0x40020C00U
#define USART2_BASE     0x40004400U
#define DMA1_BASE       0x40026000U
#define EXTI_BASE       0x40013C00U
#define SYSCFG_BASE     0x40013800U
#define NVIC_ISER0_BASE 0xE000E100U
#define NVIC_ISER1_BASE 0xE000E104U

/*=========================================================
 * RCC Registers
 *========================================================*/
#define RCC_AHB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x40))
#define RCC_APB1RSTR    (*(volatile uint32_t *)(RCC_BASE + 0x20))
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x44))

/* RCC additional flat registers (same convention) */
#define RCC_BDCR        (*(volatile uint32_t *)(RCC_BASE + 0x70))
#define RCC_CSR         (*(volatile uint32_t *)(RCC_BASE + 0x74))

#define RCC_APB1ENR_PWREN   (1U << 28)
#define RCC_BDCR_LSEON      (1U << 0)
#define RCC_BDCR_LSERDY     (1U << 1)
#define RCC_BDCR_RTCSEL_0   (1U << 8)
#define RCC_BDCR_RTCSEL_1   (1U << 9)
#define RCC_BDCR_RTCEN      (1U << 15)
#define RCC_CSR_LSION       (1U << 0)
#define RCC_CSR_LSIRDY      (1U << 1)

/*=========================================================
 * RCC APB2ENR - SYSCFG enable
 *========================================================*/
#define RCC_APB2ENR_SYSCFGEN  (1U << 14)

/*=========================================================
 * RTC Alarm A bits
 *========================================================*/
#define RTC_CR_ALRAE     (1U << 8)
#define RTC_CR_ALRAIE    (1U << 12)
#define RTC_ISR_ALRAWF   (1U << 0)
#define RTC_ISR_ALRAF    (1U << 8)

/*=========================================================
 * IRQ Numbers (for NVIC)
 *========================================================*/
#define EXTI0_IRQn       6
#define RTC_Alarm_IRQn   41

/* ---------------- PWR (struct style) ---------------- */
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t CSR;
} PWR_TypeDef;

#define PWR_BASE   0x40007000UL
#define PWR        ((PWR_TypeDef *)PWR_BASE)

#define PWR_CR_DBP (1U << 8)

/* ---------------- RTC (struct style) ---------------- */
typedef struct {
    volatile uint32_t TR;
    volatile uint32_t DR;
    volatile uint32_t CR;
    volatile uint32_t ISR;
    volatile uint32_t PRER;
    volatile uint32_t WUTR;
    volatile uint32_t CALIBR;
    volatile uint32_t ALRMAR;
    volatile uint32_t ALRMBR;
    volatile uint32_t WPR;
    volatile uint32_t SSR;
    volatile uint32_t SHIFTR;
    volatile uint32_t TSTR;
    volatile uint32_t TSDR;
    volatile uint32_t TSSSR;
    volatile uint32_t CALR;
    volatile uint32_t TAFCR;
    volatile uint32_t ALRMASSR;
    volatile uint32_t ALRMBSSR;
    uint32_t RESERVED;
    volatile uint32_t BKP0R;
} RTC_TypeDef;

#define RTC_BASE   0x40002800UL
#define RTC        ((RTC_TypeDef *)RTC_BASE)

#define RTC_ISR_INIT    (1U << 7)
#define RTC_ISR_INITF   (1U << 6)

/*=========================================================
 * GPIOA Registers
 *========================================================*/
#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_PUPDR     (*(volatile uint32_t *)(GPIOA_BASE + 0x0C))
#define GPIOA_AFRL      (*(volatile uint32_t *)(GPIOA_BASE + 0x20))

/*=========================================================
 * GPIOD Registers
 *========================================================*/
#define GPIOD_MODER     (*(volatile uint32_t *)(GPIOD_BASE + 0x00))
#define GPIOD_ODR       (*(volatile uint32_t *)(GPIOD_BASE + 0x14))

/*=========================================================
 * USART2 Registers
 *========================================================*/
#define USART2_SR       (*(volatile uint32_t *)(USART2_BASE + 0x00))
#define USART2_DR       (*(volatile uint32_t *)(USART2_BASE + 0x04))
#define USART2_BRR      (*(volatile uint32_t *)(USART2_BASE + 0x08))
#define USART2_CR1      (*(volatile uint32_t *)(USART2_BASE + 0x0C))
#define USART2_CR2      (*(volatile uint32_t *)(USART2_BASE + 0x10))
#define USART2_CR3      (*(volatile uint32_t *)(USART2_BASE + 0x14))

/*=========================================================
 * USART CR1 Bit Definitions
 *========================================================*/
#define USART_CR1_UE        (1U << 13)
#define USART_CR1_TE        (1U << 3)
#define USART_CR1_RE        (1U << 2)
#define USART_CR1_IDLEIE    (1U << 4)
#define USART_CR1_RXNEIE    (1U << 5)
#define USART_CR1_TXEIE     (1U << 7)

/*=========================================================
 * USART SR Bit Definitions
 *========================================================*/
#define USART_SR_RXNE       (1U << 5)
#define USART_SR_TXE        (1U << 7)
#define USART_SR_IDLE       (1U << 4)
#define USART_SR_ORE        (1U << 3)

/*=========================================================
 * DMA1 Global Registers
 *========================================================*/
#define DMA1_HISR       (*(volatile uint32_t *)(DMA1_BASE + 0x04))
#define DMA1_HIFCR      (*(volatile uint32_t *)(DMA1_BASE + 0x0C))

/*=========================================================
 * DMA1 Stream5 (USART2 RX)
 *========================================================*/
#define DMA1_S5CR       (*(volatile uint32_t *)(DMA1_BASE + 0x88))
#define DMA1_S5NDTR     (*(volatile uint32_t *)(DMA1_BASE + 0x8C))
#define DMA1_S5PAR      (*(volatile uint32_t *)(DMA1_BASE + 0x90))
#define DMA1_S5M0AR     (*(volatile uint32_t *)(DMA1_BASE + 0x94))
#define DMA1_S5FCR      (*(volatile uint32_t *)(DMA1_BASE + 0x98))

/*=========================================================
 * DMA1 Stream6 (USART2 TX)
 *========================================================*/
#define DMA1_S6CR       (*(volatile uint32_t *)(DMA1_BASE + 0xA0))
#define DMA1_S6NDTR     (*(volatile uint32_t *)(DMA1_BASE + 0xA4))
#define DMA1_S6PAR      (*(volatile uint32_t *)(DMA1_BASE + 0xA8))
#define DMA1_S6M0AR     (*(volatile uint32_t *)(DMA1_BASE + 0xAC))
#define DMA1_S6FCR      (*(volatile uint32_t *)(DMA1_BASE + 0xB0))

/*=========================================================
 * DMA Stream5 Flags (RX)
 *========================================================*/
#define DMA_TCIF5       (1U << 11)
#define DMA_CTCIF5      (1U << 11)

/*=========================================================
 * DMA Stream6 Flags (TX)
 *========================================================*/
#define DMA_TCIF6       (1U << 21)
#define DMA_CTCIF6      (1U << 21)

/*=========================================================
 * EXTI Registers
 *========================================================*/
#define EXTI_IMR        (*(volatile uint32_t *)(EXTI_BASE + 0x00))
#define EXTI_RTSR       (*(volatile uint32_t *)(EXTI_BASE + 0x08))
#define EXTI_PR         (*(volatile uint32_t *)(EXTI_BASE + 0x14))

/*=========================================================
 * SYSCFG Registers
 *========================================================*/
#define SYSCFG_EXTICR1  (*(volatile uint32_t *)(SYSCFG_BASE + 0x08))

/*=========================================================
 * NVIC Registers
 *========================================================*/
#define NVIC_ISER0      (*(volatile uint32_t *)(NVIC_ISER0_BASE))
#define NVIC_ISER1      (*(volatile uint32_t *)(NVIC_ISER1_BASE))

typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_TypeDef;

#define SysTick_BASE  0xE000E010UL
#define SysTick       ((SysTick_TypeDef *)SysTick_BASE)

#define SysTick_CTRL_ENABLE    (1U << 0)
#define SysTick_CTRL_TICKINT   (1U << 1)
#define SysTick_CTRL_CLKSOURCE (1U << 2)

#endif /* STM32F407_H */
