/*
 * lock.c
 *
 *  Created on: Apr 15, 2025
 *      Author: colinriker
 */
#include "lock_timer.h"

#include <stdbool.h>
#include <stdint.h>

#include "stm32l4xx_hal.h"

extern bool master_set_done;
extern TIM_HandleTypeDef htim2;


void lockTimerStartEvent(void) {
	HAL_TIM_Base_Start_IT(&htim2);
	master_set_done = false;
}

uint32_t lockTimerGetTime(void){
	return TIM2->CNT;
}

void lockTimerSetTime(uint32_t time) {
	TIM2->CNT = time;
}

void lockTimerCancel(void) {
	HAL_TIM_Base_Stop_IT(&htim2);
}
