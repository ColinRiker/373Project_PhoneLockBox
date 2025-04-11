/*
 * accelerometer.h
 *
 *  Created on: Mar 28, 2025
 *      Author: colinriker
 */

#ifndef INC_ACCELEROMETER_H_
#define INC_ACCELEROMETER_H_

#include <stdint.h>
#include <stdbool.h>

#include "stm32l4xx_hal.h"
#include "shared.h"

#define ACCELERATION_WAKE_DELTA 1000

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
extern Vector3D accelerometer_state;
extern Vector3D prev_accelerometer_state;
extern Vector3D magnometer_state;
extern Vector3D prev_magnometer_state;

void accInit(void);
void accRead(void);
bool accHasMoved();
void accCheck(void);

void magInit(void);
void magRead(Vector3D* vec);
bool magIsClosed(void);
BoxMode magResolve(void);

#endif /* INC_ACCELEROMETER_H_ */
