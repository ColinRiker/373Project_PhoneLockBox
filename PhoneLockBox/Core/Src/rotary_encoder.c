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

extern TIM_HandleTypeDef htim1;  // external timer handle for Timer 1, which is used for rotary encoder
extern SFlag flags[MAX_FLAGS];   // array to store state flags

int32_t prev_cnt;  // stores the previous counter value to calculate the delta

// Initializes the rotary encoder and starts the timer for encoder readings
void rotencInit(void) {
    TIM1->CNT = 30000;  // reset the counter of Timer 1 to a base value (e.g., 30000)
    prev_cnt = TIM1->CNT;  // store the current counter value as the previous value

#ifdef DEBUG_ROTARY_ENCODER
    // Debugging: check if the timer starts successfully for the rotary encoder
    if (HAL_TIM_Encoder_Start_IT(&htim1, TIM_CHANNEL_ALL) != HAL_OK) {
        printf("[ERROR] Failed to start timer 1 for the rotary encoder\n\r");
    } else {
        printf("[INFO] Timer 1 started\n\r");
    }
#else
    // Start Timer 1 in encoder mode to trigger interrupts on the encoder input
    HAL_TIM_Encoder_Start_IT(&htim1, TIM_CHANNEL_ALL);
#endif
}

// Gets the delta (change in value) of the rotary encoder
int32_t rotencGetDelta(void) {
    int32_t cnt = TIM1->CNT;  // read the current counter value from Timer 1

    // If the counter has changed (rotary encoder was turned), calculate the delta
    if (cnt != prev_cnt) {
        // Calculate the difference (delta) between the current and previous counter values
        int32_t delta = cnt - prev_cnt;
        prev_cnt = cnt;  // update the previous counter value with the current value

        return delta;  // return the delta, which indicates the direction of rotation
    }

    prev_cnt = cnt;  // update the previous counter value if no change in the counter
    return 0;  // return 0 if no change in the counter (i.e., no rotation detected)
}

// Callback function triggered by an event to check the rotary encoder delta and set flags
void rotencDeltaEvent(void) {
    if (rotencGetDelta() != 0) {
        // If a rotation is detected, set the flag indicating the rotary encoder was rotated
        stateInsertFlag(SFLAG_ROTENC_ROTATED);
    }
}
