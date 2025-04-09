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

#include "accelerometer.h"
#include "stm32l4xx_hal.h"
#include "shared.h"

extern Vector3D accelerometer_state;
extern Vector3D prev_accelerometer_state;
extern Vector3D magnometer_state;
extern Vector3D prev_magnometer_state;

void accCheck(void){
	uint8_t buf[10]= {IRA_REG_M};
	HAL_I2C_Master_Transmit(&hi2c1, SAD_W_M, &buf[0], 1, 1000);
	uint8_t out_buf[10] = {};
	HAL_I2C_Master_Receive(&hi2c1, SAD_R_M, &out_buf[0], 1, 1000);
}


void accInit(void){
	uint8_t buf[10]= {CTRL_REG1_A,0x97};

	accelerometer_state.x_componenet = 0;
	accelerometer_state.y_componenet = 0;
	accelerometer_state.z_componenet = 0;

	prev_accelerometer_state = accelerometer_state;

	HAL_I2C_Master_Transmit(&hi2c1, ACC_WRITE, &buf[0], 2, 1000);
}


void accRead(){

	uint8_t buf[10]= {ACC_FIRST_ADDR | (1 << 7)};
	HAL_I2C_Master_Transmit(&hi2c1, ACC_WRITE, &buf[0], 1, 1000);

	uint8_t out_buf_8[6] = {};
	HAL_I2C_Master_Receive(&hi2c1, ACC_READ, &out_buf_8[0], 6, 1000);

	accelerometer_state.x_componenet = (out_buf_8[1] << 8) | out_buf_8[0];
	accelerometer_state.y_componenet =(out_buf_8[3] << 8) | out_buf_8[2];
	accelerometer_state.z_componenet =(out_buf_8[5] << 8) | out_buf_8[4];
}

bool accHasMoved() {
	return VectorDelta(&accelerometer_state, &prev_accelerometer_state) >= ACCELERATION_WAKE_DELTA;
}

void magInit(){
	magnometer_state.x_componenet = 0;
	magnometer_state.y_componenet = 0;
	magnometer_state.z_componenet = 0;

	//bin is setting to 3.0 hz
	uint8_t buf[10]= {CRA_REG_M | (1 << 7),0b00001000,0b01100000,0b00000000};
	HAL_I2C_Master_Transmit(&hi2c1, MAG_WRITE, &buf[0], 4, 1000);
}



void magRead(Vector3D* vec){

	uint8_t buf[10]= {MAG_FIRST_ADDR | (1 << 7)};
	HAL_I2C_Master_Transmit(&hi2c1, MAG_WRITE, &buf[0], 1, 1000);

	uint8_t out_buf_8[6] = {};
	HAL_I2C_Master_Receive(&hi2c1, MAG_READ, &out_buf_8[0], 6, 1000);


	vec->x_componenet = (out_buf_8[1] << 8) | out_buf_8[0];
	vec->y_componenet =	(out_buf_8[3] << 8) | out_buf_8[2];
	vec->z_componenet = (out_buf_8[5] << 8) | out_buf_8[4];
}

void magIsClosed(void) {

}
