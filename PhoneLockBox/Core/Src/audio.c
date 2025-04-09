/*
 * audio.c
 *
 *  Created on: Apr 9, 2025
 *      Author: colinrik
 */

#include "audio.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "shared.h"
#include "stm32l4xx_hal.h"

extern TIM_HandleTypeDef htim2;

uint32_t matrix[MAX_ENTRIES] = {0};
uint8_t audio_count;
uint32_t last_interrupt_time;

void audioInit(void) {
	HAL_TIM_Base_Start(&htim2);
	audio_count = 0;
	last_interrupt_time = 0;
}

void audioCount(void) {
	uint32_t now = __HAL_TIM_GET_COUNTER(&htim2);
	uint32_t delta;

	if(now >= last_interrupt_time){
		delta = now - last_interrupt_time;
	}
	else{
		delta = (2000000 - last_interrupt_time) + now;
	}

	last_interrupt_time = now;
	++audio_count;

	if (audio_count < MAX_ENTRIES) {
		matrix[audio_count] = delta;
	}
#ifdef DEBUG_AUDIO
	printf("Interrupt #%lu triggered at %lu µs \n", audio_count, now);
#endif /* END DEBUG_AUDIO */

	if(audio_count == MAX_ENTRIES){
		//Set Flag so scheduler/state machine/event whatever calls audioMatch;
	}

}

bool audioMatch(void) {
	uint32_t sum_deltas = 0;
	float time_total= 0.0;

#ifdef DEBUG_AUDIO
	printf("=== Last %d Interrupts (ring buffer) ===\n", MAX_ENTRIES);
	for (int i = 0; i < MAX_ENTRIES; i++) {

		printf("Event %d: Δt = %lu us\n\r", i, matrix[i]);

	}
#endif /* END DEBUG_AUDIO */

	for (int r = 0; r < MAX_ENTRIES; r++) {
		sum_deltas += matrix[r];
		time_total = (float)sum_deltas/1000000.0f;
	}

#ifdef DEBUG_AUDIO
	printf("time gap total = %f seconds\n", time_total);
#endif /* END DEBUG_AUDIO */

	audio_count = 0;
	for (uint8_t i = 0; i < MAX_ENTRIES; ++i) {
		matrix[i] = 0;
	}

	return (time_total <= TIME_MAX_THRESH) && (time_total >= TIME_MIN_THRESH);
}

