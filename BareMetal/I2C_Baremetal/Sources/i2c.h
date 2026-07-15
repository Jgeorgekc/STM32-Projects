/*
 * I2C.h
 *
 *  Created on: 13-Jul-2026
 *      Author: lenovo
 */

#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>
void I2C1_Init(void);

void I2C_Start(void);
void I2C_Stop(void);

void I2C_SendAddress(uint8_t address);
void I2C_WriteByte(uint8_t data);

#endif
