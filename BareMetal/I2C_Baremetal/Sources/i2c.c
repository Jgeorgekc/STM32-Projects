/*
 * I2C.c
 *
 *  Created on: 13-Jul-2026
 *      Author: lenovo
 */


#include "stm32f407.h"
#include "i2c.h"

void I2C1_Init(void)
{
    /* Enable GPIOB Clock */
    RCC_AHB1ENR |= (1 << 1);

    /* Enable I2C1 Clock */
    RCC_APB1ENR |= (1 << 21);

    /* PB6 PB7 Alternate Function */
    GPIOB_MODER &= ~(0xF << 12);
    GPIOB_MODER |=  (0xA << 12);

    /* Open Drain */
    GPIOB_OTYPER |= (1 << 6);
    GPIOB_OTYPER |= (1 << 7);

    /* High Speed */
    GPIOB_OSPEEDR |= (0xF << 12);

    /* Pull Up */
    GPIOB_PUPDR &= ~(0xF << 12);
    GPIOB_PUPDR |=  (0x5 << 12);

    /* AF4 */
    GPIOB_AFRL &= ~(0xFF << 24);
    GPIOB_AFRL |=  (0x44 << 24);

    /* Reset I2C1 Peripheral */
    RCC_APB1RSTR |= (1 << 21);
    RCC_APB1RSTR &= ~(1 << 21);

    /* Reset I2C */
    I2C1_CR1 = 0;

    /* APB1 Clock = 16MHz */
    I2C1_CR2 = 16;

    /* Standard Mode 100kHz */
    I2C1_CCR = 80;

    /* Maximum Rise Time */
    I2C1_TRISE = 17;

    /* Enable Peripheral */
    I2C1_CR1 |= 1;

    I2C1_CR1 |= (1 << 10);   // ACK enable
}

void I2C_Start(void)
{
    /* Wait until bus is free */
    while(I2C1_SR2 & (1 << 1));

    /* Generate START */
    I2C1_CR1 |= (1 << 8);

    /* Wait for SB bit */
    while(!(I2C1_SR1 & (1 << 0)));
}

void I2C_Stop(void)
{
    I2C1_CR1 |= (1 << 9);
}

void I2C_WriteByte(uint8_t data)
{
    /* Wait until TXE */
    while(!(I2C1_SR1 & (1 << 7)));

    I2C1_DR = data;

    /* Wait until byte transfer finished */
    while(!(I2C1_SR1 & (1 << 2)));
}

void I2C_SendAddress(uint8_t address)
{
    /* Send Slave Address */
    I2C1_DR = address;

    /* Wait until ADDR flag is set */
    while(!(I2C1_SR1 & (1 << 1)));

    /* Clear ADDR flag */
    (void)I2C1_SR1;
    (void)I2C1_SR2;
}
