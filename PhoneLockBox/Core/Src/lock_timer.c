/*
 * lock.c
 *
 *  Created on: Apr 15, 2025
 *      Author: colinriker
 *
 *
 *	lock_timer:
 *		These functions handle the master timer system and the solinoid (the MOSfet)
 *		This is all done
 */
#include "lock_timer.h"

#include <stdbool.h>
#include <stdint.h>

#include "stm32l4xx_hal.h"

extern bool master_timer_done;
extern TIM_HandleTypeDef htim2;

uint32_t max_time_ms;

void lockTimerInit(void) {
	lockDisenage();
	lockTimerCancel();
	lockTimerSetTime(10000);
	master_timer_done = false;
}


void lockTimerStart(void) {
	max_time_ms = lockTimerGetTime();
	HAL_TIM_Base_Start_IT(&htim2);
	master_timer_done = false;
}

uint32_t lockTimerGetTime(void){
	return TIM2->CNT / 10;
}

void lockTimerSetTime(int32_t time) {
	if (time < 0)
		time = 0;

	TIM2->CNT = time * 10;
}

void lockTimerCancel(void) {
	HAL_TIM_Base_Stop_IT(&htim2);
}

void lockEngage(void) {
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, 1);
}
void lockDisenage(void) {
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, 0);
}
