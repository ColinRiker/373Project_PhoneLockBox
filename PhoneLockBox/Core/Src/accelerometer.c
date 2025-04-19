/*
 * accelerometer.c
 *
 *  Created on: Mar 28, 2025
 *      Author: colinriker
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "accelerometer.h"
#include "stm32l4xx_hal.h"
#include "shared.h"
#include "state_machine.h"

extern I2C_HandleTypeDef hi2c1;

Vector3D accelerometer_state;
Vector3D prev_accelerometer_state;

void accInit(void){
	uint8_t buf[10]= {CTRL_REG1_A,0x97};

	accelerometer_state.x_componenet = 0;
	accelerometer_state.y_componenet = 0;
	accelerometer_state.z_componenet = 0;



	if(HAL_I2C_Master_Transmit(&hi2c1, ACC_WRITE, &buf[0], 2, I2C_TIMEOUT) != HAL_OK) {
#ifdef DEBUG_ACC_MAG
		printf("[ERROR] Accelerometer initialization I2C transmit failed\n\r");
#endif
	}

	accRead();
	prev_accelerometer_state = accelerometer_state;
}


void accRead(){

	uint8_t buf[10]= {ACC_FIRST_ADDR | (1 << 7)};
	if (HAL_I2C_Master_Transmit(&hi2c1, ACC_WRITE, &buf[0], 1, I2C_TIMEOUT) != HAL_OK) {
#ifdef DEBUG_ACC_MAG
		printf("[ERROR] Accelerometer Data I2C transmit failed\n\r");
#endif
	}

	uint8_t out_buf_8[6] = {};
	if (HAL_I2C_Master_Receive(&hi2c1, ACC_READ, &out_buf_8[0], 6, I2C_TIMEOUT) != HAL_OK) {
#ifdef DEBUG_ACC_MAG
		printf("[ERROR] Accelerometer Data I2C receive failed\n\r");
#endif
	}
	prev_accelerometer_state = accelerometer_state;

	accelerometer_state.x_componenet = (out_buf_8[1] << 8) | out_buf_8[0];
	accelerometer_state.y_componenet = (out_buf_8[3] << 8) | out_buf_8[2];
	accelerometer_state.z_componenet = (out_buf_8[5] << 8) | out_buf_8[4];

#ifdef DEBUG_ACC_MAG
		printf("[INFO] Accelerometer Read result, x: %d, y: %d, z: %d\n\r",
				accelerometer_state.x_componenet, accelerometer_state.y_componenet, accelerometer_state.z_componenet);
#endif
}

void accDeltaEvent(void) {
	accRead();  // updates current_state and last_state

	int32_t delta = (accelerometer_state.x_componenet - prev_accelerometer_state.x_componenet) +
			(accelerometer_state.y_componenet - prev_accelerometer_state.y_componenet) +
			(accelerometer_state.z_componenet - prev_accelerometer_state.z_componenet);
#ifdef DEBUG_ACC_MAG
	printf("[INFO] Accelerometer Delta: %d\n\r", delta);
#endif
	if (abs(delta) >= ACCELERATION_WAKE_DELTA) {
		stateInsertFlag(SFLAG_ACC_BOX_MOVED);
#ifdef DEBUG_ACC_MAG
		printf("[INFO] Accelerometer detected the box has moved, flag inserted\n\r");
#endif
	}
}

void magInit(){
	uint8_t buf[10]= {CRA_REG_M | (1 << 7),0b00001000,0b01100000,0b00000000};
	if (HAL_I2C_Master_Transmit(&hi2c1, MAG_WRITE, &buf[0], 4, I2C_TIMEOUT) != HAL_OK) {
#ifdef DEBUG_ACC_MAG
		printf("[ERROR] Magnetometer initialization I2C transmit failed\n\r");
#endif
	}
}



void magRead(Vector3D* vec){
	uint8_t buf[10]= {MAG_FIRST_ADDR | (1 << 7)};
	if (HAL_I2C_Master_Transmit(&hi2c1, MAG_WRITE, &buf[0], 1, I2C_TIMEOUT) != HAL_OK) {
#ifdef DEBUG_ACC_MAG
		printf("[ERROR] Magnetometer Data I2C transmit failed\n\r");
#endif
	}

	uint8_t out_buf_8[6] = {};
	if (HAL_I2C_Master_Receive(&hi2c1, MAG_READ, &out_buf_8[0], 6, I2C_TIMEOUT) != HAL_OK) {
#ifdef DEBUG_ACC_MAG
		printf("[ERROR] Magnetometer Data I2C receive failed\n\r");
#endif
	}


	vec->x_componenet = (out_buf_8[1] << 8) | out_buf_8[0];
	vec->y_componenet =	(out_buf_8[3] << 8) | out_buf_8[2];
	vec->z_componenet = (out_buf_8[5] << 8) | out_buf_8[4];

#ifdef DEBUG_ACC_MAG
		printf("[INFO] Magnetometer Read result, x: %d, y: %d, z: %d\n\r",
				vec->x_componenet, vec->y_componenet, vec->z_componenet);
#endif
}



void magBoxStatusEvent(void) {
	Vector3D vec;
	magRead(&vec);

	int magnitude = abs(vec.x_componenet) + abs(vec.y_componenet) + abs(vec.z_componenet);

	if (magnitude > MAGNOMETER_THRESHOLD) {  // not sure what this value needs to be
		stateInsertFlag(SFLAG_BOX_CLOSED);
#ifdef DEBUG_ACC_MAG
		printf("[INFO] Magnetometer detected the box is closed, flag inserted\n\r");
#endif

	} else {
		stateInsertFlag(SFLAG_BOX_OPEN);
#ifdef DEBUG_ACC_MAG
		printf("[INFO] Magnetometer detected the box is open, flag inserted\n\r");
#endif

	}
}
