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

uint32_t prev_cnt;

void rotencInit(void) {
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

uint32_t rotencGetDelta(void) {
	uint32_t cnt = TIM1->CNT;

	if (cnt != prev_cnt) {
		uint32_t delta = cnt - prev_cnt;
		prev_cnt = cnt;

		return delta;
	}

	return 0;
}

bool rotencHasMoved(void) {
	return (bool) rotencGetDelta();
}


void rotencDeltaEvent(void) {
	uint8_t empty_idx = MAX_FLAGS;

	if (!rotencHasMoved()) return;

	for (uint8_t i = 0; i < MAX_FLAGS; ++i) {
		if (flags[i] == SFLAG_NULL && empty_idx == MAX_FLAGS) {
			if (flags[i] == SFLAG_ROTENC_ROTATED) {

#if defined(DEBUG_ROTARY_ENCODER) || defined(DEBUG_EVENT_CONTROLLER)
				printf("[INFO] ROTENC_ROTATED Flag already set\n\r");
#endif
				return;
			}

			empty_idx = i;
		}
	}

	flags[empty_idx] = SFLAG_ROTENC_ROTATED;

#if defined(DEBUG_ROTARY_ENCODER) || defined(DEBUG_EVENT_CONTROLLER)
	printf("[INFO] ROTENC_ROTATED Flag set\n\r");
#endif

}
