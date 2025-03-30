/*
 * accelerometer.h
 *
 *  Created on: Mar 28, 2025
 *      Author: colinriker
 */

#ifndef INC_ACCELEROMETER_H_
#define INC_ACCELEROMETER_H_

#include <stdint.h>

#include "stm32l4xx_hal.h"
#include "shared.h"

#define SAD_W_M 0x3C
#define SAD_R_M 0x3D

#define CTRL_REG1_A 0x20
#define CRA_REG_M 0x00
#define ACC_FIRST_ADDR 0x28
#define MAG_FIRST_ADDR 0x03

#define ACC_READ 0x33
#define ACC_WRITE 0x32
#define MAG_READ 0x3D
#define MAG_WRITE 0x3C

#define IRA_REG_M 0x0A

extern BoxState state;
extern BoxState next_state;
extern I2C_HandleTypeDef hi2c1;

void accInit(void);
void accRead(int16_t * x_axis,int16_t * y_axis,int16_t * z_axis);
void accResolve();
void accCheck(void);

void magInit(void);
void magRead(int16_t * x_mag,int16_t * y_mag,int16_t * z_mag);
void magResolve(void);

#endif /* INC_ACCELEROMETER_H_ */
