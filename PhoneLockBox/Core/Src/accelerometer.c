/*
 * accelerometer.c
 *
 *  Created on: Mar 28, 2025
 *      Author: colinriker
 */

#include "accelerometer.h"
#include <math.h>
#include <stdint.h>
#include "stm32l4xx_hal.h"
#include <stdio.h>



void accCheck(void){
	uint8_t buf[10]= {IRA_REG_M};
	HAL_I2C_Master_Transmit(&hi2c1, SAD_W_M, &buf[0], 1, 1000);
	uint8_t out_buf[10] = {};
	HAL_I2C_Master_Receive(&hi2c1, SAD_R_M, &out_buf[0], 1, 1000);
}


void accInit(void){
	uint8_t buf[10]= {CTRL_REG1_A,0x97};
	HAL_I2C_Master_Transmit(&hi2c1, ACC_WRITE, &buf[0], 2, 1000);
}


void accRead(int16_t * x_axis,int16_t * y_axis,int16_t * z_axis){

  uint8_t buf[10]= {ACC_FIRST_ADDR | (1 << 7)};
  HAL_I2C_Master_Transmit(&hi2c1, ACC_WRITE, &buf[0], 1, 1000);

  uint8_t out_buf_8[6] = {};
  HAL_I2C_Master_Receive(&hi2c1, ACC_READ, &out_buf_8[0], 6, 1000);


  *x_axis = (out_buf_8[1] << 8) | out_buf_8[0];
  *y_axis =	(out_buf_8[3] << 8) | out_buf_8[2];
  *z_axis = (out_buf_8[5] << 8) | out_buf_8[4];
}




void magInit(){
	//bin is setting to 3.0 hz
	uint8_t buf[10]= {CRA_REG_M | (1 << 7),0b00001000,0b01100000,0b00000000};
	HAL_I2C_Master_Transmit(&hi2c1, MAG_WRITE, &buf[0], 4, 1000);
}



void magRead(int16_t * x_mag,int16_t * y_mag,int16_t * z_mag){

  uint8_t buf[10]= {MAG_FIRST_ADDR | (1 << 7)};
  HAL_I2C_Master_Transmit(&hi2c1, MAG_WRITE, &buf[0], 1, 1000);

  uint8_t out_buf_8[6] = {};
  HAL_I2C_Master_Receive(&hi2c1, MAG_READ, &out_buf_8[0], 6, 1000);


  *x_mag = (out_buf_8[1] << 8) | out_buf_8[0];
  *y_mag =	(out_buf_8[3] << 8) | out_buf_8[2];
  *z_mag = (out_buf_8[5] << 8) | out_buf_8[4];
}
