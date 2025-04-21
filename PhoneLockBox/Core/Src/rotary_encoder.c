/*
 * rotary_encoder.c
 *
 *  Created on: Mar 19, 2025
 *      Author: colinriker
 */

#include "rotary_encoder.h"

#include <stdint.h>
#include <stdio.h>

#include "shared.h"
#include "state_machine.h"
#include "stm32l4xx_hal.h"

extern TIM_HandleTypeDef htim1;
extern SFlag flags[MAX_FLAGS];

int32_t prev_cnt;

void rotencInit(void) {
	TIM1->CNT = 30000;
	prev_cnt = TIM1->CNT;
#ifdef DEBUG_ROTARY_ENCODER
	if (HAL_TIM_Encoder_Start_IT(&htim1, TIM_CHANNEL_ALL) != HAL_OK ) {
		printf("[ERROR] Failed to start timer 1 for the rotary encoder\n\r");
	} else {
		printf("[INFO] Timer 1 started\n\r");
	}
#else
	HAL_TIM_Encoder_Start_IT(&htim1, TIM_CHANNEL_ALL);
#endif
}

int32_t rotencGetDelta(void) {
	int32_t cnt = TIM1->CNT;

	if (cnt != prev_cnt) {
		printf("CNT: %ld, PREV: %ld\n\r", cnt, prev_cnt);
		uint32_t delta = cnt - prev_cnt;
		prev_cnt = cnt;

		return delta;
	}
	prev_cnt = cnt;
	return 0;
}


void rotencDeltaEvent(void) {
	if (rotencGetDelta() != 0) {
		stateInsertFlag(SFLAG_ROTENC_ROTATED);
	}
}
